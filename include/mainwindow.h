#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QTextBrowser>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // --- Вспомогательные методы для обновления интерфейса ---
    void updateFavoritesList(); // Обновление экрана избранного
    void updateHistoryList();   // Обновление экрана истории
    void updateStatistics();    // Обновление экрана статистики

    Ui::MainWindow *ui;

    // --- Элементы интерфейса ---

    // Авторизация
    QLineEdit *logEdit;
    QLineEdit *passEdit;
    QLineEdit *emailEdit;
    QPushButton *btnEnter;
    QPushButton *btnRegister;
    QPushButton *btnStartSearch;

    // Поиск и фильтры (UC13, UC5, UC6, UC8)
    QListWidget *ingredientsList;
    QComboBox *cuisineBox;
    QComboBox *typeBox;
    QComboBox *complexityBox;
    QSlider *timeSlider;
    QPushButton *btnSearch;
    QPushButton *btnGoToFavorites;
    QPushButton *btnShowHistory;
    QPushButton *btnShowStats;
    
    // Статистика
    QLabel *statsLabel;

    // Результаты и Рецепт (UC9)
    QListWidget *resultsList;
    QTextBrowser *dishDescription; // Изменено на QTextBrowser для HTML
    QPushButton *btnBack;
    QPushButton *btnFavorite;
    QPushButton *btnBackToResults;
    QPushButton *btnBackFromFav;    // Исправлено: добавлено объявление

    // Списки избранного и истории
    QListWidget *favoritesList;    // Исправлено: добавлено объявление
    QListWidget *historyList;      // Исправлено: добавлено объявление
    QPushButton *btnHistoryBack;   // Исправлено: добавлено объявление
};

#endif // MAINWINDOW_H