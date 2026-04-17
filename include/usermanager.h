#ifndef USERMANAGER_H
#define USERMANAGER_H
#include <QString>
#include <QStringList>

class UserManager {
public:
    static QString auth(const QStringList& params, qintptr socketId);
    static QString reg(const QStringList& params, qintptr socketId);
    static QString get_stat(const QStringList& params, qintptr socketId);
};
#endif // USERMANAGER_H
