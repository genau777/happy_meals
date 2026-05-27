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

///
/// \brief Главный класс пользовательского интерфейса приложения.
///
/// Класс отвечает за отображение основных экранов приложения:
/// авторизацию, регистрацию, подбор блюд, просмотр рецептов,
/// избранное, историю и статистику.
///
/// На вход получает действия пользователя через элементы интерфейса.
/// На выходе обновляет состояние окон, списков, описаний блюд и пользовательских данных.
///
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    ///
    /// \brief Создает главное окно приложения.
    /// \param parent Родительский виджет Qt.
    ///
    MainWindow(QWidget *parent = nullptr);

    ///
    /// \brief Уничтожает главное окно приложения.
    ///
    ~MainWindow();

private:
    ///
    /// \brief Обновляет список избранных блюд на экране.
    ///
    void updateFavoritesList();

    ///
    /// \brief Обновляет список истории пользовательских действий.
    ///
    void updateHistoryList();

    ///
    /// \brief Обновляет статистику пользователя.
    ///
    void updateStatistics();

    Ui::MainWindow *ui; ///< Указатель на объект интерфейса, созданный из .ui файла.

    // --- Элементы интерфейса ---

    // Авторизация
    QLineEdit *logEdit;          ///< Поле ввода логина.
    QLineEdit *passEdit;         ///< Поле ввода пароля.
    QLineEdit *emailEdit;        ///< Поле ввода электронной почты.
    QPushButton *btnEnter;       ///< Кнопка входа в аккаунт.
    QPushButton *btnRegister;    ///< Кнопка регистрации.
    QPushButton *btnStartSearch; ///< Кнопка перехода к поиску блюд.

    // Поиск и фильтры (UC13, UC5, UC6, UC8)
    QListWidget *ingredientsList;  ///< Список ингредиентов для выбора пользователем.
    QComboBox *cuisineBox;         ///< Выпадающий список выбора кухни.
    QComboBox *typeBox;            ///< Выпадающий список выбора типа блюда.
    QComboBox *complexityBox;      ///< Выпадающий список выбора сложности приготовления.
    QSlider *timeSlider;           ///< Ползунок выбора максимального времени приготовления.
    QPushButton *btnSearch;        ///< Кнопка запуска поиска блюд.
    QPushButton *btnGoToFavorites; ///< Кнопка перехода к избранному.
    QPushButton *btnShowHistory;   ///< Кнопка перехода к истории.
    QPushButton *btnShowStats;     ///< Кнопка перехода к статистике.

    // Статистика
    QLabel *statsLabel; ///< Надпись для отображения статистики пользователя.

    // Результаты и Рецепт (UC9)
    QListWidget *resultsList;       ///< Список найденных блюд.
    QTextBrowser *dishDescription;  ///< Поле отображения подробного HTML-описания рецепта.
    QPushButton *btnBack;           ///< Кнопка возврата назад.
    QPushButton *btnFavorite;       ///< Кнопка добавления блюда в избранное.
    QPushButton *btnBackToResults;  ///< Кнопка возврата к списку результатов.
    QPushButton *btnBackFromFav;    ///< Кнопка возврата из экрана избранного.

    // Списки избранного и истории
    QListWidget *favoritesList;   ///< Список избранных блюд пользователя.
    QListWidget *historyList;     ///< Список истории поиска пользователя.
    QPushButton *btnHistoryBack;  ///< Кнопка возврата из экрана истории.
};

#endif // MAINWINDOW_H