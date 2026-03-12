#include "storage.h"

Storage::Storage() {
	Ingredient tomato { QUuid::createUuid(), "Помидор", IngredientCategory::VEGETABLES };
	Ingredient egg { QUuid::createUuid(), "Яйцо", IngredientCategory::OTHER };
	Ingredient chicken { QUuid::createUuid(), "Курица", IngredientCategory::MEAT };

	m_ingredients.append({tomato, egg, chicken});

	Dish omelet { QUuid::createUuid(), "Омлет", CuisineType::RUSSIAN, DishType::SECOND_COURSE, {egg, tomato}, 10 };
	Dish salad { QUuid::createUuid(), "Цезарь", CuisineType::ITALIAN, DishType::SALAD, {chicken, tomato}, 15 };

	m_dishes.append({omelet, salad});
}

QList<Dish> Storage::getAllDishes() const { return m_dishes; }
QList<Ingredient> Storage::getAllIngredients() const { return m_ingredients; }
