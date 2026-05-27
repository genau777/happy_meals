#include "dishserver.h"
#include "functionstoserver.h"
#include "db_singleton.h"

#include <QDebug>
#include <QCoreApplication>
#include <QNetworkProxy>
#include <QString>
#include <QVariant>

///
/// \brief Завершает работу сервера и закрывает соединения клиентов.
///
DishServer::~DishServer() {
    for(QTcpSocket* socket : m_clients.values()) {
        socket->close();
    }
    mTcpServer->close();
}

///
/// \brief Создает и запускает TCP-сервер приложения.
/// \param parent Родительский объект Qt.
///
DishServer::DishServer(QObject *parent) : QObject(parent){
    const quint16 port = 40000;

    mTcpServer = new QTcpServer(this);
    mTcpServer->setProxy(QNetworkProxy::NoProxy);
    connect(mTcpServer, &QTcpServer::newConnection, this, &DishServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::AnyIPv4, port)){
        qDebug() << "Server is not started:" << mTcpServer->errorString();
    } else {
        qDebug() << "Server is started on port" << port;
    }
}

///
/// \brief Обрабатывает новое подключение клиента.
///
void DishServer::slotNewConnection(){
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();
    qintptr desc = clientSocket->socketDescriptor(); // Используем qintptr по UML

    // Сохраняем дескриптор в свойствах сокета (преобразуем в qulonglong для QVariant)
    clientSocket->setProperty("descriptor", static_cast<qulonglong>(desc));
    m_clients.insert(desc, clientSocket);

    qDebug() << "New client connect! Descriptor:" << desc << "Total clients:" << m_clients.size();

    connect(clientSocket, &QTcpSocket::readyRead, this, &DishServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &DishServer::slotClientDisconnected);
}

///
/// \brief Считывает запрос клиента и отправляет ответ сервера.
///
void DishServer::slotServerRead(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    while (clientSocket->bytesAvailable() > 0) {
        QByteArray data = clientSocket->readAll();
        QString request = QString::fromUtf8(data).trimmed();
        qintptr desc = clientSocket->socketDescriptor();

        qDebug().nospace() << "Client " << desc << " sent: " << request;

        // Передаем запрос в Обработчик Команд (строго по UML)
        QString response = FunctionsToServer::parsing(request, desc);

        clientSocket->write((response + "\n").toUtf8());
    }
}

///
/// \brief Обрабатывает отключение клиента от сервера.
///
void DishServer::slotClientDisconnected(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());

    if(clientSocket) {
        // Достаем дескриптор обратно
        qintptr desc = static_cast<qintptr>(clientSocket->property("descriptor").toULongLong());
        qDebug() << "Client disconnected. Descriptor:" << desc;

        // Вызываем правильный Синглтон из UML
        DB_Singleton::getInstance()->clear_socket_id(desc);

        m_clients.remove(desc);
        clientSocket->deleteLater();
    }
}
