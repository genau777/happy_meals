#ifndef DISHMANAGER_H
#define DISHMANAGER_H

#include <QString>
#include <QStringList>
#include <QTcpSocket>

///
/// \brief Класс для обработки запросов, связанных с подбором блюд.
///
/// Класс отвечает за получение параметров поиска от пользователя,
/// обработку этих параметров и формирование ответа со списком подходящих блюд.
///
/// На вход получает список параметров подбора и идентификатор клиентского подключения.
/// На выходе возвращает строку с результатом подбора блюд или сообщением об ошибке.
///
class DishManager
{
public:
    ///
    /// \brief Выполняет подбор блюда по параметрам пользователя.
    /// \param params Список параметров поиска: ингредиенты, кухня и максимальное время приготовления.
    /// \param socketId Идентификатор клиентского подключения.
    /// \return Строка с найденными блюдами или сообщение об ошибке.
    ///
    static QString get_dish(const QStringList& params, qintptr socketId);

    static QString dish_details(const QStringList& params, qintptr socketId);
};

#endif // DISHMANAGER_H
