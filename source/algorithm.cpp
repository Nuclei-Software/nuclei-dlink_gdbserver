#include "../include/algorithm.h"

extern bool debug;

Algorithm::Algorithm(QObject *parent)
    : QObject{parent}
{
    flash.spi_base = 0x10014000;
    flash.xip_base = 0x20000000;
    flash.xip_size = 0x10000000;
    flash.block_size = 0x10000;

    workarea.addr = 0x00;
    workarea.size = 0x00;
    workarea.backup = false;
}

void Algorithm::BackupWorkarea(Target* target)
{
    //backup workarea
    if (workarea.backup) {
        workarea.mem = target->ReadMemory(workarea.addr, workarea.size);
    }
}

void Algorithm::RestoreWorkarea(Target* target)
{
    //restore workarea
    if (workarea.backup) {
        target->WriteMemory(workarea.addr, workarea.mem, workarea.size);
    }
}

void Algorithm::DownloadLoader(Target* target)
{
    QByteArray loader;
    //download flash loader
    QFile loader_f(flash.loader_path);
    if (loader_f.exists()) {
        loader_f.open(QIODevice::ReadOnly);
        loader = loader_f.readAll();
        target->WriteMemory(workarea.addr, loader, loader.size());
        buffer_addr = workarea.addr + loader.size();
    } else {
        qDebug() << flash.loader_path << " not found.";
    }
}

void Algorithm::Execute(Target* target, quint32 cs, quint32 addr, quint32 count, QByteArray buffer)
{
    QByteArray send;
    quint32 params1, params2, params3;
    char temp[1024];

    if (WRITE_CMD == cs) {
        //download write data
        target->WriteMemory(buffer_addr, buffer, count);
    }
    //execute algorithm
    switch (cs) {
    case PROBE_CMD:
        params1 = 0;
        params2 = 0;
        params3 = 0;
        break;
    case ERASE_CMD:
        params1 = addr;
        params2 = addr + count;
        params3 = 0;
        break;
    case WRITE_CMD:
        params1 = buffer_addr;
        params2 = addr;
        params3 = count;
        break;
    default:
        params1 = 0;
        params2 = 0;
        params3 = 0;
        break;
    }
    sprintf(temp, "+:algorithm:%llx,%x,%llx,%x,%x,%x;", workarea.addr, cs, flash.spi_base, params1, params2, params3);
    send.clear();
    send.append(temp);
    target->SendCmd(send);
    target->GetRsp();
    // run loader
    send.clear();
    send.append('c');
    target->SendCmd(send);
    target->GetRsp();
}
