#ifndef CLIENTAPI_H
#define CLIENTAPI_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include <QJsonObject>

class ClientApi;

///
/// \brief Класс для удаления singleton-объекта ClientApi.
///
/// Класс хранит указатель на единственный экземпляр ClientApi
/// и отвечает за корректное освобождение памяти при завершении работы программы.
///
class ClientApiDestroyer
{
private:
    ClientApi* p_instance = nullptr; ///< Указатель на экземпляр ClientApi.

public:
    ///
    /// \brief Удаляет сохраненный экземпляр ClientApi.
    ///
    ~ClientApiDestroyer();

    ///
    /// \brief Сохраняет указатель на экземпляр ClientApi.
    /// \param p Указатель на объект ClientApi.
    ///
    void initialize(ClientApi* p);
};

///
/// \brief Класс клиентского API приложения HappyMeals.
///
/// Класс является единой точкой доступа клиентской части к основным функциям:
/// подключению к серверу, регистрации пользователя, поиску блюд
/// и получению подробного описания рецепта.
///
/// На вход получает данные из интерфейса пользователя.
/// На выходе возвращает результат выполнения операций, список найденных блюд
/// или HTML-описание выбранного рецепта.
///
class ClientApi : public QObject
{
    Q_OBJECT

private:
    static ClientApi* p_instance;          ///< Единственный экземпляр ClientApi.
    static ClientApiDestroyer destroyer;   ///< Объект для удаления singleton-экземпляра.

    QTcpSocket* sock;                      ///< Сокет для подключения к серверу.
    QString serverHost;
    quint16 serverPort;

    QString sendCommand(const QString& command);
    QJsonObject sendJsonCommand(const QJsonObject& request);

protected:
    ///
    /// \brief Создает объект клиентского API.
    /// \param parent Родительский объект Qt.
    ///
    explicit ClientApi(QObject* parent = nullptr);

    ClientApi(const ClientApi&) = delete;
    ClientApi& operator=(const ClientApi&) = delete;

    ///
    /// \brief Удаляет объект клиентского API.
    ///
    ~ClientApi() override;

    friend class ClientApiDestroyer;

public:
    ///
    /// \brief Возвращает единственный экземпляр ClientApi.
    /// \return Указатель на объект ClientApi.
    ///
    static ClientApi* getInstance();

    ///
    /// \brief Выполняет подключение к серверу.
    /// \param host Адрес сервера.
    /// \param port Порт сервера.
    /// \return true, если подключение успешно, иначе false.
    ///
    bool connectToServer(const QString& host = "127.0.0.1", quint16 port = 40000);

    ///
    /// \brief Регистрирует нового пользователя.
    /// \param login Логин пользователя.
    /// \param password Пароль пользователя.
    /// \param email Электронная почта пользователя.
    /// \return Строка с результатом регистрации.
    ///
    QString registerUser(const QString& login,
                         const QString& password,
                         const QString& email);

    QString loginUser(const QString& login,
                      const QString& password);

    ///
    /// \brief Выполняет поиск блюд по заданным параметрам.
    /// \param excludedIngredients Список исключаемых ингредиентов.
    /// \param cuisine Выбранная кухня.
    /// \param type Тип блюда.
    /// \param maxTime Максимальное время приготовления.
    /// \param maxComplexity Максимальная сложность приготовления.
    /// \return Список названий найденных блюд.
    ///
    QStringList findDishes(const QStringList& excludedIngredients,
                           const QString& cuisine,
                           const QString& type,
                           int maxTime,
                           int maxComplexity);

    ///
    /// \brief Возвращает подробное описание блюда.
    /// \param dishName Название блюда.
    /// \return HTML-строка с описанием рецепта.
    ///
    QString getDishDetails(const QString& dishName);

    QString getStatistics();

    QStringList getSearchHistory();
    QStringList getFavorites();
    QString addFavorite(const QString& dishName);
    QString removeFavorite(const QString& dishName);
};

#endif // CLIENTAPI_H
