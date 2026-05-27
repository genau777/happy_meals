#include "clientapi.h"

#include <QDebug>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>

ClientApi* ClientApi::p_instance = nullptr;
ClientApiDestroyer ClientApi::destroyer;

///
/// \brief Удаляет singleton-экземпляр ClientApi.
///
ClientApiDestroyer::~ClientApiDestroyer()
{
    delete p_instance;
}

///
/// \brief Сохраняет указатель на singleton-экземпляр ClientApi.
/// \param p Указатель на объект ClientApi.
///
void ClientApiDestroyer::initialize(ClientApi* p)
{
    p_instance = p;
}

///
/// \brief Создает объект клиентского API.
/// \param parent Родительский объект Qt.
///
ClientApi::ClientApi(QObject* parent)
    : QObject(parent),
    sock(new QTcpSocket(this)),
    serverHost("127.0.0.1"),
    serverPort(40000)
{
}

///
/// \brief Уничтожает объект клиентского API.
///
ClientApi::~ClientApi()
{
}

///
/// \brief Возвращает единственный экземпляр ClientApi.
/// \return Указатель на объект ClientApi.
///
ClientApi* ClientApi::getInstance()
{
    if (!p_instance) {
        p_instance = new ClientApi();
        destroyer.initialize(p_instance);
    }

    return p_instance;
}

///
/// \brief Выполняет подключение к серверу.
/// \param host Адрес сервера.
/// \param port Порт сервера.
/// \return true, если подключение успешно.
///
bool ClientApi::connectToServer(const QString& host, quint16 port)
{
    serverHost = host;
    serverPort = port;

    if (sock->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    sock->abort();
    sock->connectToHost(serverHost, serverPort);

    if (!sock->waitForConnected(3000)) {
        qDebug() << "Server connection failed:" << sock->errorString();
        return false;
    }

    if (sock->waitForReadyRead(200)) {
        sock->readAll();
    }

    return true;
}

QString ClientApi::sendCommand(const QString& command)
{
    if (!connectToServer(serverHost, serverPort)) {
        return "ERROR:server_unavailable";
    }

    sock->write((command + "\n").toUtf8());

    if (!sock->waitForBytesWritten(3000)) {
        return "ERROR:request_not_sent";
    }

    if (!sock->waitForReadyRead(5000)) {
        return "ERROR:server_timeout";
    }

    QString response = QString::fromUtf8(sock->readLine()).trimmed();

    while (response.isEmpty() && sock->canReadLine()) {
        response = QString::fromUtf8(sock->readLine()).trimmed();
    }

    return response;
}

QJsonObject ClientApi::sendJsonCommand(const QJsonObject& request)
{
    if (!connectToServer(serverHost, serverPort)) {
        return {
            {"ok", false},
            {"message", "server_unavailable"}
        };
    }

    while (sock->bytesAvailable() > 0 || sock->canReadLine()) {
        sock->readAll();
    }

    QByteArray payload = QJsonDocument(request).toJson(QJsonDocument::Compact);
    payload.append('\n');
    sock->write(payload);

    if (!sock->waitForBytesWritten(3000)) {
        return {
            {"ok", false},
            {"message", "request_not_sent"}
        };
    }

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 5000) {
        int remaining = 5000 - static_cast<int>(timer.elapsed());
        if (!sock->canReadLine() && !sock->waitForReadyRead(qMax(1, remaining))) {
            break;
        }

        while (sock->canReadLine()) {
            QByteArray line = sock->readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }

            QJsonParseError error;
            QJsonDocument document = QJsonDocument::fromJson(line, &error);

            if (error.error == QJsonParseError::NoError && document.isObject()) {
                return document.object();
            }
        }
    }

    return {
        {"ok", false},
        {"message", "server_timeout"}
    };
}

/*
 * ============================================================
 * ФУНКЦИЯ ДЛЯ UNIT TEST
 * ============================================================
 * Назначение:
 * Проверяет корректность данных регистрации.
 *
 * На вход получает:
 * - login — логин пользователя;
 * - password — пароль пользователя;
 * - email — электронную почту пользователя.
 *
 * На выход возвращает:
 * - true, если все поля заполнены;
 * - false, если хотя бы одно поле пустое или состоит только из пробелов.
 */
