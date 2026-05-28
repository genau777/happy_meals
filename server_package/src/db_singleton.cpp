#include "db_singleton.h"

#include <QCoreApplication>
#include <QDir>
#include <QtGlobal>

namespace {
QString prettySearchSummary(const QString& raw)
{
    if (!raw.startsWith("ingredients=")) {
        return raw;
    }

    QMap<QString, QString> values;
    for (const QString& part : raw.split(';')) {
        int eq = part.indexOf('=');
        if (eq > 0) {
            values[part.left(eq).trimmed()] = part.mid(eq + 1).trimmed();
        }
    }

    auto readable = [](const QString& value, const QString& fallback) {
        return value.isEmpty() || value == "any" ? fallback : value;
    };

    return QString("Исключить: %1; кухня: %2; тип: %3; время до %4 мин; сложность: %5")
        .arg(readable(values.value("ingredients"), "любые ингредиенты"))
        .arg(readable(values.value("cuisines"), "любая"))
        .arg(readable(values.value("type"), "любой"))
        .arg(readable(values.value("maxTime"), "любое"))
        .arg(readable(values.value("complexity"), "любая"));
}
}

DB_Singleton *DB_Singleton::instance = nullptr;
DB_SingletonDestroyer DB_Singleton::destroyer;

///
/// \brief Удаляет singleton-экземпляр DB_Singleton.
///
DB_SingletonDestroyer::~DB_SingletonDestroyer()
{
    delete p_instance;
}

///
/// \brief Сохраняет указатель на singleton-экземпляр DB_Singleton.
/// \param p Указатель на объект DB_Singleton.
///
void DB_SingletonDestroyer::initialize(DB_Singleton *p)
{
    p_instance = p;
}

///
/// \brief Создает подключения к базам данных пользователей и блюд.
///
DB_Singleton::DB_Singleton()
{
    QDir appDir(QCoreApplication::applicationDirPath());
    QString dataPath = appDir.filePath("data");

    if (!QDir(dataPath).exists()) {
        QDir current(QDir::currentPath());
        dataPath = current.filePath("data");
    }

    db = QSqlDatabase::addDatabase("QSQLITE", "users_connection");
    db.setDatabaseName(QDir(dataPath).filePath("HappyMealsDB.sqlite"));

    qDebug() << "Users DB path:" << db.databaseName();

    if (!db.open()) {
        qDebug() << "DB Error:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);

    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "login VARCHAR(50) UNIQUE NOT NULL, "
               "password VARCHAR(255) NOT NULL, "
               "email VARCHAR(100))");
    query.exec("ALTER TABLE users DROP COLUMN socket_id");

    query.exec("CREATE TABLE IF NOT EXISTS history ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_id INTEGER, "
               "ingredient VARCHAR(100))");
    query.exec("ALTER TABLE history ADD COLUMN created_at TEXT");

    query.exec("CREATE TABLE IF NOT EXISTS favorites ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_id INTEGER, "
               "dish_name VARCHAR(100), "
               "UNIQUE(user_id, dish_name))");

    query.exec("CREATE TABLE IF NOT EXISTS user_sessions ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_id INTEGER NOT NULL, "
               "started_at TEXT NOT NULL, "
               "ended_at TEXT, "
               "duration_seconds INTEGER DEFAULT 0)");

    dishesDb = QSqlDatabase::addDatabase("QSQLITE", "dishes_connection");
    dishesDb.setDatabaseName(QDir(dataPath).filePath("dishes.sqlite"));

    qDebug() << "Dishes DB path:" << dishesDb.databaseName();

    if (!dishesDb.open()) {
        qDebug() << "Dishes DB Error:" << dishesDb.lastError().text();
        return;
    }

    initDishesDatabase();
}

///
/// \brief Закрывает подключения к базам данных.
///
DB_Singleton::~DB_Singleton()
{
    db.close();
    dishesDb.close();
}

///
/// \brief Возвращает единственный экземпляр DB_Singleton.
/// \return Указатель на singleton-объект DB_Singleton.
///
DB_Singleton* DB_Singleton::getInstance()
{
    if (!instance) {
        instance = new DB_Singleton();
        destroyer.initialize(instance);
    }

    return instance;
}

