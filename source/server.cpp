#include "../include/server.h"

extern bool debug;

Server::Server(QObject *parent) : QObject(parent)
{
    type = new Type;
    TcpServer = new QTcpServer;
    TcpSocket = new QTcpSocket;
    connect(this, SIGNAL(readyWrite(QByteArray)), this, SLOT(ReadyWrite(QByteArray)));
}

void Server::Init()
{
    if (TcpServer->listen(QHostAddress::Any, Port)) {
        qDebug() << "Server listen to port:" << Port;
    } else {
        qDebug() << "Server fail listen to port:" << Port;
        return;
    }
    connect(TcpServer, SIGNAL(newConnection()), this, SLOT(Connect()));
}

void Server::Deinit()
{
    Disconnect();
    disconnect(TcpServer, 0, 0, 0);
    TcpServer->close();
    qDebug() << "Server Close:" << Port;
    Port = 0;
}

void Server::Connect()
{
    TcpSocket = TcpServer->nextPendingConnection();
    connect(TcpSocket, SIGNAL(readyRead()), this, SLOT(ReadyRead()));
    connect(TcpSocket, SIGNAL(disconnected()), this, SLOT(Disconnect()));
    qDebug() << "Server socket connect:" << Port;
}

void Server::Disconnect()
{
    TcpSocket->disconnect();
    qDebug() << "Server socket disconnect:";
    disconnect(TcpSocket, 0, 0, 0);
    TcpSocket->close();
    Message.clear();
}

void Server::ReadyRead()
{
    Message.append(TcpSocket->readAll());
}

void Server::ReadyWrite(QByteArray msg)
{
    TcpSocket->write(msg);
}

void Server::Write(QByteArray msg)
{
    if (debug) {
        qDebug() << "->S:" << type->pack(msg);
    }
    emit readyWrite(type->pack(msg));
}

QByteArray Server::Read()
{
    QByteArray Temp;
    if (Message.size()) {
        if (Message.contains('$') && Message.contains('#')) {
            while(1) {
                if (Message[0] == '$') {
                    Temp = Message.mid(0, Message.indexOf('#') + 1);
                    Message.remove(0, Message.indexOf('#') + 4);
                    break;
                }
                Message.removeFirst();
            }
        } else if (Message[0] == '\x03') {
            Temp.append(Message[0]);
            Message.removeFirst();
        } else {
            Message.removeFirst();
        }
        if (debug) {
            qDebug() << "S->:" << Temp;
        }
    }
    return type->unpack(Temp);
}
