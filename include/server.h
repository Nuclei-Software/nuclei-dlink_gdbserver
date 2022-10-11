#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QQueue>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void ServerInit();

    quint16 server_port;

private:
    QTcpServer* server_tcp;
    QTcpSocket* server_socket;

private slots:
    void ServerNewConnect();
    void ServerSocketRead();
    void ServerSocketDisconnect();

public slots:
    void ServerWrite(QByteArray msg);
};

#endif // SERVER_H
