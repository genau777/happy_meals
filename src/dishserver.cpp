#include "dishserver.h"
#include "functionstoserver.h"
#include "db_singleton.h"

#include <QDebug>
#include <QCoreApplication>
#include <QString>
#include <QVariant>

DishServer::~DishServer() {
    for(QTcpSocket* socket : m_clients.values()) {
        socket->close();
    }
    mTcpServer->close();
}

DishServer::DishServer(QObject *parent) : QObject(parent){
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &DishServer::slotNewConnection);
    
    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "Server is not started";
    } else {
        qDebug() << "Server is started on port 33333";
    }
}

void DishServer::slotNewConnection(){
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();
    qintptr desc = clientSocket->socketDescriptor(); // Используем qintptr по UML
    
    // Сохраняем дескриптор в свойствах сокета (преобразуем в qulonglong для QVariant)
    clientSocket->setProperty("descriptor", static_cast<qulonglong>(desc));
    m_clients.insert(desc, clientSocket);

    qDebug() << "New client connect! Descriptor:" << desc << "Total clients:" << m_clients.size();

    clientSocket->write("Добро пожаловать в HappyMeals! Отправьте команду.\r\n");

    connect(clientSocket, &QTcpSocket::readyRead, this, &DishServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &DishServer::slotClientDisconnected);
}

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
