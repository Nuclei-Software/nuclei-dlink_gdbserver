#include "../include/server.h"

QQueue<QByteArray> server_cmd_queue;

Server::Server(QObject *parent) : QObject(parent)
{
    server_tcp = new QTcpServer;
    server_socket = new QTcpSocket;
}

void Server::ServerInit()
{
    if (server_tcp->listen(QHostAddress::Any, server_port)) {
        qDebug() << "Server listen to port:" << server_port;
    } else {
        qDebug() << "Server fail listen to port:" << server_port;
        return;
    }
    connect(server_tcp, SIGNAL(newConnection()), this, SLOT(ServerNewConnect()));
}

void Server::ServerDeinit()
{
    disconnect(server_socket, 0, 0, 0);
    server_socket->close();
    disconnect(server_tcp, 0, 0, 0);
    server_tcp->close();
    qDebug() << "Close:" << server_port;
}

void Server::ServerNewConnect()
{
    server_socket = server_tcp->nextPendingConnection();
    connect(server_socket, SIGNAL(readyRead()), this, SLOT(ServerSocketRead()));
    connect(server_socket, SIGNAL(disconnected()), this, SLOT(ServerSocketDisconnect()));
    qDebug() << "Server socket connect:" << server_port;
}

void Server::ServerSocketRead()
{
    QByteArray msg = server_socket->readAll();
    //qDebug() << "S->:" << msg;
    if ((msg.contains('$')) && (msg.contains('#'))) {
        server_cmd_queue.enqueue(msg);
    } else if (msg[0] == '\x03') {
        server_cmd_queue.enqueue(msg);
    } else {
        qDebug() << "S->:Other Command:" << msg;
    }
}

void Server::ServerSocketDisconnect()
{
    server_socket->disconnect();
    qDebug() << "Server socket disconnect:" << server_port;
}

void Server::ServerWrite(QByteArray msg)
{
    //qDebug() << "->S:" << msg;
    server_socket->write(msg);
}
