#include "dishserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QString>

DishServer::~DishServer() {
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
    mTcpSocket = mTcpServer->nextPendingConnection();
    mTcpSocket->write("Hello, I am Dish Chooser Server!\r\n");
    connect(mTcpSocket, &QTcpSocket::readyRead, this, &DishServer::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &DishServer::slotClientDisconnected);
}

void DishServer::slotServerRead(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    while (clientSocket -> bytesAvailable() > 0) {
        QByteArray data = clientSocket->readAll();
        QString request = QString::fromUtf8(data).trimmed();

		qDebug() << "Отправленные пользователем ингредиенты:" << request;
		
		// парс
		QString ingredients = parseRequest(request);
        
		// ищем блюдо
		QString result = findDish(ingredients);
		
		// ответ
        clientSocket -> write(result.toUtf8() + "\n");
    }
}

QString DishServer::parseRequest(QString data) {
    return data.toLower().trimmed();
}

QString DishServer::findDish(QString ingredients) {
    if (ingredients.contains("яйцо") && ingredients.contains("молоко"))
		return "Найдено: Омлет";

    if (ingredients.contains("картошка") || ingredients.contains("картофель"))
		return "Найдено: Жареная картошка";

    return "Никаких блюд не найдено :(";
}

void DishServer::slotClientDisconnected(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(clientSocket) {
        clientSocket -> close();
        clientSocket -> deleteLater();
    }
}
