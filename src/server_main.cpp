#include <QCoreApplication>
#include "dishserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DishServer server;
    return app.exec();
}
