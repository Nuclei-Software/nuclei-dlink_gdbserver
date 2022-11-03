#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QQueue>
#include <QFile>
#include "../include/misa.h"
#include "../include/regxml.h"
#include "../include/memxml.h"
#include "../include/algorithm.h"

class Transmit : public QThread
{
    Q_OBJECT
public:
    Transmit();
    void TransmitInit();
    void TransmitDeinit();

    QString interface;
    Algorithm::flash_t flash;
    Algorithm::workarea_t workarea;

protected:
    void run() override;

private:
    QByteArray TransmitPackage(QByteArray msg);
    QByteArray ReadTargetMemory(quint32 memory_addr, quint32 length);
    void WriteTargetMemory(quint32 memory_addr, QByteArray data, quint32 length);
    void ExecuteAlgorithm(quint32 cs, quint32 addr, quint32 count, QByteArray buffer);
    QByteArray TransmitTargetRsp(QByteArray msg);
    void TransmitTargetCmd(QByteArray msg);
    void TransmitServerCmdDeal(QByteArray msg);
    void TransmitServerCmd(QByteArray msg);
    void TransmitServerRsp(QByteArray msg);

    bool close_flag;
    bool noack_mode;
    quint32 packet_size;
    quint32 loader_addr;
    quint32 buffer_addr;
    Misa* misa;
    RegXml* regxml;
    MemXml* memxml;
    Algorithm* algorithm;
    bool server_reply_flag;
    QByteArray current_command;

signals:
    void TransmitToTarget(QByteArray);
    void TransmitToServer(QByteArray);

public slots:
    void TransmitClose();
};

#endif // TRANSMIT_H
