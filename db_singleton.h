#ifndef DB_SINGLETON_H
#define DB_SINGLETON_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QString>

class DB_Singleton;

class DB_SingletonDestroyer {
private:
    DB_Singleton * p_instance;
public:
    ~DB_SingletonDestroyer();
    void initialize(DB_Singleton * p);
};

class DB_Singleton {
private:
    static DB_Singleton * instance;
    static DB_SingletonDestroyer destroyer;
    QSqlDatabase db;

protected:
    DB_Singleton();
    DB_Singleton(const DB_Singleton& ) = delete;
    ~DB_Singleton();
    friend class DB_SingletonDestroyer;

public:
    static DB_Singleton* getInstance();
    
    bool auth(QString login, QString pass, qintptr socketId);
    bool reg(QString login, QString pass, QString email, qintptr socketId);
    void clear_socket_id(qintptr socketId);
    
    void log_search_request(qintptr socketId, QString ingredient);
    QString get_stat(qintptr socketId);
};

#endif // DB_SINGLETON_H
