#ifndef FUNCTIONSTOSERVER_H
#define FUNCTIONSTOSERVER_H

#include <QString>
#include <QStringList>
#include <QTcpSocket>

///
/// \brief Класс для обработки команд, поступающих на сервер.
///
/// Класс определяет тип запроса от клиента и передает его
/// соответствующему обработчику: авторизации, регистрации,
/// подбору блюд, статистике и другим функциям.
///
/// На вход получает текстовую команду от клиента и идентификатор подключения.
/// На выходе возвращает строковый ответ, который сервер отправляет клиенту.
///
class FunctionsToServer
{
public:
    ///
    /// \brief Обрабатывает входящую команду клиента.
    /// \param request Текст запроса, полученный от клиента.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return Строка с результатом обработки команды.
    ///
    static QString parsing(const QString& request, qintptr socketId);
};

#endif // FUNCTIONSTOSERVER_H
