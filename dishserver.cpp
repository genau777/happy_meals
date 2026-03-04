#include "dishserver.h"
#include <QDebug>
#include <QCoreApplication>
#include<QString>

DishServer::~DishServer()
{

    mTcpServer->close();
    //server_status=0;
}

DishServer::DishServer(QObject *parent) : QObject(parent){
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &DishServer::slotNewConnection);

    if(!mTcpServer -> listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        //server_status=1;
        qDebug() << "server is started";
    }
}

void DishServer::slotNewConnection() {
 //   if(server_status==1){
        mTcpSocket = mTcpServer->nextPendingConnection();
        mTcpSocket -> write("Здравствуйте. Это приложение для выбора блюд по ингредиентам.\r\n");
        connect(mTcpSocket, &QTcpSocket::readyRead, this, &DishServer::slotServerRead);
        connect(mTcpSocket, &QTcpSocket::disconnected, this, &DishServer::slotClientDisconnected);
   // }
}

void DishServer::slotServerRead(){
	QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
	if (!clientSocket) return;

	while (clientSocket -> bytesAvailable() > 0) {
		QByteArray data = clientSocket -> readAll();
		QString request = QString::fromUtf8(data).trimmed();

		qDebug() << "Выбранные пользователем ингредиенты:" << request;

		// парсим
		QString ingredients = parseRequest(request);

		// ищем блюдо 
		QString result = findDish(ingredients);

		// отправляем ответ
		clientSocket -> write(result.toUtf8() + '\n');
	}
}

QString DishServer::parseRequest(QString data) {
	return data.toLower().trimmed();
}

QString DishServer::findDish(QString ingredients) {
	if (ingredients.contains("eggs") && ingredients.contains("milk"))
		return "Found: Omelette (Омлет)";

	else if (ingredients.contains("potato"))
		return "Found: Fried Potatoes (Жареная картошка)";

	else
		return "No dishes found for your ingredients :(";
}

void DishServer::slotClientDisconnected(){
    mTcpSocket->close();
}
