#ifndef STORAGE_H
#define STORAGE_H

#include "models.h"

class Storage {
private:
	QList<Dish> m_dishes;
	QList<Ingredient> m_ingredients;

public:
	Storage();
	QList<Dish> getAllDishes() const;
	QList<Ingredient> getAllIngredients() const;
};

#endif
