#include <QCoreApplication>
#include "dishserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DishServer myserv;
    return a.exec();
}
