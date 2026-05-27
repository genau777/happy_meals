#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

///
/// \brief Класс экрана выбора ингредиентов.
///
/// Класс отвечает за отображение списка ингредиентов и переход
/// к следующему этапу работы приложения.
///
/// На вход получает родительский виджет Qt.
/// На выходе формирует экран со списком ингредиентов и кнопкой продолжения.
///
class IngredientsScreen : public QWidget
{
    Q_OBJECT

private:
    QListWidget *list;       ///< Список ингредиентов, отображаемый пользователю.
    QPushButton *buttonNext; ///< Кнопка перехода к следующему экрану.

public:
    ///
    /// \brief Создает экран выбора ингредиентов.
    /// \param parent Родительский виджет Qt.
    ///
    IngredientsScreen(QWidget *parent = nullptr);
};