QString validateRegisterData(const QString& login,
                             const QString& password,
                             const QString& email)
{
    static const QRegularExpression emailRegex(
        R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)"
        );

    QString trimmedLogin = login.trimmed();

    if (trimmedLogin.size() < 3) {
        return "ERROR:Логин должен быть не короче 3 символов";
    }

    if (trimmedLogin.contains(QRegularExpression(R"(\s)"))) {
        return "ERROR:Логин не должен содержать пробелы";
    }

    if (trimmedLogin.contains(QRegularExpression(R"(^\d+$)"))) {
        return "ERROR:Логин не может состоять только из цифр";
    }

    if (password.size() < 8) {
        return "ERROR:Пароль должен быть не короче 8 символов";
    }

    if (!emailRegex.match(email.trimmed()).hasMatch()) {
        return "ERROR:Email должен быть в формате example@example.com";
    }

    return {};
}

///
/// \brief Регистрирует нового пользователя.
/// \param login Логин пользователя.
/// \param password Пароль пользователя.
/// \param email Электронная почта пользователя.
/// \return Строка с результатом регистрации.
///
QString ClientApi::registerUser(const QString& login,
                                const QString& password,
                                const QString& email)
{
    QString validationError = validateRegisterData(login, password, email);
    bool isValid = validationError.isEmpty();

    qDebug() << "UNIT TEST: isRegisterDataValid(login, password, email) =" << isValid;

    if (!isValid) {
        qDebug() << "UNIT TEST RESULT: registration data is invalid";
        return validationError;
    }

    qDebug() << "UNIT TEST RESULT: registration data is valid";

    QJsonObject response = sendJsonCommand({
        {"command", "reg"},
        {"login", login.trimmed()},
        {"password", password},
        {"email", email.trimmed()}
    });

    return response.value("ok").toBool()
               ? "OK:" + response.value("message").toString()
               : "ERROR:" + response.value("message").toString();
}

QString ClientApi::loginUser(const QString& login,
                             const QString& password)
{
    if (login.trimmed().isEmpty()) {
        return "ERROR:Введите логин";
    }

    if (password.size() < 8) {
        return "ERROR:Пароль должен быть не короче 8 символов";
    }

    QJsonObject response = sendJsonCommand({
        {"command", "auth"},
        {"login", login.trimmed()},
        {"password", password}
    });

    return response.value("ok").toBool()
               ? "OK:" + response.value("message").toString()
               : "ERROR:" + response.value("message").toString();
}

