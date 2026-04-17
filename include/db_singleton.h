#ifndef DB_SINGLETON_H
#define DB_SINGLETON_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QMap>
#include "models.h"

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
    QSqlDatabase dishesDb;

protected:
    DB_Singleton();
    DB_Singleton(const DB_Singleton& ) = delete;
    ~DB_Singleton();
    friend class DB_SingletonDestroyer;
    
    void initDishesDatabase();
    void populateDishesData();

public:
    static DB_Singleton* getInstance();
    
    // User management
    bool auth(QString login, QString pass, qintptr socketId);
    bool reg(QString login, QString pass, QString email, qintptr socketId);
    void clear_socket_id(qintptr socketId);
    
    void log_search_request(qintptr socketId, QString ingredient);
    void log_search_by_username(QString username, QString ingredient);
    QString get_stat(qintptr socketId);
    QString get_stat_by_username(QString username);
    
    // Favorites management
    bool addFavorite(QString username, QString dishName);
    bool removeFavorite(QString username, QString dishName);
    QStringList getFavorites(QString username);
    
    // Get detailed dish information
    QString getDishDetails(QString dishName);
    
    // Dishes database
    QList<Dish> filterDishes(const QStringList& excludedIngredients, 
                             const QStringList& cuisines, 
                             int maxTime);
};

#endif // DB_SINGLETON_H
