#include "usermanager.h"
#include "db_singleton.h"

QString UserManager::auth(const QStringList& params, qintptr socketId) {
    if (params.size() < 2) return "ERROR:Формат должен быть auth:login,pass";
    if (DB_Singleton::getInstance()->auth(params[0], params[1], socketId))
        return "OK:Авторизация успешна";
    return "ERROR:Неверный логин или пароль";
}

QString UserManager::reg(const QStringList& params, qintptr socketId) {
    if (params.size() < 3) return "ERROR:Формат должен быть reg:login,pass,email";
    if (DB_Singleton::getInstance()->reg(params[0], params[1], params[2], socketId))
        return "OK:Регистрация успешна";
    return "ERROR:Логин уже занят";
}

QString UserManager::get_stat(const QStringList& params, qintptr socketId) {
    Q_UNUSED(params);
    QString stat = DB_Singleton::getInstance()->get_stat(socketId);
    if (stat.startsWith("ERROR")) return stat;
    return "OK:" + stat;
}
