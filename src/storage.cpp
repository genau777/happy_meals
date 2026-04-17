#include "storage.h"

Storage::Storage() {
	// Ингредиенты
	Ingredient tomato { QUuid::createUuid(), "Помидор", IngredientCategory::VEGETABLES };
	Ingredient egg { QUuid::createUuid(), "Яйцо", IngredientCategory::OTHER };
	Ingredient chicken { QUuid::createUuid(), "Курица", IngredientCategory::MEAT };
	Ingredient rice { QUuid::createUuid(), "Рис", IngredientCategory::GRAINS };
	Ingredient salmon { QUuid::createUuid(), "Лосось", IngredientCategory::FISH };
	Ingredient cheese { QUuid::createUuid(), "Сыр", IngredientCategory::DAIRY };
	Ingredient pasta { QUuid::createUuid(), "Паста", IngredientCategory::GRAINS };
	Ingredient beef { QUuid::createUuid(), "Говядина", IngredientCategory::MEAT };
	Ingredient nori { QUuid::createUuid(), "Нори", IngredientCategory::OTHER };
	Ingredient avocado { QUuid::createUuid(), "Авокадо", IngredientCategory::VEGETABLES };
	Ingredient potato { QUuid::createUuid(), "Картофель", IngredientCategory::VEGETABLES };
	Ingredient onion { QUuid::createUuid(), "Лук", IngredientCategory::VEGETABLES };
	Ingredient carrot { QUuid::createUuid(), "Морковь", IngredientCategory::VEGETABLES };
	Ingredient tofu { QUuid::createUuid(), "Тофу", IngredientCategory::OTHER };

	m_ingredients.append({tomato, egg, chicken, rice, salmon, cheese, pasta, beef, nori, avocado, potato, onion, carrot, tofu});

	// Блюда с разными кухнями и временем приготовления
	Dish omelet { QUuid::createUuid(), "Омлет", CuisineType::RUSSIAN, DishType::SECOND_COURSE, {egg, tomato}, 10 };
	Dish caesar { QUuid::createUuid(), "Цезарь", CuisineType::ITALIAN, DishType::SALAD, {chicken, tomato, cheese}, 15 };
	Dish sushi { QUuid::createUuid(), "Суши", CuisineType::JAPANESE, DishType::SECOND_COURSE, {rice, salmon, nori}, 30 };
	Dish carbonara { QUuid::createUuid(), "Карбонара", CuisineType::ITALIAN, DishType::SECOND_COURSE, {pasta, egg, cheese}, 20 };
	Dish borscht { QUuid::createUuid(), "Борщ", CuisineType::RUSSIAN, DishType::FIRST_COURSE, {beef, potato, carrot, onion, tomato}, 60 };
	Dish friedRice { QUuid::createUuid(), "Жареный рис", CuisineType::CHINESE, DishType::SECOND_COURSE, {rice, egg, chicken, carrot}, 25 };
	Dish miso { QUuid::createUuid(), "Мисо-суп", CuisineType::JAPANESE, DishType::FIRST_COURSE, {nori, tofu}, 10 };
	Dish grilledSalmon { QUuid::createUuid(), "Лосось на гриле", CuisineType::JAPANESE, DishType::SECOND_COURSE, {salmon}, 15 };
	Dish beefSteak { QUuid::createUuid(), "Стейк", CuisineType::ANY, DishType::SECOND_COURSE, {beef}, 12 };
	Dish mashedPotato { QUuid::createUuid(), "Картофельное пюре", CuisineType::RUSSIAN, DishType::SECOND_COURSE, {potato}, 20 };

	m_dishes.append({omelet, caesar, sushi, carbonara, borscht, friedRice, miso, grilledSalmon, beefSteak, mashedPotato});
}

QList<Dish> Storage::getAllDishes() const { return m_dishes; }
QList<Ingredient> Storage::getAllIngredients() const { return m_ingredients; }
