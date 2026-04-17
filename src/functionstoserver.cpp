#include "functionstoserver.h"
#include "usermanager.h"
#include "dishmanager.h"
#include <QMap>
#include <functional>

// Тип функции для нашего FuncMap (принимает параметры и socketId, возвращает QString)
typedef std::function<QString(const QStringList&, qintptr)> CommandHandler;

QString FunctionsToServer::parsing(const QString &message, qintptr socketId) {
    if (message.isEmpty()) return "ERROR:Пустое сообщение";

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
            } else {
                // Для остальных команд используем запятую
                params = paramsStr.split(',');
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
        {"get_dish", DishManager::get_dish}
    };

    if (FuncMap.contains(cmd)) {
        return FuncMap[cmd](params, socketId); // Вызов нужного менеджера
    }

    return "ERROR:Неизвестная команда. Доступны: auth, reg, get_dish, get_stat";
}
