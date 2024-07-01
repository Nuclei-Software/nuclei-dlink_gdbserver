#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include "type.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    int Init();
    void Deinit();
    void Write(QByteArray msg);
    QByteArray Read();

    quint16 Port;

private:
    Type* type;
    QTcpServer* TcpServer;
    QTcpSocket* TcpSocket;
    QByteArray Message;

signals:
    void readyWrite(QByteArray msg);

private slots:
    void Connect();
    void Disconnect();
    void ReadyRead();
    void ReadyWrite(QByteArray msg);
};

#endif // SERVER_H
