#include "dishmanager.h"
#include "db_singleton.h"
#include "storage.h"
#include "filters.h"

QString DishManager::get_dish(const QStringList& params, qintptr socketId) {
    QString ingredient = params.isEmpty() ? "" : params[0].toLower().trimmed();
    
    // Пишем в историю (если пользователь гость - БД это проигнорирует, согласно Use Case UC3)
    if (!ingredient.isEmpty()) {
        DB_Singleton::getInstance()->log_search_request(socketId, ingredient);
    }

    Storage storage;
    FilterManager filterManager;
    Preferences prefs;

    if (!ingredient.isEmpty()) {
        Ingredient disliked {QUuid::createUuid(), ingredient, IngredientCategory::OTHER};
        prefs.dislikedIngredients.append(disliked);
    }

    QList<Dish> allDishes = storage.getAllDishes();
    QList<Dish> recommended = filterManager.applyAll(allDishes, prefs);

    if (recommended.isEmpty()) return "ERROR:Нет блюд удовлетворяющих вашим предпочтениям";
    
    QStringList result;
    for (const Dish& dish : recommended) {
        result.append(dish.name);
    }
    
    return "OK:" + result.join(","); // Ответ по UML: "OK:Омлет,Цезарь"
}
