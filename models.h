#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QUuid>
#include <QList>

enum class IngredientCategory { VEGETABLES, MEAT, FISH, DAIRY, GRAINS, OTHER };
enum class CuisineType { JAPANESE, ITALIAN, RUSSIAN, CHINESE, MEXICAN, ANY };
enum class DishType { FIRST_COURSE, SECOND_COURSE, SALAD, DESSERT, ANY };

struct Ingredient {
	QUuid id;
	QString name;
	IngredientCategory category;

	bool operator==(const Ingredient& other) const { return id == other.id; }
};

struct Dish {
	QUuid id;
	QString name;
	CuisineType cuisine;
	DishType type;
	QList<Ingredient> ingredients;

	int prepTime; // время приготовления в минутах
};

// Предпочтения
struct Preferences {
	QList<CuisineType> favoriteCuisines;
	QList<DishType> favoriteDishTypes;
	QList<Ingredient> dislikedIngredients;
};

#endif
