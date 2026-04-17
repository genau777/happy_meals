#include "db_singleton.h"

DB_Singleton * DB_Singleton::instance = nullptr;
DB_SingletonDestroyer DB_Singleton::destroyer;

DB_SingletonDestroyer::~DB_SingletonDestroyer() { delete p_instance; }
void DB_SingletonDestroyer::initialize(DB_Singleton *p) { p_instance = p; }

DB_Singleton::DB_Singleton() {
    // Users database
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data/HappyMealsDB.sqlite");

    if(!db.open()) {
        qDebug() << "DB Error:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, login VARCHAR(50) UNIQUE NOT NULL, password VARCHAR(255) NOT NULL, email VARCHAR(100), socket_id VARCHAR(20))");
    query.exec("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, ingredient VARCHAR(100))");
    query.exec("CREATE TABLE IF NOT EXISTS favorites (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, dish_name VARCHAR(100), UNIQUE(user_id, dish_name))");
    
    // Dishes database
    dishesDb = QSqlDatabase::addDatabase("QSQLITE", "dishes_connection");
    dishesDb.setDatabaseName("data/dishes.sqlite");
    
    if(!dishesDb.open()) {
        qDebug() << "Dishes DB Error:" << dishesDb.lastError().text();
        return;
    }
    
    initDishesDatabase();
}

DB_Singleton::~DB_Singleton() { 
    db.close();
    dishesDb.close();
}

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

void DB_Singleton::log_search_by_username(QString username, QString ingredient) {
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", username);
    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();
        QSqlQuery insert(db);
        insert.prepare("INSERT INTO history (user_id, ingredient) VALUES (:uid, :ingr)");
        insert.bindValue(":uid", userId);
        insert.bindValue(":ingr", ingredient);
        insert.exec();
    }
}

QString DB_Singleton::get_stat_by_username(QString username) {
    QSqlQuery query(db);
    query.prepare("SELECT u.login, COUNT(h.id) FROM users u LEFT JOIN history h ON u.id = h.user_id WHERE u.login = :login GROUP BY u.id");
    query.bindValue(":login", username);
    if (query.exec() && query.next()) {
        return QString("Пользователь: %1, Запросов подбора: %2").arg(query.value(0).toString()).arg(query.value(1).toInt());
    }
    return "ERROR:Пользователь не найден";
}

bool DB_Singleton::addFavorite(QString username, QString dishName) {
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT id FROM users WHERE login = :login");
    userQuery.bindValue(":login", username);
    if (userQuery.exec() && userQuery.next()) {
        int userId = userQuery.value(0).toInt();
        QSqlQuery insert(db);
        insert.prepare("INSERT OR IGNORE INTO favorites (user_id, dish_name) VALUES (:uid, :dish)");
        insert.bindValue(":uid", userId);
        insert.bindValue(":dish", dishName);
        return insert.exec();
    }
    return false;
}