///
/// \brief Выполняет авторизацию пользователя.
/// \param login Логин пользователя.
/// \param pass Пароль пользователя.
/// \param socketId Идентификатор клиентского подключения.
/// \return true, если авторизация успешна, иначе false.
///
int DB_Singleton::authUserId(QString login, QString pass)
{
    login = login.trimmed();

    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE login = :login AND password = :pass");
    query.bindValue(":login", login);
    query.bindValue(":pass", pass);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool DB_Singleton::userExists(QString login)
{
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE login = :login");
    query.bindValue(":login", login.trimmed());
    return query.exec() && query.next();
}

///
/// \brief Регистрирует нового пользователя.
/// \param login Логин пользователя.
/// \param pass Пароль пользователя.
/// \param email Электронная почта пользователя.
/// \param socketId Идентификатор клиентского подключения.
/// \return true, если регистрация успешна, иначе false.
///
bool DB_Singleton::reg(QString login, QString pass, QString email)
{
    login = login.trimmed();
    email = email.trimmed();

    QSqlQuery check(db);
    check.prepare("SELECT id FROM users WHERE login = :login");
    check.bindValue(":login", login);

    if (check.exec() && check.next()) {
        return false;
    }

    QSqlQuery insert(db);
    insert.prepare("INSERT INTO users (login, password, email) "
                   "VALUES (:login, :pass, :email)");
    insert.bindValue(":login", login);
    insert.bindValue(":pass", pass);
    insert.bindValue(":email", email);

    return insert.exec();
}

///
/// \brief Очищает идентификатор сокета пользователя.
/// \param socketId Идентификатор клиентского подключения.
///
void DB_Singleton::log_search_for_user(QString login, QString summary)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", login.trimmed());

    if (userQuery.exec() && userQuery.next()) {
        log_search_for_user_id(userQuery.value(0).toInt(), summary);
    }
}

void DB_Singleton::log_search_for_user_id(int userId, QString summary)
{
    summary = prettySearchSummary(summary.trimmed());
    if (userId <= 0 || summary.isEmpty()) {
        return;
    }

    QSqlQuery lastQuery(db);
    lastQuery.prepare("SELECT ingredient FROM history "
                      "WHERE user_id = :uid "
                      "ORDER BY id DESC LIMIT 1");
    lastQuery.bindValue(":uid", userId);

    if (lastQuery.exec() && lastQuery.next()
        && prettySearchSummary(lastQuery.value(0).toString()) == summary) {
        return;
    }

    QSqlQuery insert(db);
    insert.prepare("INSERT INTO history (user_id, ingredient, created_at) "
                   "VALUES (:uid, :ingr, datetime('now', 'localtime'))");
    insert.bindValue(":uid", userId);
    insert.bindValue(":ingr", summary);
    insert.exec();
}

QStringList DB_Singleton::get_search_history_for_user(QString login)
{
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE login = :login");
    query.bindValue(":login", login.trimmed());

    if (query.exec() && query.next()) {
        return get_search_history_for_user_id(query.value(0).toInt());
    }

    return {};
}

QStringList DB_Singleton::get_search_history_for_user_id(int userId)
{
    QStringList history;
    QString previous;

    QSqlQuery query(db);
    query.prepare("SELECT ingredient, COALESCE(created_at, '') "
                  "FROM history "
                  "WHERE user_id = :uid "
                  "ORDER BY id DESC "
                  "LIMIT 50");
    query.bindValue(":uid", userId);

    if (query.exec()) {
        while (query.next() && history.size() < 30) {
            QString text = prettySearchSummary(query.value(0).toString());
            QString createdAt = query.value(1).toString();

            if (text.isEmpty() || text == previous) {
                continue;
            }

            history.append(createdAt.isEmpty() ? text : createdAt + " | " + text);
            previous = text;
        }
    }

    return history;
}

QString DB_Singleton::get_stat_for_user(QString login)
{
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE login = :login");
    query.bindValue(":login", login.trimmed());

    if (query.exec() && query.next()) {
        return get_stat_for_user_id(query.value(0).toInt());
    }

    return "ERROR:Пользователь не найден";
}

