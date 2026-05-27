#include "usermanager.h"
#include "db_singleton.h"

#include <QRegularExpression>

namespace {
bool isRegistrationDataValid(const QString& login,
                             const QString& password,
                             const QString& email)
{
    static const QRegularExpression emailRegex(
        R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)"
        );

    QString trimmedLogin = login.trimmed();

    return trimmedLogin.size() >= 3
           && !trimmedLogin.contains(QRegularExpression(R"(\s)"))
           && !trimmedLogin.contains(QRegularExpression(R"(^\d+$)"))
           && password.size() >= 8
           && emailRegex.match(email.trimmed()).hasMatch();
}
}

///
/// \brief Выполняет авторизацию пользователя.
/// \param params Список параметров авторизации: логин и пароль.
/// \param socketId Идентификатор клиентского подключения.
/// \return Строка с результатом авторизации.
///
QString UserManager::auth(const QStringList& params, qintptr socketId) {
    if (params.size() < 2) return "ERROR:Формат должен быть auth:login,pass";

    if (params[0].trimmed().isEmpty()) {
        return "ERROR:Введите логин";
    }

    if (params[1].size() < 8) {
        return "ERROR:Пароль должен быть не короче 8 символов";
    }

    if (DB_Singleton::getInstance()->auth(params[0].trimmed(), params[1], socketId))
        return "OK:Авторизация успешна";
    return "ERROR:Неверный логин или пароль";
}

///
/// \brief Выполняет регистрацию пользователя.
/// \param params Список параметров регистрации: логин, пароль и email.
/// \param socketId Идентификатор клиентского подключения.
/// \return Строка с результатом регистрации.
///
QString UserManager::reg(const QStringList& params, qintptr socketId) {
    if (params.size() < 3) return "ERROR:Формат должен быть reg:login,pass,email";

    if (!isRegistrationDataValid(params[0], params[1], params[2])) {
        return "ERROR:Логин должен быть не короче 3 символов и не только из цифр, пароль - от 8 символов, email - в формате example@example.com";
    }

    if (DB_Singleton::getInstance()->reg(params[0].trimmed(), params[1], params[2].trimmed(), socketId))
        return "OK:Регистрация успешна";
    return "ERROR:Логин уже занят";
}

///
/// \brief Возвращает статистику пользователя.
/// \param params Дополнительные параметры команды.
/// \param socketId Идентификатор клиентского подключения.
/// \return Строка со статистикой пользователя или сообщение об ошибке.
///
QString UserManager::get_stat(const QStringList& params, qintptr socketId) {
    QString login = params.value(0).trimmed();
    QString stat = login.isEmpty()
                       ? DB_Singleton::getInstance()->get_stat(socketId)
                       : DB_Singleton::getInstance()->get_stat_for_user(login);
    if (stat.startsWith("ERROR")) return stat;
    return "OK:" + stat;
}

QString UserManager::add_favorite(const QStringList& params, qintptr socketId) {
    if (params.size() < 2) return "ERROR:Формат должен быть add_favorite:login,dishName";
    Q_UNUSED(socketId);
    return DB_Singleton::getInstance()->addFavorite(params[0].trimmed(), params[1].trimmed())
               ? "OK:favorite_added"
               : "ERROR:favorite_not_added";
}

QString UserManager::remove_favorite(const QStringList& params, qintptr socketId) {
    if (params.size() < 2) return "ERROR:Формат должен быть remove_favorite:login,dishName";
    Q_UNUSED(socketId);
    return DB_Singleton::getInstance()->removeFavorite(params[0].trimmed(), params[1].trimmed())
               ? "OK:favorite_removed"
               : "ERROR:favorite_not_removed";
}

QString UserManager::get_favorites(const QStringList& params, qintptr socketId) {
    Q_UNUSED(socketId);
    QString login = params.value(0).trimmed();
    if (login.isEmpty()) return "ERROR:Вы не авторизованы";
    return "OK:" + DB_Singleton::getInstance()->getFavorites(login).join("|");
}

QString UserManager::get_history(const QStringList& params, qintptr socketId) {
    Q_UNUSED(socketId);
    QString login = params.value(0).trimmed();
    if (login.isEmpty()) return "ERROR:Вы не авторизованы";
    return "OK:" + DB_Singleton::getInstance()->get_search_history_for_user(login).join(QString(QChar(0x1F)));
}
