#ifndef DB_SINGLETON_H
#define DB_SINGLETON_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QTcpSocket>

#include "models.h"

class DB_Singleton;

///
/// \brief Класс для удаления singleton-объекта DB_Singleton.
///
/// Используется для корректного освобождения памяти,
/// занятой единственным экземпляром класса DB_Singleton.
///
class DB_SingletonDestroyer
{
private:
    DB_Singleton *p_instance = nullptr; ///< Указатель на экземпляр DB_Singleton.

public:
    ///
    /// \brief Удаляет сохраненный экземпляр DB_Singleton.
    ///
    ~DB_SingletonDestroyer();

    ///
    /// \brief Сохраняет указатель на экземпляр DB_Singleton.
    /// \param p Указатель на объект DB_Singleton.
    ///
    void initialize(DB_Singleton *p);
};

///
/// \brief Класс для централизованной работы с базами данных приложения.
///
/// Класс отвечает за подключение к SQLite-базам, регистрацию и авторизацию
/// пользователей, хранение истории поиска, работу с избранными блюдами
/// и получение информации о рецептах.
///
/// На вход получает данные пользователя, параметры поиска и названия блюд.
/// На выходе возвращает результаты операций, списки блюд, статистику
/// или подробное описание рецепта.
///
class DB_Singleton
{
private:
    static DB_Singleton *instance;          ///< Единственный экземпляр класса.
    static DB_SingletonDestroyer destroyer; ///< Объект для удаления singleton-экземпляра.

    QSqlDatabase db;       ///< База данных пользователей, истории и избранного.
    QSqlDatabase dishesDb; ///< База данных блюд и ингредиентов.

    ///
    /// \brief Создает объект подключения к базам данных.
    ///
    DB_Singleton();

    DB_Singleton(const DB_Singleton &) = delete;
    DB_Singleton &operator=(const DB_Singleton &) = delete;

    ///
    /// \brief Инициализирует таблицы базы данных блюд.
    ///
    void initDishesDatabase();

    ///
    /// \brief Заполняет базу данных начальными блюдами и ингредиентами.
    ///
    void populateDishesData();

    friend class DB_SingletonDestroyer;

public:
    ///
    /// \brief Закрывает подключения к базам данных.
    ///
    ~DB_Singleton();

    ///
    /// \brief Возвращает единственный экземпляр DB_Singleton.
    /// \return Указатель на объект DB_Singleton.
    ///
    static DB_Singleton *getInstance();

    ///
    /// \brief Авторизует пользователя.
    /// \param login Логин пользователя.
    /// \param pass Пароль пользователя.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return true, если авторизация успешна, иначе false.
    ///
    int authUserId(QString login, QString pass);
    bool userExists(QString login);

    ///
    /// \brief Регистрирует нового пользователя.
    /// \param login Логин пользователя.
    /// \param pass Пароль пользователя.
    /// \param email Электронная почта пользователя.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return true, если регистрация успешна, иначе false.
    ///
    bool reg(QString login, QString pass, QString email);

    ///
    /// \brief Сохраняет поисковый запрос пользователя в историю.
    /// \param socketId Идентификатор клиентского подключения.
    /// \param ingredient Ингредиент или поисковая строка.
    ///
    void log_search_for_user_id(int userId, QString summary);
    void log_search_for_user(QString login, QString summary);
    QStringList get_search_history_for_user_id(int userId);
    QStringList get_search_history_for_user(QString login);

    ///
    /// \brief Сохраняет поисковый запрос пользователя по логину.
    /// \param username Логин пользователя.
    /// \param ingredient Ингредиент или поисковая строка.
    ///
    void log_search_by_username(QString username, QString ingredient);

    ///
    /// \brief Возвращает статистику пользователя по socketId.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return Строка со статистикой или сообщением об ошибке.
    ///
    QString get_stat_for_user_id(int userId);
    QString get_stat_for_user(QString login);
    int startUserSession(int userId);
    void endUserSession(int userId);

    ///
    /// \brief Возвращает статистику пользователя по логину.
    /// \param username Логин пользователя.
    /// \return Строка со статистикой или сообщением об ошибке.
    ///
    QString get_stat_by_username(QString username);

    ///
    /// \brief Добавляет блюдо в избранное пользователя.
    /// \param username Логин пользователя.
    /// \param dishName Название блюда.
    /// \return true, если блюдо добавлено, иначе false.
    ///
    bool addFavorite(QString username, QString dishName);
    bool addFavorite(int userId, QString dishName);

    ///
    /// \brief Удаляет блюдо из избранного пользователя.
    /// \param username Логин пользователя.
    /// \param dishName Название блюда.
    /// \return true, если блюдо удалено, иначе false.
    ///
    bool removeFavorite(QString username, QString dishName);
    bool removeFavorite(int userId, QString dishName);

    ///
    /// \brief Возвращает список избранных блюд пользователя.
    /// \param username Логин пользователя.
    /// \return Список названий избранных блюд.
    ///
    QStringList getFavorites(QString username);
    QStringList getFavorites(int userId);

    ///
    /// \brief Фильтрует блюда по параметрам пользователя.
    /// \param excludedIngredients Список исключаемых ингредиентов.
    /// \param cuisines Список выбранных кухонь.
    /// \param maxTime Максимальное время приготовления.
    /// \return Список блюд, подходящих под фильтры.
    ///
    QList<Dish> filterDishes(const QStringList &excludedIngredients,
                             const QStringList &cuisines,
                             const QStringList &dishTypes,
                             int maxComplexity,
                             int maxTime);

    ///
    /// \brief Возвращает подробное описание блюда.
    /// \param dishName Название блюда.
    /// \return HTML-строка с описанием рецепта.
    ///
    QString getDishDetails(QString dishName);
};

#endif // DB_SINGLETON_H
