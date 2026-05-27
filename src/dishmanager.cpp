#include "dishmanager.h"
#include "db_singleton.h"
#include "models.h"

#include <QString>
#include <QStringList>

///
/// \brief Выполняет подбор блюд по параметрам пользователя.
/// \param params Список параметров поиска: исключаемые ингредиенты, кухня и максимальное время приготовления.
/// \param socketId Идентификатор клиентского подключения.
/// \return Строка с найденными блюдами или сообщение об ошибке.
///
QString DishManager::get_dish(const QStringList& params, qintptr socketId)
{
    QString ingredientsStr = params.size() > 0 ? params[0].trimmed() : "";
    QString cuisinesStr = params.size() > 1 ? params[1].trimmed() : "";
    int maxTime = params.size() > 2 ? params[2].toInt() : 0;
    QString typeStr = params.size() > 3 ? params[3].trimmed() : "";
    int maxComplexity = params.size() > 4 ? params[4].toInt() : 0;
    QString login = params.size() > 5 ? params[5].trimmed() : "";
    QString summary = params.size() > 6 ? params[6].trimmed() : "";

    if (summary.isEmpty()) {
        summary = QString("Исключить: %1; кухня: %2; тип: %3; время до %4 мин; сложность: %5")
            .arg(ingredientsStr.isEmpty() ? "any" : ingredientsStr)
            .arg(cuisinesStr.isEmpty() ? "any" : cuisinesStr)
            .arg(typeStr.isEmpty() ? "any" : typeStr)
            .arg(maxTime > 0 ? QString::number(maxTime) : "any")
            .arg(maxComplexity > 0 ? QString::number(maxComplexity) : "any");
    }

    if (!login.isEmpty()) {
        DB_Singleton::getInstance()->log_search_for_user(login, summary);
    } else {
        DB_Singleton::getInstance()->log_search_request(socketId, summary);
    }

    QStringList excludedIngredients;

    if (!ingredientsStr.isEmpty()) {
        QStringList ingList = ingredientsStr.split(',');

        for (const QString& ing : ingList) {
            QString trimmed = ing.trimmed().toLower();

            if (!trimmed.isEmpty()) {
                excludedIngredients.append(trimmed);
            }
        }
    }

    QStringList cuisines;

    if (!cuisinesStr.isEmpty()) {
        QStringList cuisineList = cuisinesStr.split(',');

        for (const QString& c : cuisineList) {
            QString trimmed = c.trimmed().toUpper();

            if (!trimmed.isEmpty()) {
                if (trimmed == "JAPANESE" || trimmed == "ЯПОНСКАЯ") {
                    cuisines.append("JAPANESE");
                } else if (trimmed == "ITALIAN" || trimmed == "ИТАЛЬЯНСКАЯ") {
                    cuisines.append("ITALIAN");
                } else if (trimmed == "RUSSIAN" || trimmed == "РУССКАЯ") {
                    cuisines.append("RUSSIAN");
                } else if (trimmed == "CHINESE" || trimmed == "КИТАЙСКАЯ") {
                    cuisines.append("CHINESE");
                } else if (trimmed == "MEXICAN" || trimmed == "МЕКСИКАНСКАЯ") {
                    cuisines.append("MEXICAN");
                } else if (trimmed == "ANY" || trimmed == "ЛЮБАЯ") {
                    cuisines.append("ANY");
                }
            }
        }
    }

    QStringList dishTypes;

    if (!typeStr.isEmpty()) {
        QStringList typeList = typeStr.split(',');

        for (const QString& type : typeList) {
            QString trimmed = type.trimmed().toUpper();

            if (!trimmed.isEmpty()) {
                if (trimmed == "BREAKFAST" || trimmed.contains("ЗАВТРАК")) {
                    dishTypes.append("BREAKFAST");
                } else if (trimmed == "SECOND_COURSE" || trimmed == "MAIN" || trimmed.contains("ВТОРОЕ")) {
                    dishTypes.append("SECOND_COURSE");
                } else if (trimmed == "SALAD" || trimmed.contains("САЛАТ")) {
                    dishTypes.append("SALAD");
                } else if (trimmed == "DESSERT" || trimmed.contains("ДЕСЕРТ")) {
                    dishTypes.append("DESSERT");
                } else if (trimmed == "ANY") {
                    dishTypes.append("ANY");
                }
            }
        }
    }

    QList<Dish> recommended = DB_Singleton::getInstance()->filterDishes(
        excludedIngredients,
        cuisines,
        dishTypes,
        maxComplexity,
        maxTime
        );

    if (recommended.isEmpty()) {
        return "ERROR:Нет блюд удовлетворяющих вашим предпочтениям";
    }

    QStringList result;

    for (const Dish& dish : recommended) {
        result.append(QString("%1\t%2").arg(dish.name).arg(dish.prepTime));
    }

    return "OK:" + result.join("|");
}

QString DishManager::dish_details(const QStringList& params, qintptr socketId)
{
    Q_UNUSED(socketId);

    if (params.isEmpty() || params[0].trimmed().isEmpty()) {
        return "ERROR:Формат должен быть dish_details:name";
    }

    return "OK:" + DB_Singleton::getInstance()->getDishDetails(params[0].trimmed());
}
