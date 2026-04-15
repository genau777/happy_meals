#ifndef DISHSERVER_H
#define DISHSERVER_H
#include <QObject>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>

class DishServer : public QObject
{
    Q_OBJECT
public:
    explicit DishServer(QObject *parent = nullptr);
    ~DishServer();
public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();
private:
	QTcpServer * mTcpServer;
	QMap<qintptr, QTcpSocket*> m_clients;
};

#endif // DISHSERVER_H
