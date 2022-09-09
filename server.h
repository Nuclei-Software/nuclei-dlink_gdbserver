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
    ~Server();
    void ServerInit();

    quint16 server_port;
    QQueue<QByteArray> server_queue;

private:
    QTcpServer* server_tcp;
    QTcpSocket* server_socket;

signals:
    void ServerToTarget(QByteArray);

public slots:
    void ServerNewConnect();
    void ServerSocketRead();
    void ServerSocketDisconnect();
    void ServerWrite(QByteArray msg);
};

#endif // SERVER_H
