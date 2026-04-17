#ifndef CLIENTAPI_H
#define CLIENTAPI_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpSocket>

class ClientApi;

class ClientApiDestroyer
{
private:
    ClientApi* p_instance = nullptr;

public:
    ~ClientApiDestroyer();
    void initialize(ClientApi* p);
};

class ClientApi : public QObject
{
    Q_OBJECT

private:
    static ClientApi* p_instance;
    static ClientApiDestroyer destroyer;

    QTcpSocket* sock;

protected:
    explicit ClientApi(QObject* parent = nullptr);
    ClientApi(const ClientApi&) = delete;
    ClientApi& operator=(const ClientApi&) = delete;
    ~ClientApi() override;

    friend class ClientApiDestroyer;

public:
    static ClientApi* getInstance();

    bool connectToServer(const QString& host = "127.0.0.1", quint16 port = 33333);

    QString registerUser(const QString& login,
                         const QString& password,
                         const QString& email);

    QStringList findDishes(const QStringList& excludedIngredients,
                           const QString& cuisine,
                           const QString& type,
                           int maxTime,
                           int maxComplexity);
    
    QString getDishDetails(const QString& dishName);
};

#endif // CLIENTAPI_H
