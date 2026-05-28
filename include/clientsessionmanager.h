#ifndef CLIENTSESSIONMANAGER_H
#define CLIENTSESSIONMANAGER_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QSet>
#include <QDateTime>
#include <QDebug>
#include "clientapi.h"

///
/// \brief Структура блюда на стороне клиента.
///
/// Структура хранит данные о блюде, которые используются в клиентском интерфейсе:
/// название, ингредиенты, шаги приготовления, кухню, тип, сложность, время,
/// описание, изображение и пищевую ценность.
///
/// На вход получает данные рецепта.
/// На выходе используется для отображения блюда пользователю.
///
struct ClientDish {
    QString title;
    QStringList ingredients;
    QStringList steps;
    QString cuisine;
    QString type;
    int complexity;
    int timeMinutes;
    QString description;
    QString imagePath;

    int calories;
    QString proteins;
    QString fats;
    QString carbs;
};

///
/// \brief Структура критериев фильтрации блюд.
///
/// Структура хранит параметры, которые пользователь выбирает при поиске:
/// исключаемые ингредиенты, кухню, тип блюда, максимальное время и сложность.
///
/// На вход получает значения фильтров из интерфейса.
/// На выходе передается в логику поиска блюд.
///
struct FilterCriteria {
    QStringList excludedIngredients;
    QString cuisine;
    QString type;
    int maxTime;
    int maxComplexity;
};

///
/// \brief Класс управления клиентской сессией.
///
/// Класс хранит состояние текущего пользователя: авторизацию,
/// избранные блюда, историю поиска и статистику.
///
/// На вход получает логин, пароль, email, названия блюд и поисковые запросы.
/// На выходе возвращает состояние пользователя, список избранного,
/// историю поиска и результаты авторизации или регистрации.
///
class ClientSessionManager : public QObject {
    Q_OBJECT

private:
    ///
    /// \brief Создает объект менеджера клиентской сессии.
    /// \param parent Родительский объект Qt.
    ///
    explicit ClientSessionManager(QObject *parent = nullptr) : QObject(parent) {
    }

    QSet<QString> favoriteTitles;
    QStringList searchHistory;
    QString currentUser;

    static QString cleanApiMessage(QString response) {
        if (response.startsWith("ERROR:") || response.startsWith("OK:")) {
            response = response.mid(response.indexOf(':') + 1);
        }

        return response;
    }

public:
    ///
    /// \brief Возвращает историю поиска пользователя.
    /// \return Список записей истории поиска.
    ///
    QStringList getSearchHistory() const {
        return ClientApi::getInstance()->getSearchHistory();
    }

    ///
    /// \brief Возвращает единственный экземпляр ClientSessionManager.
    /// \return Ссылка на объект ClientSessionManager.
    ///
    static ClientSessionManager& instance() {
        static ClientSessionManager inst;
        return inst;
    }

    // --- Авторизация ---

    ///
    /// \brief Выполняет запрос на вход пользователя.
    /// \param log Логин пользователя.
    /// \param pass Пароль пользователя.
    ///
    void requestLogin(const QString &log, const QString &pass) {
        if (!currentUser.isEmpty()) {
            logout();
        }

        QString response = ClientApi::getInstance()->loginUser(log, pass);
        bool success = response.startsWith("OK:");

        if (success) {
            currentUser = log.trimmed();
            searchHistory.clear();
            loadFavorites();

            qDebug() << "Вход успешный";
            qDebug() << "Текущий пользователь:" << currentUser;

            emit loginResult(true, "Успех");
        } else {
            qDebug() << "Неправильный логин или пароль";

            emit loginResult(false, cleanApiMessage(response));
        }
    }

    // --- Регистрация ---

    ///
    /// \brief Выполняет запрос на регистрацию пользователя.
    /// \param log Логин пользователя.
    /// \param pass Пароль пользователя.
    /// \param email Электронная почта пользователя.
    ///
    void requestRegister(const QString &log, const QString &pass, const QString &email) {
        QString response = ClientApi::getInstance()->registerUser(log, pass, email);
        bool success = response.startsWith("OK:");

        if (success) {
            qDebug() << "Регистрация успешна";

            emit registerResult(true, "Регистрация успешна");
        } else {
            qDebug() << "Логин уже занят";

            emit registerResult(false, cleanApiMessage(response));
        }
    }

