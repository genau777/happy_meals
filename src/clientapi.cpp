#include "clientapi.h"
#include "db_singleton.h"
#include <QDebug>

ClientApi* ClientApi::p_instance = nullptr;
ClientApiDestroyer ClientApi::destroyer;

ClientApiDestroyer::~ClientApiDestroyer()
{
    delete p_instance;
}

void ClientApiDestroyer::initialize(ClientApi* p)
{
    p_instance = p;
}

ClientApi::ClientApi(QObject* parent)
    : QObject(parent),
      sock(new QTcpSocket(this))
{
}

ClientApi::~ClientApi()
{
}

ClientApi* ClientApi::getInstance()
{
    if (!p_instance) {
        p_instance = new ClientApi();
        destroyer.initialize(p_instance);
    }
    return p_instance;
}

bool ClientApi::connectToServer(const QString& host, quint16 port)
{
    Q_UNUSED(host);
    Q_UNUSED(port);
    
    // Локальное подключение - используем напрямую DB_Singleton
    return true;
}

QString ClientApi::registerUser(const QString& login,
                                const QString& password,
                                const QString& email)
{
    if (login.trimmed().isEmpty() || email.trimmed().isEmpty() || password.trimmed().isEmpty())
        return "ERROR:empty_fields";
    
    // Используем реальную базу данных
    bool success = DB_Singleton::getInstance()->reg(login, password, email, 0);
    
    if (success)
        return "OK:registered";
    else
        return "ERROR:user_exists";
}

QStringList ClientApi::findDishes(const QStringList& excludedIngredients,
                                  const QString& cuisine,
                                  const QString& type,
                                  int maxTime,
                                  int maxComplexity)
{
    Q_UNUSED(type);
    Q_UNUSED(maxComplexity);
    
    QStringList result;
    
    // Конвертируем кухню в формат базы данных
    QStringList cuisines;
    if (!cuisine.isEmpty() && cuisine != "Любая кухня") {
        QString cuisineUpper = cuisine.toUpper();
        if (cuisineUpper.contains("РУССКАЯ")) cuisines.append("Русская");
        else if (cuisineUpper.contains("ИТАЛЬЯНСКАЯ")) cuisines.append("Итальянская");
        else if (cuisineUpper.contains("ЯПОНСКАЯ")) cuisines.append("Японская");
        else if (cuisineUpper.contains("КИТАЙСКАЯ")) cuisines.append("Китайская");
        else if (cuisineUpper.contains("МЕКСИКАНСКАЯ")) cuisines.append("Мексиканская");
    }
    
    // Конвертируем исключенные ингредиенты в нижний регистр
    QStringList excludedLower;
    for (const QString& ing : excludedIngredients) {
        excludedLower.append(ing.toLower());
    }
    
    // Получаем блюда из базы данных
    QList<Dish> dishes = DB_Singleton::getInstance()->filterDishes(
        excludedLower,
        cuisines,
        maxTime
    );
    
    // Возвращаем только названия
    for (const Dish& dish : dishes) {
        result.append(dish.name);
    }
    
    return result;
}

QString ClientApi::getDishDetails(const QString& dishName)
{
    return DB_Singleton::getInstance()->getDishDetails(dishName);
}
