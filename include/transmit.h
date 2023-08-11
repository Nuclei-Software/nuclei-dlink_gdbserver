#ifndef TRANSMIT_H
#define TRANSMIT_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QQueue>
#include "type.h"
#include "server.h"
#include "target.h"
#include "memxml.h"
#include "regxml.h"
#include "cpuinfo.h"
#include "etrace.h"
#include "algorithm.h"

class Transmit : public QThread
{
    Q_OBJECT
public:
    Transmit();
    void Reset();
    void Init();
    void Deinit();

    QString protocol;
    Server* server;
    Target* target;
    Algorithm* algorithm;

protected:
    void run() override;

private:
    void ServerCmdDeal(QByteArray msg);

    bool close_flag;
    bool target_run_flag;
    Type* type;
    MemXml* memxml;
    RegXml* regxml;
    Cpuinfo* cpuinfo;
    Etrace* etrace;

public slots:
    void Close();
};

#endif // TRANSMIT_H