QString DB_Singleton::get_stat_for_user_id(int userId)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT login FROM users WHERE id = :uid");
    userQuery.bindValue(":uid", userId);

    if (!userQuery.exec() || !userQuery.next()) {
        return "ERROR:Пользователь не найден";
    }

    QString login = userQuery.value(0).toString();

    QSqlQuery searchQuery(db);
    searchQuery.prepare("SELECT COUNT(*) FROM history WHERE user_id = :uid");
    searchQuery.bindValue(":uid", userId);
    searchQuery.exec();
    int searches = searchQuery.next() ? searchQuery.value(0).toInt() : 0;

    QSqlQuery favQuery(db);
    favQuery.prepare("SELECT COUNT(*) FROM favorites WHERE user_id = :uid");
    favQuery.bindValue(":uid", userId);
    favQuery.exec();
    int favorites = favQuery.next() ? favQuery.value(0).toInt() : 0;

    QSqlQuery timeQuery(db);
    timeQuery.prepare("SELECT COALESCE(SUM("
                      "CASE "
                      "WHEN ended_at IS NULL THEN duration_seconds + CAST((julianday(datetime('now', 'localtime')) - julianday(started_at)) * 86400 AS INTEGER) "
                      "ELSE duration_seconds "
                      "END), 0) "
                      "FROM user_sessions WHERE user_id = :uid");
    timeQuery.bindValue(":uid", userId);
    timeQuery.exec();
    int seconds = timeQuery.next() ? timeQuery.value(0).toInt() : 0;
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;

    return QString("Пользователь: %1\nЗапросов подбора: %2\nИзбранных рецептов: %3\nВремя в приложении: %4 ч %5 мин")
        .arg(login)
        .arg(searches)
        .arg(favorites)
        .arg(hours)
        .arg(minutes);
}

int DB_Singleton::startUserSession(int userId)
{
    if (userId <= 0) {
        return 0;
    }

    endUserSession(userId);

    QSqlQuery insert(db);
    insert.prepare("INSERT INTO user_sessions (user_id, started_at) "
                   "VALUES (:uid, datetime('now', 'localtime'))");
    insert.bindValue(":uid", userId);

    if (!insert.exec()) {
        return 0;
    }

    return insert.lastInsertId().toInt();
}

void DB_Singleton::endUserSession(int userId)
{
    if (userId <= 0) {
        return;
    }

    QSqlQuery update(db);
    update.prepare("UPDATE user_sessions "
                   "SET ended_at = datetime('now', 'localtime'), "
                   "duration_seconds = CAST((julianday(datetime('now', 'localtime')) - julianday(started_at)) * 86400 AS INTEGER) "
                   "WHERE id = ("
                   "SELECT id FROM user_sessions "
                   "WHERE user_id = :uid AND ended_at IS NULL "
                   "ORDER BY id DESC LIMIT 1)");
    update.bindValue(":uid", userId);
    update.exec();
}

///
/// \brief Записывает поисковый запрос пользователя в историю по логину.
/// \param username Логин пользователя.
/// \param ingredient Ингредиент или поисковая строка.
///
void DB_Singleton::log_search_by_username(QString username, QString ingredient)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", username);

    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();

        QSqlQuery insert(db);
        insert.prepare("INSERT INTO history (user_id, ingredient, created_at) "
                       "VALUES (:uid, :ingr, datetime('now', 'localtime'))");
        insert.bindValue(":uid", userId);
        insert.bindValue(":ingr", ingredient);
        insert.exec();
    }
}

///
/// \brief Возвращает статистику пользователя по логину.
/// \param username Логин пользователя.
/// \return Строка со статистикой или сообщение об ошибке.
///
QString DB_Singleton::get_stat_by_username(QString username)
{
    QSqlQuery query(db);
    query.prepare("SELECT u.login, COUNT(h.id) "
                  "FROM users u "
                  "LEFT JOIN history h ON u.id = h.user_id "
                  "WHERE u.login = :login "
                  "GROUP BY u.id");
    query.bindValue(":login", username);

    if (query.exec() && query.next()) {
        return QString("Пользователь: %1, Запросов подбора: %2")
            .arg(query.value(0).toString())
            .arg(query.value(1).toInt());
    }

    return "ERROR:Пользователь не найден";
}

///
/// \brief Добавляет блюдо в избранное пользователя.
/// \param username Логин пользователя.
/// \param dishName Название блюда.
/// \return true, если блюдо успешно добавлено, иначе false.
///
bool DB_Singleton::addFavorite(QString username, QString dishName)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", username);

    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();

        QSqlQuery insert(db);
        insert.prepare("INSERT OR IGNORE INTO favorites (user_id, dish_name) "
                       "VALUES (:uid, :dish)");
        insert.bindValue(":uid", userId);
        insert.bindValue(":dish", dishName);

        return insert.exec();
    }

    return false;
}

