#include "filters.h"

IngredientFilter::IngredientFilter(const QList<Ingredient>& excludedIngredients) : excluded(excludedIngredients) {}

QList<Dish> IngredientFilter::apply(const QList<Dish>& dishes) {
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

QList<Dish> CuisineFilter::apply(const QList<Dish>& dishes) {
	return dishes;
}

QList<Dish> FilterManager::applyAll(const QList<Dish>& dishes, const Preferences& prefs) {
	QList<Dish> filtered = dishes;

	IngredientFilter ingFilter(prefs.dislikedIngredients);
	filtered = ingFilter.apply(filtered);

	// for the future
	// CuisineFilter cFilter(prefs.favoriteCuisines)
	// filtered = cFilter.apply(filtered);
	
	return filtered;
}
