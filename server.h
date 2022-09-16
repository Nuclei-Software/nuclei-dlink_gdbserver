#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QQueue>
#include "misa.h"
#include "xml.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
    void ServerInit();

    quint16 server_port;
    QQueue<QByteArray> server_queue;
    QString interface;

private:
    void ServerSocketWrite(QByteArray msg);
    bool ServerSocketCmd(QByteArray msg);

    void ServerTargetWrite(QByteArray msg);
    bool ServerTargetCmd(QByteArray msg);

    QTcpServer* server_tcp;
    QTcpSocket* server_socket;
    QByteArray server_msg;

    Misa* misa;
    Xml* xml;
    bool noack_mode;
    quint32 target_xml_addr;
    quint32 target_xml_len;
    quint32 target_misa;
    quint64 target_vlenb;

signals:
    void ServerToTarget(QByteArray);

public slots:
    void ServerNewConnect();
    void ServerSocketRead();
    void ServerSocketDisconnect();
    void ServerWrite(QByteArray msg);
};

#endif // SERVER_H
