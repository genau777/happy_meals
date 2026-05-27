#ifndef STORAGE_H
#define STORAGE_H

#include "models.h"

///
/// \brief Класс локального хранилища блюд и ингредиентов.
///
/// Класс содержит списки блюд и ингредиентов, которые используются
/// для тестовой загрузки данных и дальнейшей фильтрации.
///
/// На вход получает данные при инициализации хранилища.
/// На выходе возвращает списки доступных блюд и ингредиентов.
///
class Storage {
private:
    QList<Dish> m_dishes;              ///< Список блюд.
    QList<Ingredient> m_ingredients;   ///< Список ингредиентов.

public:
    ///
    /// \brief Создает хранилище и инициализирует данные.
    ///
    Storage();

    ///
    /// \brief Возвращает список всех блюд.
    /// \return Список объектов Dish.
    ///
    QList<Dish> getAllDishes() const;

    ///
    /// \brief Возвращает список всех ингредиентов.
    /// \return Список объектов Ingredient.
    ///
    QList<Ingredient> getAllIngredients() const;
};

#endif