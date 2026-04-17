#include "ingredientsscreen.h"
#include <QLabel>

IngredientsScreen::IngredientsScreen(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Выберите ингредиенты");
    layout->addWidget(title);

    list = new QListWidget(this);

    // тестовые данные
    list->addItem("Картофель");
    list->addItem("Курица");
    list->addItem("Рис");
    list->addItem("Помидор");

    layout->addWidget(list);

    buttonNext = new QPushButton("Далее");
    layout->addWidget(buttonNext);
}