bool DB_Singleton::addFavorite(int userId, QString dishName)
{
    if (userId <= 0 || dishName.trimmed().isEmpty()) {
        return false;
    }

    QSqlQuery insert(db);
    insert.prepare("INSERT OR IGNORE INTO favorites (user_id, dish_name) "
                   "VALUES (:uid, :dish)");
    insert.bindValue(":uid", userId);
    insert.bindValue(":dish", dishName.trimmed());
    return insert.exec();
}

///
/// \brief Удаляет блюдо из избранного пользователя.
/// \param username Логин пользователя.
/// \param dishName Название блюда.
/// \return true, если блюдо успешно удалено, иначе false.
///
bool DB_Singleton::removeFavorite(QString username, QString dishName)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", username);

    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();

        QSqlQuery deleteQuery(db);
        deleteQuery.prepare("DELETE FROM favorites WHERE user_id = :uid AND dish_name = :dish");
        deleteQuery.bindValue(":uid", userId);
        deleteQuery.bindValue(":dish", dishName);

        return deleteQuery.exec();
    }

    return false;
}

bool DB_Singleton::removeFavorite(int userId, QString dishName)
{
    if (userId <= 0 || dishName.trimmed().isEmpty()) {
        return false;
    }

    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM favorites WHERE user_id = :uid AND dish_name = :dish");
    deleteQuery.bindValue(":uid", userId);
    deleteQuery.bindValue(":dish", dishName.trimmed());
    return deleteQuery.exec();
}

///
/// \brief Возвращает список избранных блюд пользователя.
/// \param username Логин пользователя.
/// \return Список названий избранных блюд.
///
QStringList DB_Singleton::getFavorites(QString username)
{
    QStringList favorites;

    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", username);

    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();

        QSqlQuery favQuery(db);
        favQuery.prepare("SELECT dish_name FROM favorites WHERE user_id = :uid");
        favQuery.bindValue(":uid", userId);

        if (favQuery.exec()) {
            while (favQuery.next()) {
                favorites << favQuery.value(0).toString();
            }
        }
    }

    return favorites;
}

QStringList DB_Singleton::getFavorites(int userId)
{
    QStringList favorites;

    QSqlQuery favQuery(db);
    favQuery.prepare("SELECT dish_name FROM favorites WHERE user_id = :uid ORDER BY dish_name");
    favQuery.bindValue(":uid", userId);

    if (favQuery.exec()) {
        while (favQuery.next()) {
            favorites << favQuery.value(0).toString();
        }
    }

    return favorites;
}

///
/// \brief Создает таблицы базы данных блюд при их отсутствии.
///
void DB_Singleton::initDishesDatabase()
{
    QSqlQuery query(dishesDb);

    query.exec("CREATE TABLE IF NOT EXISTS dishes ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name VARCHAR(100) NOT NULL, "
               "cuisine VARCHAR(50), "
               "dish_type VARCHAR(50), "
               "prep_time INTEGER, "
               "description TEXT, "
               "instructions TEXT, "
               "image_url TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS ingredients ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name VARCHAR(100) NOT NULL UNIQUE, "
               "category VARCHAR(50))");

    query.exec("CREATE TABLE IF NOT EXISTS dish_ingredients ("
               "dish_id INTEGER, "
               "ingredient_id INTEGER, "
               "measure VARCHAR(100), "
               "FOREIGN KEY(dish_id) REFERENCES dishes(id), "
               "FOREIGN KEY(ingredient_id) REFERENCES ingredients(id), "
               "PRIMARY KEY(dish_id, ingredient_id))");

    query.exec("SELECT COUNT(*) FROM dishes");

    if (query.next() && query.value(0).toInt() == 0) {
        populateDishesData();
    }
}

