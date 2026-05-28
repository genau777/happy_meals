#include "functionstoserver.h"
#include "usermanager.h"
#include "dishmanager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <functional>

///
/// \brief Тип функции-обработчика серверной команды.
///
/// Обработчик получает список параметров команды и идентификатор подключения,
/// после чего возвращает строковый ответ для клиента.
///
typedef std::function<QString(const QStringList&, qintptr)> CommandHandler;

namespace {
QString jsonResponse(bool ok, const QString& message = QString(), const QJsonObject& extra = QJsonObject())
{
    QJsonObject response = extra;
    response["ok"] = ok;
    response["message"] = message;
    return QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact));
}

QString responseMessage(const QString& response)
{
    int colonIndex = response.indexOf(':');
    return colonIndex >= 0 ? response.mid(colonIndex + 1) : response;
}

QString processJsonRequest(const QJsonObject& request, qintptr socketId)
{
    QString command = request.value("command").toString().trimmed().toLower();
    int userId = request.value("userId").toInt();
    QString response;

    if (command == "auth") {
        response = UserManager::auth({request.value("login").toString(), request.value("password").toString()}, socketId);
        QJsonObject extra;
        QString message = responseMessage(response);
        int marker = message.indexOf(";user_id=");
        if (marker >= 0) {
            extra["userId"] = message.mid(marker + 9).toInt();
            message = message.left(marker);
        }
        return jsonResponse(response.startsWith("OK:"), message, extra);
    }

    if (command == "reg") {
        response = UserManager::reg({request.value("login").toString(), request.value("password").toString(), request.value("email").toString()}, socketId);
        return jsonResponse(response.startsWith("OK:"), responseMessage(response));
    }

    if (command == "get_dish") {
        QStringList excludedIngredients;
        for (const QJsonValue& value : request.value("excludedIngredients").toArray()) {
            excludedIngredients.append(value.toString());
        }

        QStringList cuisines;
        for (const QJsonValue& value : request.value("cuisines").toArray()) {
            cuisines.append(value.toString());
        }

        QStringList dishTypes;
        for (const QJsonValue& value : request.value("dishTypes").toArray()) {
            dishTypes.append(value.toString());
        }

        response = DishManager::get_dish({
            excludedIngredients.join(','),
            cuisines.join(','),
            QString::number(request.value("maxTime").toInt()),
            dishTypes.join(','),
            QString::number(request.value("maxComplexity").toInt()),
            QString::number(userId),
            request.value("summary").toString()
        }, socketId);
        if (!response.startsWith("OK:")) {
            return jsonResponse(false, responseMessage(response));
        }

        QJsonArray dishes;
        for (const QString& entry : response.mid(3).split('|')) {
            QStringList fields = entry.split('\t');
            QString name = fields.value(0).trimmed();
            int prepTime = fields.value(1).toInt();

            if (!name.isEmpty()) {
                dishes.append(QJsonObject{{"name", name}, {"prepTime", prepTime}});
            }
        }

        return jsonResponse(true, QString(), {{"dishes", dishes}});
    }

    if (command == "dish_details") {
        response = DishManager::dish_details({request.value("name").toString()}, socketId);
        return response.startsWith("OK:")
                   ? jsonResponse(true, QString(), {{"html", response.mid(3)}})
                   : jsonResponse(false, responseMessage(response));
    }

    if (command == "get_stat") {
        response = UserManager::get_stat({QString::number(userId)}, socketId);
        return jsonResponse(response.startsWith("OK:"), responseMessage(response));
    }

    if (command == "get_favorites") {
        response = UserManager::get_favorites({QString::number(userId)}, socketId);
        QJsonArray favorites;

        if (response.startsWith("OK:")) {
            for (const QString& favorite : response.mid(3).split('|')) {
                QString trimmed = favorite.trimmed();
                if (!trimmed.isEmpty()) favorites.append(trimmed);
            }
        }

        return jsonResponse(response.startsWith("OK:"), responseMessage(response), {{"favorites", favorites}});
    }

    if (command == "get_history") {
        response = UserManager::get_history({QString::number(userId)}, socketId);
        QJsonArray history;

        if (response.startsWith("OK:")) {
            for (const QString& item : response.mid(3).split(QChar(0x1F))) {
                QString trimmed = item.trimmed();
                if (!trimmed.isEmpty()) history.append(trimmed);
            }
        }

        return jsonResponse(response.startsWith("OK:"), responseMessage(response), {{"history", history}});
    }

    if (command == "add_favorite") {
        response = UserManager::add_favorite({QString::number(userId), request.value("name").toString()}, socketId);
        return jsonResponse(response.startsWith("OK:"), responseMessage(response));
    }

    if (command == "remove_favorite") {
        response = UserManager::remove_favorite({QString::number(userId), request.value("name").toString()}, socketId);
        return jsonResponse(response.startsWith("OK:"), responseMessage(response));
    }

    if (command == "logout") {
        response = UserManager::logout({QString::number(userId)}, socketId);
        return jsonResponse(response.startsWith("OK:"), responseMessage(response));
    }

    return jsonResponse(false, "Неизвестная команда");
}
}

///
/// \brief Разбирает сообщение клиента и вызывает нужный обработчик команды.
/// \param message Текст команды от клиента.
/// \param socketId Идентификатор клиентского подключения.
/// \return Строка с результатом выполнения команды.
///
QString FunctionsToServer::parsing(const QString &message, qintptr socketId) {
    if (message.isEmpty()) return "ERROR:Пустое сообщение";

    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError && jsonDocument.isObject()) {
        return processJsonRequest(jsonDocument.object(), socketId);
    }

    // Протокол: команда:параметры
    // Для get_dish используем разделитель ; между группами параметров
    // get_dish:ингредиенты;кухни;время
    int colonIdx = message.indexOf(':');
    QString cmd;
    QStringList params;

    if (colonIdx != -1) {
        cmd = message.left(colonIdx).trimmed().toLower();
        QString paramsStr = message.mid(colonIdx + 1).trimmed();
        if (!paramsStr.isEmpty()) {
            // Для get_dish используем ; как разделитель групп
            if (cmd == "get_dish") {
                params = paramsStr.split(';');
            } else if (cmd == "auth" || cmd == "reg") {
                params = paramsStr.split(',');
            } else {
                params = {paramsStr};
            }
        }
    } else {
        cmd = message.trimmed().toLower();
    }

    // Тот самый FuncMap из UML диаграммы
    static QMap<QString, CommandHandler> FuncMap = {
        {"auth", UserManager::auth},
        {"reg", UserManager::reg},
        {"get_stat", UserManager::get_stat},
        {"add_favorite", UserManager::add_favorite},
        {"remove_favorite", UserManager::remove_favorite},
        {"get_favorites", UserManager::get_favorites},
        {"get_history", UserManager::get_history},
        {"logout", UserManager::logout},
        {"get_dish", DishManager::get_dish},
        {"dish_details", DishManager::dish_details}
    };

    if (FuncMap.contains(cmd)) {
        return FuncMap[cmd](params, socketId); // Вызов нужного менеджера
    }

    return "ERROR:Неизвестная команда. Доступны: auth, reg, get_dish, get_stat";
}
