#include <QApplication>
#include "include/application.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Application app;
    app.Init(argc, argv);

    return a.exec();
}
