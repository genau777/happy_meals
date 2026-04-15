#include "db_singleton.h"

DB_Singleton * DB_Singleton::instance = nullptr;
DB_SingletonDestroyer DB_Singleton::destroyer;

DB_SingletonDestroyer::~DB_SingletonDestroyer() { delete p_instance; }
void DB_SingletonDestroyer::initialize(DB_Singleton *p) { p_instance = p; }

DB_Singleton::DB_Singleton() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/data/HappyMealsDB.sqlite");

    if(!db.open()) {
        qDebug() << "DB Error:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, login VARCHAR(50) UNIQUE NOT NULL, password VARCHAR(255) NOT NULL, email VARCHAR(100), socket_id VARCHAR(20))");
    query.exec("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, ingredient VARCHAR(100))");
}

DB_Singleton::~DB_Singleton() { db.close(); }

DB_Singleton* DB_Singleton::getInstance() {
    if (!instance) {
        instance = new DB_Singleton();
        destroyer.initialize(instance);
    }
    return instance;
}

bool DB_Singleton::auth(QString login, QString pass, qintptr socketId) {
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE login = :login AND password = :pass");
    query.bindValue(":login", login);
    query.bindValue(":pass", pass);
    
    if (query.exec() && query.next()) {
        QSqlQuery update(db);
        update.prepare("UPDATE users SET socket_id = :sid WHERE login = :login");
        update.bindValue(":sid", QString::number(socketId));
        update.bindValue(":login", login);
        update.exec();
        return true;
    }
    return false;
}

bool DB_Singleton::reg(QString login, QString pass, QString email, qintptr socketId) {
    QSqlQuery check(db);
    check.prepare("SELECT id FROM users WHERE login = :login");
    check.bindValue(":login", login);
    if (check.exec() && check.next()) return false;

    QSqlQuery insert(db);
    insert.prepare("INSERT INTO users (login, password, email, socket_id) VALUES (:login, :pass, :email, :sid)");
    insert.bindValue(":login", login);
    insert.bindValue(":pass", pass);
    insert.bindValue(":email", email);
    insert.bindValue(":sid", QString::number(socketId));
    return insert.exec();
}

void DB_Singleton::clear_socket_id(qintptr socketId) {
    QSqlQuery query(db);
    query.prepare("UPDATE users SET socket_id = NULL WHERE socket_id = :sid");
    query.bindValue(":sid", QString::number(socketId));
    query.exec();
}

void DB_Singleton::log_search_request(qintptr socketId, QString ingredient) {
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE socket_id = :sid");
    userQuery.bindValue(":sid", QString::number(socketId));
    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();
        QSqlQuery insert(db);
        insert.prepare("INSERT INTO history (user_id, ingredient) VALUES (:uid, :ingr)");
        insert.bindValue(":uid", userId);
        insert.bindValue(":ingr", ingredient);
        insert.exec();
    }
}

QString DB_Singleton::get_stat(qintptr socketId) {
    QSqlQuery query(db);
    query.prepare("SELECT u.login, COUNT(h.id) FROM users u LEFT JOIN history h ON u.id = h.user_id WHERE u.socket_id = :sid GROUP BY u.id");
    query.bindValue(":sid", QString::number(socketId));
    if (query.exec() && query.next()) {
        return QString("Пользователь: %1, Запросов подбора: %2").arg(query.value(0).toString()).arg(query.value(1).toInt());
    }
    return "ERROR:Вы не авторизованы";
}