///
/// \brief Заполняет базу данных начальными ингредиентами и блюдами.
///
void DB_Singleton::populateDishesData()
{
    QSqlQuery query(dishesDb);

    QMap<QString, int> ingredientIds;

    QStringList ingredients = {
        "Помидор", "Яйцо", "Курица", "Рис", "Лосось",
        "Сыр", "Паста", "Говядина", "Нори", "Авокадо",
        "Картофель", "Лук", "Морковь", "Тофу"
    };

    for (const QString& ing : ingredients) {
        query.prepare("INSERT OR IGNORE INTO ingredients (name, category) "
                      "VALUES (:name, 'OTHER')");
        query.bindValue(":name", ing);
        query.exec();

        query.prepare("SELECT id FROM ingredients WHERE name = :name");
        query.bindValue(":name", ing);
        query.exec();

        if (query.next()) {
            ingredientIds[ing] = query.value(0).toInt();
        }
    }

    struct DishData {
        QString name;
        QString cuisine;
        int prepTime;
        QStringList ingredients;
    };

    QList<DishData> dishes = {
        {"Омлет", "RUSSIAN", 10, {"Яйцо", "Помидор"}},
        {"Цезарь", "ITALIAN", 15, {"Курица", "Помидор", "Сыр"}},
        {"Суши", "JAPANESE", 30, {"Рис", "Лосось", "Нори"}},
        {"Карбонара", "ITALIAN", 20, {"Паста", "Яйцо", "Сыр"}},
        {"Борщ", "RUSSIAN", 60, {"Говядина", "Картофель", "Морковь", "Лук", "Помидор"}},
        {"Жареный рис", "CHINESE", 25, {"Рис", "Яйцо", "Курица", "Морковь"}},
        {"Мисо-суп", "JAPANESE", 10, {"Нори", "Тофу"}},
        {"Лосось на гриле", "JAPANESE", 15, {"Лосось"}},
        {"Стейк", "ANY", 12, {"Говядина"}},
        {"Картофельное пюре", "RUSSIAN", 20, {"Картофель"}}
    };

    for (const DishData& dish : dishes) {
        query.prepare("INSERT INTO dishes (name, cuisine, dish_type, prep_time) "
                      "VALUES (:name, :cuisine, 'SECOND_COURSE', :time)");
        query.bindValue(":name", dish.name);
        query.bindValue(":cuisine", dish.cuisine);
        query.bindValue(":time", dish.prepTime);
        query.exec();

        int dishId = query.lastInsertId().toInt();

        for (const QString& ing : dish.ingredients) {
            if (ingredientIds.contains(ing)) {
                query.prepare("INSERT INTO dish_ingredients (dish_id, ingredient_id) "
                              "VALUES (:dish_id, :ing_id)");
                query.bindValue(":dish_id", dishId);
                query.bindValue(":ing_id", ingredientIds[ing]);
                query.exec();
            }
        }
    }
}

