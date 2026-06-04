#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

class HappyMealsServer : public QTcpServer {
    Q_OBJECT

public:
    bool start() {
        if (!listen(QHostAddress::Any, 33333)) {
            qDebug() << "Server failed to start";
            return false;
        }

        qDebug() << "Server started on port 33333";
        return true;
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);

        connect(socket, &QTcpSocket::readyRead, [=]() {
            QString req = socket->readAll().trimmed();
            qDebug() << "Request:" << req;

            QString response = process(req);

            socket->write(response.toUtf8());
            socket->flush();
        });

        connect(socket, &QTcpSocket::disconnected, [=]() {
            socket->deleteLater();
        });
    }

private:

    QString process(const QString &req) {
        if (!req.startsWith("get_dish:"))
            return "ERROR:Unknown command";

        QString data = req.mid(QString("get_dish:").length());

        QStringList parts = data.split(";");

        QString ingredients = parts.value(0);
        QString cuisines    = parts.value(1);
        QString maxTime     = parts.value(2);

        return getDishes(cuisines, maxTime);
    }

    QString getDishes(const QString &cuisines, const QString &maxTime) {

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("data/dishes.sqlite");

        if (!db.open())
            return "ERROR:DB open failed";

        QString sql =
            "SELECT name, time, cuisine FROM dishes WHERE 1=1";

        if (!cuisines.isEmpty()) {
            QStringList list = cuisines.split(",");
            QString in = "'" + list.join("','") + "'";
            sql += " AND cuisine IN (" + in + ")";
        }

        if (!maxTime.isEmpty()) {
            sql += " AND time <= " + maxTime;
        }

        QSqlQuery query;
        if (!query.exec(sql)) {
            return "ERROR:SQL failed";
        }

        QStringList result;

        while (query.next()) {
            QString name = query.value(0).toString();
            QString time = query.value(1).toString();

            result << name + " (" + time + " мин)";
        }

        if (result.isEmpty())
            return "ERROR:Нет блюд";

        return "OK:" + result.join(", ");
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    HappyMealsServer server;

    if (!server.start())
        return 1;

    return a.exec();
}

#include "server.moc"
