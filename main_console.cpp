#include <QCoreApplication>
#include "include/application_console.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Application app;
    app.ApplicationInit(argc, argv);

    return a.exec();
}
