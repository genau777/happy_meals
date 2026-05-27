#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QStringList>

///
/// \brief Класс для обработки пользовательских команд.
///
/// Класс отвечает за авторизацию, регистрацию и получение статистики пользователя.
/// Используется как промежуточный слой между серверными запросами и базой данных.
///
/// На вход получает параметры команды и идентификатор клиентского подключения.
/// На выходе возвращает строковый ответ с результатом операции.
///
class UserManager {
public:
    ///
    /// \brief Выполняет авторизацию пользователя.
    /// \param params Список параметров авторизации: логин и пароль.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return Строка с результатом авторизации.
    ///
    static QString auth(const QStringList& params, qintptr socketId);

    ///
    /// \brief Выполняет регистрацию пользователя.
    /// \param params Список параметров регистрации: логин, пароль и email.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return Строка с результатом регистрации.
    ///
    static QString reg(const QStringList& params, qintptr socketId);

    ///
    /// \brief Возвращает статистику пользователя.
    /// \param params Список параметров запроса статистики.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return Строка со статистикой пользователя.
    ///
    static QString get_stat(const QStringList& params, qintptr socketId);

    static QString add_favorite(const QStringList& params, qintptr socketId);
    static QString remove_favorite(const QStringList& params, qintptr socketId);
    static QString get_favorites(const QStringList& params, qintptr socketId);
    static QString get_history(const QStringList& params, qintptr socketId);
};

#endif // USERMANAGER_H
