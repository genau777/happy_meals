#ifndef FUNCTIONSTOSERVER_H
#define FUNCTIONSTOSERVER_H

#include <QString>
#include <QStringList>

class FunctionsToServer {
public:
    static QString parsing(const QString &message, qintptr socketId);
};

#endif // FUNCTIONSTOSERVER_H