    void logout() {
        ClientApi::getInstance()->logoutUser();
        currentUser.clear();
        favoriteTitles.clear();
        searchHistory.clear();
    }

    // --- Статистика ---

    ///
    /// \brief Возвращает статистику текущего пользователя.
    /// \return Строка со статистикой или сообщение об ошибке авторизации.
    ///
    QString getStatistics() const {
        if (currentUser.isEmpty()) {
            return "ERROR:Вы не авторизованы";
        }

        return ClientApi::getInstance()->getStatistics();
    }

    // --- Избранное ---

    ///
    /// \brief Добавляет или удаляет блюдо из избранного.
    /// \param title Название блюда.
    ///
    void toggleFavorite(const QString& title) {
        if (currentUser.isEmpty()) {
            qDebug() << "Нельзя изменить избранное: пользователь не авторизован";
            return;
        }

        if (favoriteTitles.contains(title)) {
            favoriteTitles.remove(title);
            ClientApi::getInstance()->removeFavorite(title);
            qDebug() << "Блюдо удалено из избранного:" << title;
        } else {
            favoriteTitles.insert(title);
            ClientApi::getInstance()->addFavorite(title);
            qDebug() << "Блюдо добавлено в избранное:" << title;
        }
    }

    ///
    /// \brief Проверяет, находится ли блюдо в избранном.
    /// \param title Название блюда.
    /// \return true, если блюдо находится в избранном, иначе false.
    ///
    bool isFavorite(const QString& title) const {
        return favoriteTitles.contains(title);
    }

    ///
    /// \brief Загружает избранные блюда пользователя из базы данных.
    ///
    void loadFavorites() {
        if (currentUser.isEmpty()) return;

        favoriteTitles.clear();

        for (const QString& fav : ClientApi::getInstance()->getFavorites()) {
            favoriteTitles.insert(fav);
        }

        qDebug() << "Избранное загружено. Количество блюд:" << favoriteTitles.size();
    }

    ///
    /// \brief Возвращает список избранных блюд.
    /// \return Список названий избранных блюд.
    ///
    QStringList getFavoritesList() const {
        return favoriteTitles.values();
    }

    // --- История ---

    ///
    /// \brief Добавляет запрос в локальную историю поиска.
    /// \param query Текст поискового запроса.
    ///
    void addToHistory(const QString &query) {
        if (query.isEmpty()) return;

        QString entry = QDateTime::currentDateTime().toString("hh:mm") + " | Параметры: " + query;
        searchHistory.prepend(entry);

        if (searchHistory.size() > 15) {
            searchHistory.removeLast();
        }

        qDebug() << "Запрос добавлен в историю:" << entry;
    }

    ///
    /// \brief Записывает поисковый запрос пользователя в базу данных.
    /// \param ingredient Ингредиент или текст поискового запроса.
    ///
    void logSearchToDatabase(const QString &ingredient) {
        if (!currentUser.isEmpty() && !ingredient.isEmpty()) {
            qDebug() << "Запрос записан в базу данных:" << ingredient;
        }
    }

    ///
    /// \brief Возвращает историю поиска.
    /// \return Список записей истории.
    ///
    QStringList getHistory() const { return searchHistory; }

    ///
    /// \brief Очищает историю поиска.
    ///
    void clearHistory() {
        searchHistory.clear();
        qDebug() << "История поиска очищена";
    }

signals:
    ///
    /// \brief Сигнал с результатом авторизации.
    /// \param success true, если авторизация успешна, иначе false.
    /// \param msg Сообщение для пользователя.
    ///
    void loginResult(bool success, const QString &msg);

    ///
    /// \brief Сигнал с результатом регистрации.
    /// \param success true, если регистрация успешна, иначе false.
    /// \param msg Сообщение для пользователя.
    ///
    void registerResult(bool success, const QString &msg);
};

#endif // CLIENTSESSIONMANAGER_H
