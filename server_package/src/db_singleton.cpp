#include "db_singleton.h"

#include <QCoreApplication>
#include <QDir>
#include <QtGlobal>

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
               "email VARCHAR(100), "
               "socket_id VARCHAR(20))");

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
bool DB_Singleton::auth(QString login, QString pass, qintptr socketId)
{
    login = login.trimmed();

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

///
/// \brief Регистрирует нового пользователя.
/// \param login Логин пользователя.
/// \param pass Пароль пользователя.
/// \param email Электронная почта пользователя.
/// \param socketId Идентификатор клиентского подключения.
/// \return true, если регистрация успешна, иначе false.
///
bool DB_Singleton::reg(QString login, QString pass, QString email, qintptr socketId)
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
    insert.prepare("INSERT INTO users (login, password, email, socket_id) "
                   "VALUES (:login, :pass, :email, :sid)");
    insert.bindValue(":login", login);
    insert.bindValue(":pass", pass);
    insert.bindValue(":email", email);
    insert.bindValue(":sid", QString::number(socketId));

    return insert.exec();
}

///
/// \brief Очищает идентификатор сокета пользователя.
/// \param socketId Идентификатор клиентского подключения.
///
void DB_Singleton::clear_socket_id(qintptr socketId)
{
    QSqlQuery query(db);
    query.prepare("UPDATE users SET socket_id = NULL WHERE socket_id = :sid");
    query.bindValue(":sid", QString::number(socketId));
    query.exec();
}

///
/// \brief Записывает поисковый запрос пользователя в историю.
/// \param socketId Идентификатор клиентского подключения.
/// \param ingredient Ингредиент или поисковая строка.
///
void DB_Singleton::log_search_request(qintptr socketId, QString ingredient)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE socket_id = :sid");
    userQuery.bindValue(":sid", QString::number(socketId));

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

QStringList DB_Singleton::get_search_history(qintptr socketId)
{
    QStringList history;

    QSqlQuery query(db);
    query.prepare("SELECT h.ingredient, COALESCE(h.created_at, '') "
                  "FROM users u "
                  "JOIN history h ON u.id = h.user_id "
                  "WHERE u.socket_id = :sid "
                  "ORDER BY h.id DESC "
                  "LIMIT 30");
    query.bindValue(":sid", QString::number(socketId));

    if (query.exec()) {
        while (query.next()) {
            QString text = query.value(0).toString();
            QString createdAt = query.value(1).toString();
            history.append(createdAt.isEmpty() ? text : createdAt + " | " + text);
        }
    }

    return history;
}

///
/// \brief Возвращает статистику пользователя по socketId.
/// \param socketId Идентификатор клиентского подключения.
/// \return Строка со статистикой или сообщение об ошибке.
///
QString DB_Singleton::get_stat(qintptr socketId)
{
    QSqlQuery query(db);
    query.prepare("SELECT u.login, COUNT(h.id) "
                  "FROM users u "
                  "LEFT JOIN history h ON u.id = h.user_id "
                  "WHERE u.socket_id = :sid "
                  "GROUP BY u.id");
    query.bindValue(":sid", QString::number(socketId));

    if (query.exec() && query.next()) {
        QString login = query.value(0).toString();
        int searches = query.value(1).toInt();

        QSqlQuery favQuery(db);
        favQuery.prepare("SELECT COUNT(f.id) "
                         "FROM users u "
                         "LEFT JOIN favorites f ON u.id = f.user_id "
                         "WHERE u.socket_id = :sid");
        favQuery.bindValue(":sid", QString::number(socketId));
        favQuery.exec();
        int favorites = favQuery.next() ? favQuery.value(0).toInt() : 0;

        return QString("Пользователь: %1\nЗапросов подбора: %2\nИзбранных рецептов: %3")
            .arg(login)
            .arg(searches)
            .arg(favorites);
    }

    return "ERROR:Вы не авторизованы";
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

bool DB_Singleton::addFavorite(qintptr socketId, QString dishName)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT login FROM users WHERE socket_id = :sid");
    userQuery.bindValue(":sid", QString::number(socketId));

    if (userQuery.exec() && userQuery.next()) {
        return addFavorite(userQuery.value(0).toString(), dishName);
    }

    return false;
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

bool DB_Singleton::removeFavorite(qintptr socketId, QString dishName)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT login FROM users WHERE socket_id = :sid");
    userQuery.bindValue(":sid", QString::number(socketId));

    if (userQuery.exec() && userQuery.next()) {
        return removeFavorite(userQuery.value(0).toString(), dishName);
    }

    return false;
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

QStringList DB_Singleton::getFavorites(qintptr socketId)
{
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT login FROM users WHERE socket_id = :sid");
    userQuery.bindValue(":sid", QString::number(socketId));

    if (userQuery.exec() && userQuery.next()) {
        return getFavorites(userQuery.value(0).toString());
    }

    return {};
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
