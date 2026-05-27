#ifndef DISHSERVER_H
#define DISHSERVER_H

#include <QObject>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>

///
/// \brief Класс TCP-сервера приложения HappyMeals.
///
/// Класс отвечает за прием клиентских подключений, чтение входящих команд
/// и отправку ответов клиентам.
///
/// На вход получает сетевые подключения и запросы пользователей.
/// На выходе возвращает клиентам ответы по авторизации, регистрации,
/// подбору блюд, статистике и другим действиям.
///
class DishServer : public QObject
{
    Q_OBJECT

public:
    ///
    /// \brief Создает TCP-сервер.
    /// \param parent Родительский объект Qt.
    ///
    explicit DishServer(QObject *parent = nullptr);

    ///
    /// \brief Завершает работу сервера и освобождает ресурсы.
    ///
    ~DishServer();

public slots:
    ///
    /// \brief Обрабатывает новое подключение клиента.
    ///
    void slotNewConnection();

    ///
    /// \brief Обрабатывает отключение клиента.
    ///
    void slotClientDisconnected();

    ///
    /// \brief Обрабатывает входящие данные от клиента.
    ///
    void slotServerRead();

private:
    QTcpServer *mTcpServer;               ///< Объект TCP-сервера.
    QMap<qintptr, QTcpSocket*> m_clients; ///< Список подключенных клиентов.
};

#endif // DISHSERVER_H