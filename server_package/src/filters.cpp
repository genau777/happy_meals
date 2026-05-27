#include "filters.h"

///
/// \brief Создает фильтр по исключаемым ингредиентам.
/// \param excludedIngredients Список ингредиентов, которые нужно исключить.
///
IngredientFilter::IngredientFilter(const QList<Ingredient>& excludedIngredients)
    : excluded(excludedIngredients)
{
}

///
/// \brief Применяет фильтр по исключаемым ингредиентам.
/// \param dishes Исходный список блюд.
/// \return Список блюд, в которых нет исключаемых ингредиентов.
///
QList<Dish> IngredientFilter::apply(const QList<Dish>& dishes)
{
    if (excluded.isEmpty())
        return dishes;

    QList<Dish> result;

    for (const Dish& dish : dishes) {
        bool hasDisliked = false;

        for (const Ingredient& dishIng : dish.ingredients) {
            for (const Ingredient& exclIng : excluded) {
                if (dishIng.name.toLower() == exclIng.name.toLower()) {
                    hasDisliked = true;
                    break;
                }
            }

            if (hasDisliked)
                break;
        }

        if (!hasDisliked)
            result.append(dish);
    }

    return result;
}

///
/// \brief Применяет фильтр по типу кухни.
/// \param dishes Исходный список блюд.
/// \return Список блюд, соответствующих выбранным кухням.
///
QList<Dish> CuisineFilter::apply(const QList<Dish>& dishes)
{
    if (cuisines.isEmpty() || cuisines.contains(CuisineType::ANY))
        return dishes;

    QList<Dish> result;

    for (const Dish& dish : dishes) {
        if (cuisines.contains(dish.cuisine)) {
            result.append(dish);
        }
    }

    return result;
}

///
/// \brief Последовательно применяет все фильтры к списку блюд.
/// \param dishes Исходный список блюд.
/// \param prefs Предпочтения пользователя.
/// \return Итоговый список блюд после применения фильтров.
///
QList<Dish> FilterManager::applyAll(const QList<Dish>& dishes, const Preferences& prefs)
{
    QList<Dish> filtered = dishes;

    IngredientFilter ingFilter(prefs.dislikedIngredients);
    filtered = ingFilter.apply(filtered);

    if (!prefs.favoriteCuisines.isEmpty()) {
        CuisineFilter cFilter(prefs.favoriteCuisines);
        filtered = cFilter.apply(filtered);
    }

    return filtered;
}