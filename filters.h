#ifndef FILTERS_H
#define FILTERS_H

#include "models.h"

class Filter
{
public:
	virtual ~Filter() = default;
	virtual QList<Dish> apply(const QList<Dish>& dishes) = 0;
};

class IngredientFilter : public Filter
{
private:
	QList<Ingredient> excluded;

public:
	IngredientFilter(const QList<Ingredient>& excludedIngredients);
	QList<Dish> apply(const QList<Dish>& dishes) override;
};

class CuisineFilter : public Filter
{
private:
	QList<CuisineType> cuisines;

public:
	CuisineFilter(const QList<CuisineType>& c) : cuisines(c) {}
	QList<Dish> apply(const QList<Dish>& dishes) override;
};

class FilterManager
{
public:
	QList<Dish> applyAll(const QList<Dish>& dishes, const Preferences& prefs);
};

#endif

