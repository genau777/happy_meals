#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class IngredientsScreen : public QWidget
{
    Q_OBJECT

private:
    QListWidget *list;
    QPushButton *buttonNext;

public:
    IngredientsScreen(QWidget *parent = nullptr);
};