#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <QObject>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "server.h"
#include "target.h"
#include "mainwindow.h"

class Transmit : public QThread
{
    Q_OBJECT
public:
    Transmit(int argc, char *argv[]);
    ~Transmit();

protected:
    void run() override;

private:
    MainWindow* window;
    Server* gdb_server;
    Target* rv_target;

    QString port;
    QString serial;
    QString baud;
    QString interface;

    bool close_flag = false;

public slots:
    void user_connect(QString p, QString s, QString b, QString i);
    void user_close();
};

#endif // TRANSMIT_H
