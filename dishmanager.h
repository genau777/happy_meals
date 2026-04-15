#ifndef DISHMANAGER_H
#define DISHMANAGER_H
#include <QString>
#include <QStringList>

class DishManager {
public:
    static QString get_dish(const QStringList& params, qintptr socketId);
};
#endif // DISHMANAGER_H
