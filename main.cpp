#include <QApplication>
#include <QTextCursor>
#include "transmit.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* to fix qt warning */
    qRegisterMetaType<QTextCursor >("QTextCursor");

    Transmit transmit(argc, argv);
    transmit.start();

    return a.exec();
}
