#include "dishserver.h"
#include "storage.h"
#include "filters.h"

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
	QTcpSocket *clientSocket = mTcpServer -> nextPendingConnection();

	m_clients.append(clientSocket);
	qDebug() << "New client connect! Total clients:" << m_clients.size();

	clientSocket -> write("Добро пожаловать! Это приложение для подбора блюд HappyMeals.\r\n");

    connect(clientSocket, &QTcpSocket::readyRead, this, &DishServer::slotServerRead);
    connect(clietnSocket, &QTcpSocket::disconnected, this, &DishServer::slotClientDisconnected);
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

QString DishServer::findDish(QString inputStr) {
	Storage storage;
	FilterManager filterManager;
	Preferences prefs;

	// если клиент прислал слово "помидор", добавим его в нелюбимые, чтобы проверить функциональность
	if (!inputStr.isEmpty()) {
		Ingredient disliked {QUuid::createUuid(), inputStr, IngredientCategory::OTHER };
		prefs.dislikedIngredients.append(disliked);
	}

	QList<Dish> allDishes = storage.getAllDishes();

	QList<Dish> recommended = filterManager.applyAll(allDishes, prefs);

	if (recommended.isEmpty())
		return "Нет блюд удовлетворяющих вашим предпочтениям.\n";
	
	QString response = "Мы рекомендуем:\n";
	for (const Dish& dish : recommended) 
		response += "- " + dish.name + " (" + QString::number(dish.prepTime) + " минут)\n";

	return response;
}

void DishServer::slotClientDisconnected(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());

    if(clientSocket) {
		m_clients.removeOne(clientSocket);
		qDebug() << "Client disconnected. Total clients:" << m_clients.size();
		
		clientSocket -> close();
        clientSocket -> deleteLater();
    }
}
