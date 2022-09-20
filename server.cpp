#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    server_tcp = new QTcpServer;
    server_socket = new QTcpSocket;
    misa = new Misa;
    xml = new Xml;
}

Server::~Server()
{
    server_tcp->close();
    server_socket->close();
    delete server_tcp;
    delete server_socket;
    delete misa;
    delete xml;
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

void Server::ServerSocketWrite(QByteArray msg)
{
    QByteArray send;
    quint8 checksum = 0;
    char checksum_p[3];
    if (noack_mode) {
        send.append('$');
    } else {
        send.append('+');
        send.append('$');
    }
    send.append(msg);
    send.append('#');
    foreach (char var, msg) {
        checksum += var;
    }
    checksum &= 0xff;
    sprintf(checksum_p, "%02x", checksum);
    send.append(checksum_p);
    qDebug() << "->S:" << send;
    server_socket->write(send);
}

bool Server::ServerSocketCmd(QByteArray msg)
{
    QByteArray send;

    qDebug() << "S->:" << msg;
    switch (msg[0]) {
    case 'q':
        if (strncmp(msg.constData(), "qXfer:features:read:target.xml:0,", 33) == 0) {
            sscanf(msg.constData(), "qXfer:features:read:target.xml:%x,%x", &target_xml_addr, &target_xml_len);
            ServerTargetWrite("+:read:misa:vlenb;");
            return true;
        } else if (strncmp(msg.constData(), "qXfer:features:read:target.xml:", 31) == 0) {
            sscanf(msg.constData(), "qXfer:features:read:target.xml:%x,%x", &target_xml_addr, &target_xml_len);
            quint32 xml_len;
            xml_len = xml->GetXmlLen();
            if (target_xml_len >= (xml_len - target_xml_addr)) {
                target_xml_len = xml_len - target_xml_addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(xml->GetXml(target_xml_addr).constData(), target_xml_len);
            ServerSocketWrite(send);
            return true;
        }
        break;
    case 'Q':
        if (strncmp(msg.constData(), "QStartNoAckMode", 15) == 0) {
            noack_mode = true;
            return false;
        }
        break;
    default:
        break;
    }
    return false;
}

void Server::ServerTargetWrite(QByteArray msg)
{
    QByteArray send;
    quint8 checksum = 0;
    char checksum_p[3];
    if (noack_mode) {
        send.append('$');
    } else {
        send.append('+');
        send.append('$');
    }
    send.append(msg);
    send.append('#');
    foreach (char var, msg) {
        checksum += var;
    }
    checksum &= 0xFF;
    sprintf(checksum_p, "%02x", checksum);
    send.append(checksum_p);
    qDebug() << "->T:" << send;
    emit ServerToTarget(send);
}

bool Server::ServerTargetCmd(QByteArray msg)
{
    QByteArray send;

    qDebug() << "T->:" << msg;
    switch (msg[2]) {
    case 'r':
        if (strncmp(msg.constData(), "-:read:misa:vlenb:", 12) == 0) {
            sscanf(msg.constData(), "-:read:misa:vlenb:%08x:%016llx;", &target_misa, &target_vlenb);
            misa->MisaInit(target_misa);
            xml->GetInitXml(misa, target_vlenb);
            quint32 xml_len;
            xml_len = xml->GetXmlLen();
            if (target_xml_len >= (xml_len - target_xml_addr)) {
                target_xml_len = xml_len - target_xml_addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(xml->GetXml(target_xml_addr).constData(), target_xml_len);
            ServerSocketWrite(send);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void Server::ServerNewConnect()
{
    server_socket = server_tcp->nextPendingConnection();
    connect(server_socket, SIGNAL(readyRead()), this, SLOT(ServerSocketRead()));
    connect(server_socket, SIGNAL(disconnected()), this, SLOT(ServerSocketDisconnect()));
    qDebug() << "Server socket connect:" << server_port << Qt::endl;

    noack_mode = false;
    QByteArray msg;
    msg.append("+:set:inerface;");
    msg.append(interface.toLatin1());
    msg.append(';');
    ServerTargetWrite(msg);
}

void Server::ServerSocketRead()
{
    quint32 i = 0;
    server_msg.append(server_socket->readAll());
    if ((server_msg.contains('$')) && (server_msg.contains('#'))) {
        foreach (char var, server_msg) {
            i++;
            if ('$' == var) {
                break;
            }
        }
        if (ServerSocketCmd(&server_msg[i])) {
            server_msg.clear();
            return;
        }
        qDebug() << "S->T:" << server_msg;
        emit ServerToTarget(server_msg);
        server_msg.clear();
    }
}

void Server::ServerSocketDisconnect()
{
    server_socket->disconnect();
    server_socket->close();
    qDebug() << "Server socket disconnect:" << server_port << Qt::endl;
}

void Server::ServerWrite(QByteArray msg)
{
    quint32 i = 0;
    foreach (char var, msg) {
        i++;
        if ('$' == var) {
            break;
        }
    }
    if ('-' == msg[i]) {
        if (ServerTargetCmd(&msg[i])) {
            return;
        }
    }
    qDebug() << "T->S:" << msg;
    server_socket->write(msg);
}