bool DB_Singleton::removeFavorite(QString username, QString dishName) {
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

QStringList DB_Singleton::getFavorites(QString username) {
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


void DB_Singleton::initDishesDatabase() {
    QSqlQuery query(dishesDb);
    
    // Таблица блюд с расширенными полями
    query.exec("CREATE TABLE IF NOT EXISTS dishes ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name VARCHAR(100) NOT NULL, "
               "cuisine VARCHAR(50), "
               "dish_type VARCHAR(50), "
               "prep_time INTEGER, "
               "description TEXT, "
               "instructions TEXT, "
               "image_url TEXT)");
    
    // Таблица ингредиентов
    query.exec("CREATE TABLE IF NOT EXISTS ingredients ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name VARCHAR(100) NOT NULL UNIQUE, "
               "category VARCHAR(50))");
    
    // Связь многие-ко-многим между блюдами и ингредиентами с количеством
    query.exec("CREATE TABLE IF NOT EXISTS dish_ingredients ("
               "dish_id INTEGER, "
               "ingredient_id INTEGER, "
               "measure VARCHAR(100), "
               "FOREIGN KEY(dish_id) REFERENCES dishes(id), "
               "FOREIGN KEY(ingredient_id) REFERENCES ingredients(id), "
               "PRIMARY KEY(dish_id, ingredient_id))");
    
    // Проверяем, есть ли данные
    query.exec("SELECT COUNT(*) FROM dishes");
    if (query.next() && query.value(0).toInt() == 0) {
        populateDishesData();
    }
}

void DB_Singleton::populateDishesData() {
    QSqlQuery query(dishesDb);
    
    // Добавляем ингредиенты
    QMap<QString, int> ingredientIds;
    QStringList ingredients = {"Помидор", "Яйцо", "Курица", "Рис", "Лосось", 
                               "Сыр", "Паста", "Говядина", "Нори", "Авокадо", 
                               "Картофель", "Лук", "Морковь", "Тофу"};
    
    for (const QString& ing : ingredients) {
        query.prepare("INSERT OR IGNORE INTO ingredients (name, category) VALUES (:name, 'OTHER')");
        query.bindValue(":name", ing);
        query.exec();
        
        query.prepare("SELECT id FROM ingredients WHERE name = :name");
        query.bindValue(":name", ing);
        query.exec();
        if (query.next()) {
            ingredientIds[ing] = query.value(0).toInt();
        }
    }
    
    // Добавляем блюда
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
        
        // Добавляем ингредиенты для блюда
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

QList<Dish> DB_Singleton::filterDishes(const QStringList& excludedIngredients, 
                                        const QStringList& cuisines, 
                                        int maxTime) {
    QList<Dish> result;
    
    QString sql = "SELECT DISTINCT d.id, d.name, d.cuisine, d.dish_type, d.prep_time "
                  "FROM dishes d ";
    
    QStringList conditions;
    bool hasWhere = false;
    
    // Фильтр по нежелательным ингредиентам
    if (!excludedIngredients.isEmpty()) {
        sql += "WHERE d.id NOT IN ("
               "  SELECT di.dish_id FROM dish_ingredients di "
               "  JOIN ingredients i ON di.ingredient_id = i.id "
               "  WHERE LOWER(i.name) IN (";
        
        QStringList placeholders;
        for (int i = 0; i < excludedIngredients.size(); ++i) {
            placeholders.append("?");
        }
        sql += placeholders.join(", ") + ")) ";
        hasWhere = true;
    }
    
    // Фильтр по кухне
    if (!cuisines.isEmpty() && !cuisines.contains("ANY")) {
        QString cuisineCondition = hasWhere ? "AND " : "WHERE ";
        cuisineCondition += "d.cuisine IN (";
        QStringList cuisinePlaceholders;
        for (int i = 0; i < cuisines.size(); ++i) {
            cuisinePlaceholders.append("?");
        }
        cuisineCondition += cuisinePlaceholders.join(", ") + ") ";
        sql += cuisineCondition;
        hasWhere = true;
    }
    
    // Фильтр по времени
    if (maxTime > 0) {
        QString timeCondition = hasWhere ? "AND " : "WHERE ";
        timeCondition += "d.prep_time <= ? ";
        sql += timeCondition;
    }
    
    QSqlQuery query(dishesDb);
    query.prepare(sql);
    
    // Биндим параметры
    int bindPos = 0;
    
    // Биндим исключенные ингредиенты
    for (const QString& ing : excludedIngredients) {
        query.bindValue(bindPos++, ing.toLower());
    }
    
    // Биндим кухни
    for (const QString& cuisine : cuisines) {
        if (cuisine != "ANY") {
            query.bindValue(bindPos++, cuisine);  // Don't convert to uppercase
        }
    }
    
    // Биндим время
    if (maxTime > 0) {
        query.bindValue(bindPos++, maxTime);
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
        if (cuisineStr == "JAPANESE") dish.cuisine = CuisineType::JAPANESE;
        else if (cuisineStr == "ITALIAN") dish.cuisine = CuisineType::ITALIAN;
        else if (cuisineStr == "RUSSIAN") dish.cuisine = CuisineType::RUSSIAN;
        else if (cuisineStr == "CHINESE") dish.cuisine = CuisineType::CHINESE;
        else if (cuisineStr == "MEXICAN") dish.cuisine = CuisineType::MEXICAN;
        else dish.cuisine = CuisineType::ANY;
        
        dish.type = DishType::SECOND_COURSE;
        dish.prepTime = query.value(4).toInt();
        
        result.append(dish);
    }
    
    return result;
}

QString DB_Singleton::getDishDetails(QString dishName) {
    QSqlQuery query(dishesDb);
    
    qDebug() << "getDishDetails called with:" << dishName;
    
    // Get dish information
    query.prepare("SELECT name, cuisine, dish_type, prep_time, description, instructions, image_url FROM dishes WHERE name = ?");
    query.bindValue(0, dishName);
    
    if (!query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return "<p>Ошибка запроса: " + query.lastError().text() + "</p>";
    }
    
    if (!query.next()) {
        qDebug() << "No dish found with name:" << dishName;
        // Try to list all dishes to debug
        QSqlQuery debugQuery(dishesDb);
        debugQuery.exec("SELECT name FROM dishes LIMIT 5");
        qDebug() << "Sample dishes in database:";
        while (debugQuery.next()) {
            qDebug() << "  -" << debugQuery.value(0).toString();
        }
        return "<p>Информация о блюде не найдена. Искали: " + dishName + "</p>";
    }
    
    QString name = query.value(0).toString();
    QString cuisine = query.value(1).toString();
    QString dishType = query.value(2).toString();
    int prepTime = query.value(3).toInt();
    QString description = query.value(4).toString();
    QString instructions = query.value(5).toString();
    QString imageUrl = query.value(6).toString();
    
    // Get dish ID for ingredients lookup
    query.prepare("SELECT id FROM dishes WHERE name = ?");
    query.bindValue(0, dishName);
    query.exec();
    query.next();
    int dishId = query.value(0).toInt();
    
    // Get ingredients with measurements
    QStringList ingredientsList;
    query.prepare("SELECT i.name, di.measure FROM ingredients i "
                  "JOIN dish_ingredients di ON i.id = di.ingredient_id "
                  "WHERE di.dish_id = ?");
    query.bindValue(0, dishId);
    
    if (query.exec()) {
        while (query.next()) {
            QString ingredient = query.value(0).toString();
            QString measure = query.value(1).toString();
            if (!measure.isEmpty()) {
                ingredientsList.append(measure + " " + ingredient);
            } else {
                ingredientsList.append(ingredient);
            }
        }
    }
    
    // Build HTML
    QString html = "<h2 style='color: #e66a20;'>🍽️ " + name + "</h2>";
    
    if (!cuisine.isEmpty()) {
        html += "<p><b>Кухня:</b> " + cuisine + "</p>";
    }
    
    if (prepTime > 0) {
        html += "<p><b>Время приготовления:</b> " + QString::number(prepTime) + " минут</p>";
    }
    
    if (!ingredientsList.isEmpty()) {
        html += "<h3>Ингредиенты:</h3><ul>";
        for (const QString& ing : ingredientsList) {
            html += "<li>" + ing + "</li>";
        }
        html += "</ul>";
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