///
/// \brief Фильтрует блюда по ингредиентам, кухне и времени приготовления.
/// \param excludedIngredients Список исключаемых ингредиентов.
/// \param cuisines Список выбранных кухонь.
/// \param maxTime Максимальное время приготовления.
/// \return Список блюд, подходящих под заданные фильтры.
///
QList<Dish> DB_Singleton::filterDishes(const QStringList& excludedIngredients,
                                       const QStringList& cuisines,
                                       const QStringList& dishTypes,
                                       int maxComplexity,
                                       int maxTime)
{
    QList<Dish> result;
    int effectiveMaxTime = maxTime;

    if (maxComplexity == 1) {
        effectiveMaxTime = effectiveMaxTime > 0 ? qMin(effectiveMaxTime, 30) : 30;
    } else if (maxComplexity == 2) {
        effectiveMaxTime = effectiveMaxTime > 0 ? qMin(effectiveMaxTime, 60) : 60;
    }

    QString sql = "SELECT DISTINCT d.id, d.name, d.cuisine, d.dish_type, d.prep_time "
                  "FROM dishes d ";

    bool hasWhere = false;

    if (!excludedIngredients.isEmpty()) {
        sql += "WHERE d.id NOT IN ("
               "SELECT di.dish_id FROM dish_ingredients di "
               "JOIN ingredients i ON di.ingredient_id = i.id "
               "WHERE LOWER(i.name) IN (";

        QStringList placeholders;
        for (int i = 0; i < excludedIngredients.size(); ++i) {
            placeholders.append("?");
        }

        sql += placeholders.join(", ") + ")) ";
        hasWhere = true;
    }

    if (!cuisines.isEmpty() && !cuisines.contains("ANY")) {
        sql += hasWhere ? "AND " : "WHERE ";
        sql += "d.cuisine IN (";

        QStringList placeholders;
        for (int i = 0; i < cuisines.size(); ++i) {
            placeholders.append("?");
        }

        sql += placeholders.join(", ") + ") ";
        hasWhere = true;
    }

    if (!dishTypes.isEmpty() && !dishTypes.contains("ANY")) {
        sql += hasWhere ? "AND " : "WHERE ";
        sql += "UPPER(d.dish_type) IN (";

        QStringList placeholders;
        for (int i = 0; i < dishTypes.size(); ++i) {
            placeholders.append("?");
        }

        sql += placeholders.join(", ") + ") ";
        hasWhere = true;
    }

    if (effectiveMaxTime > 0) {
        sql += hasWhere ? "AND " : "WHERE ";
        sql += "d.prep_time <= ? ";
    }

    sql += "ORDER BY d.prep_time ASC, d.name ASC LIMIT 60";

    QSqlQuery query(dishesDb);
    query.prepare(sql);

    int bindPos = 0;

    for (const QString& ing : excludedIngredients) {
        query.bindValue(bindPos++, ing.toLower());
    }

    for (const QString& cuisine : cuisines) {
        if (cuisine != "ANY") {
            query.bindValue(bindPos++, cuisine);
        }
    }

    for (const QString& dishType : dishTypes) {
        if (dishType != "ANY") {
            query.bindValue(bindPos++, dishType.toUpper());
        }
    }

    if (effectiveMaxTime > 0) {
        query.bindValue(bindPos++, effectiveMaxTime);
    }

    if (!query.exec()) {
        qDebug() << "Filter query error:" << query.lastError().text();
        qDebug() << "SQL:" << sql;
        return result;
    }

    while (query.next()) {
        Dish dish;
        dish.id = QUuid::createUuid();
        dish.name = query.value(1).toString();

        QString cuisineStr = query.value(2).toString();

        if (cuisineStr == "JAPANESE")
            dish.cuisine = CuisineType::JAPANESE;
        else if (cuisineStr == "ITALIAN")
            dish.cuisine = CuisineType::ITALIAN;
        else if (cuisineStr == "RUSSIAN")
            dish.cuisine = CuisineType::RUSSIAN;
        else if (cuisineStr == "CHINESE")
            dish.cuisine = CuisineType::CHINESE;
        else if (cuisineStr == "MEXICAN")
            dish.cuisine = CuisineType::MEXICAN;
        else
            dish.cuisine = CuisineType::ANY;

        dish.type = DishType::SECOND_COURSE;
        dish.prepTime = query.value(4).toInt();

        result.append(dish);
    }

    return result;
}

///
/// \brief Возвращает подробное HTML-описание блюда.
/// \param dishName Название блюда.
/// \return HTML-строка с описанием рецепта.
///
QString DB_Singleton::getDishDetails(QString dishName)
{
    QSqlQuery query(dishesDb);

    query.prepare("SELECT name, cuisine, dish_type, prep_time, description, instructions, image_url "
                  "FROM dishes WHERE name = ?");
    query.bindValue(0, dishName);

    if (!query.exec()) {
        return "<p>Ошибка запроса: " + query.lastError().text() + "</p>";
    }

    if (!query.next()) {
        return "<p>Информация о блюде не найдена. Искали: " + dishName + "</p>";
    }

    QString name = query.value(0).toString();
    QString cuisine = query.value(1).toString();
    int prepTime = query.value(3).toInt();
    QString description = query.value(4).toString();
    QString instructions = query.value(5).toString();

    QString html = "<h2 style='color: #e66a20;'>🍽️ " + name + "</h2>";

    if (!cuisine.isEmpty()) {
        html += "<p><b>Кухня:</b> " + cuisine + "</p>";
    }

    if (prepTime > 0) {
        html += "<p><b>Время приготовления:</b> " + QString::number(prepTime) + " минут</p>";
    }

    if (!instructions.isEmpty()) {
        html += "<h3>Приготовление:</h3>";
        html += "<p>" + instructions.replace("\n", "<br>") + "</p>";
    } else if (!description.isEmpty()) {
        html += "<h3>Описание:</h3>";
        html += "<p>" + description + "</p>";
    }

    return html;
}
