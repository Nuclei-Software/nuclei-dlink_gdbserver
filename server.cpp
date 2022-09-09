#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    server_tcp = new QTcpServer;
    server_socket = new QTcpSocket;
}

Server::~Server()
{
    server_tcp->close();
    server_socket->close();
    delete server_tcp;
    delete server_socket;
}

void Server::ServerInit()
{
    if (server_tcp->listen(QHostAddress::Any, server_port)) {
        qDebug() << "Server listen to port:" << server_port << Qt::endl;
    } else {
        qDebug() << "Server fail listen to port:" << server_port << Qt::endl;
        return;
    }
    connect(server_tcp, SIGNAL(newConnection()), this, SLOT(ServerNewConnect()));
}

void Server::ServerNewConnect()
{
    server_socket = server_tcp->nextPendingConnection();
    connect(server_socket, SIGNAL(readyRead()), this, SLOT(ServerSocketRead()));
    connect(server_socket, SIGNAL(disconnected()), this, SLOT(ServerSocketDisconnect()));
    qDebug() << "Server socket connect:" << server_port << Qt::endl;
}

void Server::ServerSocketRead()
{
    emit ServerToTarget(server_socket->readAll());
}

void Server::ServerSocketDisconnect()
{
    server_socket->disconnect();
    server_socket->close();
    qDebug() << "Server socket disconnect:" << server_port << Qt::endl;
}

void Server::ServerWrite(QByteArray msg)
{
    qDebug() << "T->S:" << msg;
    server_socket->write(msg);
}