///
/// \brief Выполняет поиск блюд по заданным фильтрам.
/// \param excludedIngredients Список исключаемых ингредиентов.
/// \param cuisine Выбранная кухня.
/// \param type Тип блюда.
/// \param maxTime Максимальное время приготовления.
/// \param maxComplexity Максимальная сложность приготовления.
/// \return Список названий найденных блюд.
///
QStringList ClientApi::findDishes(const QStringList& excludedIngredients,
                                  const QString& cuisine,
                                  const QString& type,
                                  int maxTime,
                                  int maxComplexity)
{
    QStringList result;
    QStringList cuisines;

    if (!cuisine.trimmed().isEmpty() && cuisine != "Любая кухня") {
        QString cuisineUpper = cuisine.trimmed().toUpper();

        if (cuisineUpper.contains("РУССКАЯ") || cuisineUpper.contains("RUSSIAN")) {
            cuisines.append("RUSSIAN");
        } else if (cuisineUpper.contains("ИТАЛЬЯНСКАЯ") || cuisineUpper.contains("ITALIAN")) {
            cuisines.append("ITALIAN");
        } else if (cuisineUpper.contains("ЯПОНСКАЯ") || cuisineUpper.contains("JAPANESE")) {
            cuisines.append("JAPANESE");
        } else if (cuisineUpper.contains("КИТАЙСКАЯ") || cuisineUpper.contains("CHINESE")) {
            cuisines.append("CHINESE");
        } else if (cuisineUpper.contains("МЕКСИКАНСКАЯ") || cuisineUpper.contains("MEXICAN")) {
            cuisines.append("MEXICAN");
        } else if (cuisineUpper.contains("ЛЮБАЯ") || cuisineUpper.contains("ANY")) {
            cuisines.append("ANY");
        }
    }

    QStringList excludedLower;

    for (const QString& ing : excludedIngredients) {
        QString trimmed = ing.trimmed().toLower();

        if (!trimmed.isEmpty()) {
            excludedLower.append(trimmed);
        }
    }

    QJsonArray excludedArray;
    for (const QString& ingredient : excludedLower) {
        excludedArray.append(ingredient);
    }

    QJsonArray cuisineArray;
    for (const QString& cuisineName : cuisines) {
        cuisineArray.append(cuisineName);
    }

    QStringList dishTypes;
    QString typeUpper = type.trimmed().toUpper();
    if (!typeUpper.isEmpty()) {
        if (typeUpper.contains("ЗАВТРАК") || typeUpper.contains("BREAKFAST")) {
            dishTypes.append("BREAKFAST");
        } else if (typeUpper.contains("ВТОРОЕ") || typeUpper.contains("MAIN") || typeUpper.contains("SECOND")) {
            dishTypes.append("SECOND_COURSE");
        } else if (typeUpper.contains("САЛАТ") || typeUpper.contains("SALAD")) {
            dishTypes.append("SALAD");
        } else if (typeUpper.contains("ДЕСЕРТ") || typeUpper.contains("DESSERT")) {
            dishTypes.append("DESSERT");
        }
    }

    QJsonArray typeArray;
    for (const QString& dishType : dishTypes) {
        typeArray.append(dishType);
    }

    QJsonObject response = sendJsonCommand({
        {"command", "get_dish"},
        {"excludedIngredients", excludedArray},
        {"cuisines", cuisineArray},
        {"dishTypes", typeArray},
        {"maxTime", maxTime},
        {"maxComplexity", maxComplexity}
    });

    if (!response.value("ok").toBool()) {
        return {};
    }

    QJsonArray dishes = response.value("dishes").toArray();
    for (const QJsonValue& value : dishes) {
        QJsonObject dish = value.toObject();
        QString title = dish.value("name").toString().trimmed();
        int time = dish.value("prepTime").toInt();
        if (!title.isEmpty()) {
            result.append(time > 0 ? QString("%1\t%2").arg(title).arg(time) : title);
        }
    }

    return result;
}

///
/// \brief Получает подробное описание блюда.
/// \param dishName Название блюда.
/// \return HTML-строка с описанием рецепта.
///
QString ClientApi::getDishDetails(const QString& dishName)
{
    QJsonObject response = sendJsonCommand({
        {"command", "dish_details"},
        {"name", dishName}
    });

    if (response.value("ok").toBool()) {
        return response.value("html").toString();
    }

    return "<p>" + response.value("message").toString().toHtmlEscaped() + "</p>";
}

QString ClientApi::getStatistics()
{
    QJsonObject response = sendJsonCommand({{"command", "get_stat"}});

    return response.value("ok").toBool()
               ? "OK:" + response.value("message").toString()
               : "ERROR:" + response.value("message").toString();
}

QStringList ClientApi::getFavorites()
{
    QJsonObject response = sendJsonCommand({{"command", "get_favorites"}});
    if (!response.value("ok").toBool()) {
        return {};
    }

    QStringList favorites;

    for (const QJsonValue& value : response.value("favorites").toArray()) {
        QString favorite = value.toString().trimmed();

        if (!favorite.isEmpty()) {
            favorites.append(favorite);
        }
    }

    return favorites;
}

QStringList ClientApi::getSearchHistory()
{
    QJsonObject response = sendJsonCommand({{"command", "get_history"}});
    if (!response.value("ok").toBool()) {
        return {};
    }

    QStringList history;

    for (const QJsonValue& value : response.value("history").toArray()) {
        QString item = value.toString().trimmed();

        if (!item.isEmpty()) {
            history.append(item);
        }
    }

    return history;
}

QString ClientApi::addFavorite(const QString& dishName)
{
    QJsonObject response = sendJsonCommand({
        {"command", "add_favorite"},
        {"name", dishName}
    });

    return response.value("ok").toBool()
               ? "OK:" + response.value("message").toString()
               : "ERROR:" + response.value("message").toString();
}

QString ClientApi::removeFavorite(const QString& dishName)
{
    QJsonObject response = sendJsonCommand({
        {"command", "remove_favorite"},
        {"name", dishName}
    });

    return response.value("ok").toBool()
               ? "OK:" + response.value("message").toString()
               : "ERROR:" + response.value("message").toString();
}
