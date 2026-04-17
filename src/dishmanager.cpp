#include "dishmanager.h"
#include "db_singleton.h"

QString DishManager::get_dish(const QStringList& params, qintptr socketId) {
    // Новый формат: get_dish:ingredients;cuisines;maxTime
    // ingredients - список через запятую: помидор,яйцо
    // cuisines - список через запятую: italian,japanese
    // maxTime - число
    // Пример: get_dish:помидор,яйцо;italian,japanese;20
    // Пример: get_dish:;;30 (только время)
    // Пример: get_dish:помидор (только ингредиенты)
    
    QString ingredientsStr = params.size() > 0 ? params[0].trimmed() : "";
    QString cuisinesStr = params.size() > 1 ? params[1].trimmed() : "";
    int maxTime = params.size() > 2 ? params[2].toInt() : 0;
    
    // Парсим список нежелательных ингредиентов
    QStringList excludedIngredients;
    if (!ingredientsStr.isEmpty()) {
        QStringList ingList = ingredientsStr.split(',');
        for (const QString& ing : ingList) {
            QString trimmed = ing.trimmed().toLower();
            if (!trimmed.isEmpty()) {
                excludedIngredients.append(trimmed);
                // Логируем первый ингредиент для истории
                if (excludedIngredients.size() == 1) {
                    DB_Singleton::getInstance()->log_search_request(socketId, trimmed);
                }
            }
        }
    }
    
    // Парсим список кухонь
    QStringList cuisines;
    if (!cuisinesStr.isEmpty()) {
        QStringList cuisineList = cuisinesStr.split(',');
        for (const QString& c : cuisineList) {
            QString trimmed = c.trimmed().toUpper();
            if (!trimmed.isEmpty()) {
                // Конвертируем в формат базы данных
                if (trimmed == "JAPANESE" || trimmed == "ЯПОНСКАЯ") cuisines.append("JAPANESE");
                else if (trimmed == "ITALIAN" || trimmed == "ИТАЛЬЯНСКАЯ") cuisines.append("ITALIAN");
                else if (trimmed == "RUSSIAN" || trimmed == "РУССКАЯ") cuisines.append("RUSSIAN");
                else if (trimmed == "CHINESE" || trimmed == "КИТАЙСКАЯ") cuisines.append("CHINESE");
                else if (trimmed == "MEXICAN" || trimmed == "МЕКСИКАНСКАЯ") cuisines.append("MEXICAN");
            }
        }
    }
    
    // Получаем блюда из базы данных с фильтрацией
    QList<Dish> recommended = DB_Singleton::getInstance()->filterDishes(
        excludedIngredients, 
        cuisines, 
        maxTime
    );

    if (recommended.isEmpty()) return "ERROR:Нет блюд удовлетворяющих вашим предпочтениям";
    
    QStringList result;
    for (const Dish& dish : recommended) {
        result.append(QString("%1 (%2 мин)").arg(dish.name).arg(dish.prepTime));
    }
    
    return "OK:" + result.join(", ");
}
