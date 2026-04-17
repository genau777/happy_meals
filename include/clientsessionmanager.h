#ifndef CLIENTSESSIONMANAGER_H
#define CLIENTSESSIONMANAGER_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QSet>
#include <QDateTime>
#include "db_singleton.h"

// 1. Структура блюда для клиента (отличается от серверной ClientDish в models.h)
struct ClientDish {
    QString title;
    QStringList ingredients;   // Список ингредиентов
    QStringList steps;         // Шаги приготовления
    QString cuisine;
    QString type;
    int complexity;
    int timeMinutes;
    QString description;       // Краткое описание
    QString imagePath;

    // Новые поля для подробностей
    int calories;              // Калории
    QString proteins;          // Белки
    QString fats;              // Жиры
    QString carbs;             // Углеводы
};

// 2. Структура критериев фильтрации (для "Движка фильтрации")
struct FilterCriteria {
    QStringList excludedIngredients;
    QString cuisine;
    QString type;
    int maxTime;
    int maxComplexity;
};

class ClientSessionManager : public QObject {
    Q_OBJECT

private:
    explicit ClientSessionManager(QObject *parent = nullptr) : QObject(parent) {
        // Конструктор - больше не нужна mock база данных
    }

    QSet<QString> favoriteTitles;
    QStringList searchHistory;
    QString currentUser; // Текущий авторизованный пользователь

public:
    QStringList getSearchHistory() const { return searchHistory; }
    static ClientSessionManager& instance() {
        static ClientSessionManager inst;
        return inst;
    }

    // --- Авторизация ---
    void requestLogin(const QString &log, const QString &pass) {
        // Используем реальную базу данных для авторизации
        bool success = DB_Singleton::getInstance()->auth(log, pass, 0);
        if (success) {
            currentUser = log;
            loadFavorites();
            emit loginResult(true, "Успех");
        } else {
            emit loginResult(false, "Неверный логин или пароль");
        }
    }
    
    // --- Регистрация ---
    void requestRegister(const QString &log, const QString &pass, const QString &email) {
        // Используем реальную базу данных для регистрации
        bool success = DB_Singleton::getInstance()->reg(log, pass, email, 0);
        if (success) {
            currentUser = log;
            loadFavorites();
            emit registerResult(true, "Регистрация успешна");
        } else {
            emit registerResult(false, "Логин уже занят");
        }
    }
    
    // --- Статистика ---
    QString getStatistics() const {
        if (currentUser.isEmpty()) {
            return "ERROR:Вы не авторизованы";
        }
        return DB_Singleton::getInstance()->get_stat_by_username(currentUser);
    }

    // --- Избранное ---
    void toggleFavorite(const QString& title) {
        if (currentUser.isEmpty()) return;
        
        if (favoriteTitles.contains(title)) {
            favoriteTitles.remove(title);
            DB_Singleton::getInstance()->removeFavorite(currentUser, title);
        } else {
            favoriteTitles.insert(title);
            DB_Singleton::getInstance()->addFavorite(currentUser, title);
        }
    }

    bool isFavorite(const QString& title) const {
        return favoriteTitles.contains(title);
    }
    
    void loadFavorites() {
        if (currentUser.isEmpty()) return;
        favoriteTitles.clear();
        QStringList favs = DB_Singleton::getInstance()->getFavorites(currentUser);
        for (const QString& fav : favs) {
            favoriteTitles.insert(fav);
        }
    }
    
    QStringList getFavoritesList() const {
        return favoriteTitles.values();
    }

    // --- История ---
    void addToHistory(const QString &query) {
        if (query.isEmpty()) return;
        QString entry = QDateTime::currentDateTime().toString("hh:mm") + " | Параметры: " + query;
        searchHistory.prepend(entry);
        if (searchHistory.size() > 15) searchHistory.removeLast();
    }
    
    void logSearchToDatabase(const QString &ingredient) {
        if (!currentUser.isEmpty() && !ingredient.isEmpty()) {
            DB_Singleton::getInstance()->log_search_by_username(currentUser, ingredient);
        }
    }

    QStringList getHistory() const { return searchHistory; }
    void clearHistory() { searchHistory.clear(); }

signals:
    void loginResult(bool success, const QString &msg);
    void registerResult(bool success, const QString &msg);
};

#endif // CLIENTSESSIONMANAGER_H