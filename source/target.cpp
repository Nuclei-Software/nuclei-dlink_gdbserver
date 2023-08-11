#include "../include/target.h"

extern bool debug;
extern quint64 target_packet_max;

Target::Target(QObject *parent) : QObject(parent)
{
    type = new Type;
    serial = new Serial;
    misa = new Misa;
}

void Target::Init()
{
    serial->Init();
}

void Target::Deinit()
{
    serial->Deinit();
}

void Target::SendCmd(QByteArray msg)
{
    if (debug) {
        qDebug() << "->T:" << type->pack(msg);
    }
    serial->Write(type->pack(msg));
}

QByteArray Target::GetRsp()
{
    QByteArray read;
    while(1) {
        read = serial->Read();
        if (read.size()) {
            if (debug) {
                qDebug() << "T->:" << read;
            }
            break;
        }
    }
    return type->unpack(read);
}


void Target::WriteMemory(quint64 addr, QByteArray data, quint32 length)
{
    quint64 current_size = target_packet_max/2;
    quint64 current_addr = 0;
    QByteArray send;
    char temp[1024];
    do {
        if (length < current_size) {
            current_size = length;
        }
        sprintf(temp, "M%llx,%llx:", current_addr + addr, current_size);
        send.clear();
        send.append(temp);
        send.append(type->bin_to_hex(data.mid(current_addr), current_size));
        current_addr += current_size;
        length -= current_size;
        SendCmd(send);
        GetRsp();
    } while(length);
}

QByteArray Target::ReadMemory(quint64 addr, quint32 length)
{
    quint64 current_size = target_packet_max/2;
    quint64 current_addr = 0;
    QByteArray send, read, bin;
    char temp[1024];
    do {
        if (length < current_size) {
            current_size = length;
        }
        sprintf(temp, "m%llx,%llx", current_addr + addr, current_size);
        send.clear();
        send.append(temp);
        SendCmd(send);
        read = GetRsp();
        bin.append(type->hex_to_bin(read, current_size));
        current_addr += current_size;
        length -= current_size;
    } while (length);
    return bin;
}

void Target::WriteRegister(quint32 number, quint64 value)
{
    char temp[1024];
    QByteArray send;
    sprintf(temp, "P%x=%llx", number, value);
    send.append(temp);
    SendCmd(send);
    GetRsp();
}

quint64 Target::ReadRegister(quint32 number)
{
    char temp[1024];
    QByteArray send, read;
    quint64 value = 0;

    sprintf(temp, "p%x", number);
    send.append(temp);
    SendCmd(send);
    read = GetRsp();
    if (read.size() <= 8) {
        value = type->hex_to_uint32_le(read);
    } else {
        value = type->hex_to_uint64_le(read);
    }
    return value;
}
