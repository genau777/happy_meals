#include <QtTest/QtTest>

#include "filters.h"
#include "models.h"

class TestIngredientFilter : public QObject
{
    Q_OBJECT

private slots:
    void testExcludeDishWithDislikedIngredient();
    void testReturnAllDishesIfExcludedListIsEmpty();
};

void TestIngredientFilter::testExcludeDishWithDislikedIngredient()
{
    // Подготовка входных данных

    Ingredient egg{
        QUuid::createUuid(),
        "Яйцо",
        IngredientCategory::OTHER};

    Ingredient rice{
        QUuid::createUuid(),
        "Рис",
        IngredientCategory::GRAINS};

    Dish omelet{
        QUuid::createUuid(),
        "Омлет",
        CuisineType::RUSSIAN,
        DishType::SECOND_COURSE,
        {egg},
        10};

    Dish ricePorridge{
        QUuid::createUuid(),
        "Рисовая каша",
        CuisineType::RUSSIAN,
        DishType::SECOND_COURSE,
        {rice},
        20};

    QList<Dish> dishes;
    dishes.append(omelet);
    dishes.append(ricePorridge);

    QList<Ingredient> excludedIngredients;
    excludedIngredients.append(egg);

    // Выполнение тестируемой функции

    IngredientFilter filter(excludedIngredients);
    QList<Dish> result = filter.apply(dishes);

    // Проверка результата

    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].name, QString("Рисовая каша"));
}

void TestIngredientFilter::testReturnAllDishesIfExcludedListIsEmpty()
{
    // Подготовка входных данных

    Ingredient egg{
        QUuid::createUuid(),
        "Яйцо",
        IngredientCategory::OTHER};

    Ingredient rice{
        QUuid::createUuid(),
        "Рис",
        IngredientCategory::GRAINS};

    Dish omelet{
        QUuid::createUuid(),
        "Омлет",
        CuisineType::RUSSIAN,
        DishType::SECOND_COURSE,
        {egg},
        10};

    Dish ricePorridge{
        QUuid::createUuid(),
        "Рисовая каша",
        CuisineType::RUSSIAN,
        DishType::SECOND_COURSE,
        {rice},
        20};

    QList<Dish> dishes;
    dishes.append(omelet);
    dishes.append(ricePorridge);

    QList<Ingredient> excludedIngredients;

    // Выполнение тестируемой функции

    IngredientFilter filter(excludedIngredients);
    QList<Dish> result = filter.apply(dishes);

    // Проверка результата

    QCOMPARE(result.size(), 2);
    QCOMPARE(result[0].name, QString("Омлет"));
    QCOMPARE(result[1].name, QString("Рисовая каша"));
}

QTEST_MAIN(TestIngredientFilter)

#include "test_ingredientfilter.moc"