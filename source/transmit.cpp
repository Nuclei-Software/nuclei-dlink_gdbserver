#include "../include/transmit.h"

QString version = "V1.0.0";

extern QQueue<QByteArray> server_cmd_queue;
QQueue<QByteArray> server_rsp_queue;
QQueue<QByteArray> target_cmd_queue;
extern QQueue<QByteArray> target_rsp_queue;

/*==== Loader ====*/
#define ERASE_CMD           (1)
#define WRITE_CMD           (2)
#define READ_CMD            (3)
#define PROBE_CMD           (4)

static QByteArray bin_decode(QByteArray xbin)
{
    QByteArray bin;
    quint32 i;
    bool escape_found = false;
    for(i = 0; i < xbin.size(); i++) {
        if (xbin[i] == 0x7d) {
            escape_found = true;
        } else {
            if (escape_found) {
                bin.append(xbin[i] ^ 0x20);
                escape_found = false;
            } else {
                bin.append(xbin[i]);
            }
        }
    }
    return bin;
}

Transmit::Transmit()
{
    misa = new Misa;
    regxml = new RegXml;
    memxml = new MemXml;
    algorithm = new Algorithm;
}

void Transmit::TransmitInit()
{
    memxml->AddFlash(flash);
    algorithm->AddFlash(flash);
    algorithm->AddWorkarea(workarea);

    noack_mode = false;
    server_reply_flag = true;
    packet_size = 0x400;

    qDebug() << "Nuclei Dlink GDB Server " << version << "Command Line Version";
}

QByteArray Transmit::TransmitPackage(QByteArray msg)
{
    QByteArray send;
    quint8 checksum = 0;
    char checksum_c[3];
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
    sprintf(checksum_c, "%02x", checksum);
    send.append(checksum_c);
    return send;
}

QByteArray Transmit::ReadTargetMemory(quint32 memory_addr, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send, read;
    do {
        send.append("m");
        send.append(data_addr + memory_addr);
        send.append(',');
        if (length < data_size) {
            data_size = length;
        }
        send.append(data_size);
        data_addr += data_size;
        length -= data_size;
        TransmitTargetCmd(send);
        while (target_rsp_queue.empty());
        read.append(TransmitTargetRsp(target_rsp_queue.dequeue()));
    } while (length);
    return read;
}

void Transmit::WriteTargetMemory(quint32 memory_addr, QByteArray data, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send;
    do {
        send.append("M");
        send.append(memory_addr + data_addr);
        send.append(',');
        if (length < data_size) {
            data_size = length;
        }
        send.append(data_size);
        send.append(':');
        send.append(data.mid(data_addr, data_size));
        data_addr += data_size;
        length -= data_size;
        TransmitTargetCmd(send);
        while (target_rsp_queue.empty());
        target_rsp_queue.dequeue();
    } while(length);
}

void Transmit::ExecuteAlgorithm(quint32 cs, quint32 addr, quint32 count, QByteArray buffer)
{
    QByteArray send;
    quint32 loader_addr, buffer_addr;
    quint32 params1, params2, params3;
    //backup workarea
    if (workarea.backup) {
        workarea.mem.append(ReadTargetMemory(workarea.addr, workarea.size));
    }
    //download flash loader
    QFile loader(flash.loader_path);
    if (loader.exists()) {
        loader.open(QIODevice::ReadOnly);
        QByteArray bin = loader.readAll();
        loader_addr = workarea.addr;
        WriteTargetMemory(loader_addr, bin, bin.size());
        if (WRITE_CMD == cs) {
            //download write data
            buffer_addr = loader_addr + bin.size();
            WriteTargetMemory(buffer_addr, buffer, buffer.size());
        }
    } else {
        loader_addr = 0;
        buffer_addr = 0;
        qDebug() << flash.loader_path << " not found.";
    }
    //execute algorithm
    send.clear();
    send.append("+:algorithm:");
    send.append(loader_addr);
    send.append(',');
    send.append(cs);
    send.append(',');
    send.append(flash.spi_base);
    send.append(',');
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
    send.append(params1);
    send.append(',');
    send.append(params2);
    send.append(',');
    send.append(params3);
    send.append(';');
    TransmitTargetCmd(send);
    while (target_rsp_queue.empty());
    target_rsp_queue.dequeue();
    //restore workarea
    if (workarea.backup) {
        WriteTargetMemory(workarea.addr, workarea.mem, workarea.size);
    }
}

QByteArray Transmit::TransmitTargetRsp(QByteArray msg)
{
    return msg.mid(msg.indexOf('$') + 1, msg.indexOf('#') - msg.indexOf('$') - 1);
}

void Transmit::TransmitTargetCmd(QByteArray msg)
{
    emit TransmitToTarget(TransmitPackage(msg));
}

void Transmit::TransmitServerCmdDeal(QByteArray msg)
{
    QByteArray send, recv;
    switch (msg[0]) {
    case '\x03':/* Ctrl+C command */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'q':
        if (strncmp(msg.constData(), "qSupported:", 11) == 0) {
            send.append("PacketSize=405;"
                        "QStartNoAckMode+;"
                        "qXfer:features:read+;"
                        "qXfer:memory-map:read+;"
                        "swbreak+;"
                        "hwbreak+;");
            TransmitServerRsp(send);
            //Set interface and connect target
            send.clear();
            send.append("+:set:interface:");
            send.append(interface.toLatin1());
            send.append(';');
            TransmitTargetCmd(send);
            while (target_rsp_queue.empty());
            recv = TransmitTargetRsp(target_rsp_queue.dequeue());
            if (recv.contains("interface")) {
                qDebug() << "set interface and connect success.";
            } else {
                qDebug() << "set interface and connect fail.";
            }

            //Get target MISA CSR register
            send.clear();
            send.append("+:read:misa;");
            TransmitTargetCmd(send);
            while (target_rsp_queue.empty());
            recv = TransmitTargetRsp(target_rsp_queue.dequeue());
            if (recv.contains("misa")) {
                quint32 target_misa;
                sscanf(recv.constData(), "-:read:misa:%08x;", &target_misa);
                misa->MisaInit(target_misa);
                memxml->InitMemXml(misa);
                qDebug() << "read misa:" << target_misa;
            } else {
                qDebug() << "read misa fail.";
            }

            //Get target VLENB CSR register
            send.clear();
            send.append("+:read:vlenb;");
            TransmitTargetCmd(send);
            while (target_rsp_queue.empty());
            recv = TransmitTargetRsp(target_rsp_queue.dequeue());
            if (recv.contains("vlenb")) {
                quint64 target_vlenb;
                sscanf(recv.constData(), "-:read:vlenb:%016llx;", &target_vlenb);
                regxml->InitRegXml(misa, target_vlenb);
                qDebug() << "read vlenb:" << target_vlenb;
            } else {
                qDebug() << "read vlenb fail.";
            }
        } else if (strncmp(msg.constData(), "qXfer:memory-map:read::", 23) == 0) {
            quint32 target_memxml_addr;
            quint32 target_memxml_len;
            quint32 xml_len;
            sscanf(msg.constData(), "qXfer:memory-map:read::%x,%x", &target_memxml_addr, &target_memxml_len);
            xml_len = memxml->GetMemXmlLen();
            if (target_memxml_len >= (xml_len - target_memxml_addr)) {
                target_memxml_len = xml_len - target_memxml_addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(memxml->GetMemXml(target_memxml_addr).constData(), target_memxml_len);
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "qXfer:features:read:target.xml:", 31) == 0) {
            quint32 target_regxml_addr;
            quint32 target_regxml_len;
            quint32 xml_len;
            sscanf(msg.constData(), "qXfer:features:read:target.xml:%x,%x", &target_regxml_addr, &target_regxml_len);
            xml_len = regxml->GetRegXmlLen();
            if (target_regxml_len >= (xml_len - target_regxml_addr)) {
                target_regxml_len = xml_len - target_regxml_addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(regxml->GetRegXml(target_regxml_addr).constData(), target_regxml_len);
            TransmitServerRsp(send);
        } else {
            /* Not support 'q' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    case 'Q':
        if (strncmp(msg.constData(), "QStartNoAckMode", 15) == 0) {
            TransmitTargetCmd(msg);
            while (target_rsp_queue.empty());
            TransmitServerRsp(target_rsp_queue.dequeue());
            noack_mode = true;
        } else {
            /* Not support 'Q' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    case 'H':/* `H op thread-id` */
        /* Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
        TransmitServerRsp(send);
        break;
    case '?':/* `?` */
        /* Indicate the reason the target halted. The reply is the same as for step and continue. */
        send.append("S02");
        TransmitServerRsp(send);
        break;
    case 'g':/* `g` */
        /* Read general registers. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'G':/* `G XX...` */
        /* Write general registers. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'k':/* `k` */
        /* Kill request. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'c':/* `c [addr]` */
        /* Continue at addr, which is the address to resume. */
        /* If addr is omitted, resume at current address. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'm':/* `m addr,length` */
        /* Read length addressable memory units starting at address addr. */
        /* Note that addr may not be aligned to any particular boundary. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'M':/* `M addr,length:XX...` */
        /* Write length addressable memory units starting at address addr. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'X':/* `X addr,length:XX...` */
        /* Write data to memory, where the data is transmitted in binary. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'p':/* `p n` */
        /* Read the value of register n; n is in hex. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'P':/* `P n...=r...` */
        /* Write register n... with value r... The register number n is in hexadecimal, */
        /* and r... contains two hex digits for each byte in the register (target byte order). */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 's':/* `s [addr]` */
        /* Single step, resuming at addr. If addr is omitted, resume at same address. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'z':/* `z type,addr,kind` */
        /* remove a type breakpoint or watchpoint starting at address, address of kind. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'Z':/* `Z type,addr,kind` */
        /* Insert a type breakpoint or watchpoint starting at address, address of kind. */
        TransmitTargetCmd(msg);
        while (target_rsp_queue.empty());
        TransmitServerRsp(target_rsp_queue.dequeue());
        break;
    case 'v':
        if (strncmp(msg.constData(), "vMustReplyEmpty", 15) == 0) {
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "vFlashErase:", 12) == 0) {
            quint64 target_erase_addr;
            quint64 target_erase_len;
            sscanf(msg.constData(), "vFlashErase:%llx,%llx", &target_erase_addr, &target_erase_len);
            qDebug() << "Erase:" << target_erase_addr << ":" << target_erase_len;
            ExecuteAlgorithm(ERASE_CMD, target_erase_addr, target_erase_len, NULL);
        } else if (strncmp(msg.constData(), "vFlashWrite:", 12) == 0) {
            quint64 target_write_addr;
            quint64 target_write_len;
            QByteArray target_write_data;
            sscanf(msg.constData(), "vFlashWrite:%llx:", &target_write_addr);
            target_write_data = bin_decode(msg.mid(msg.lastIndexOf(':') + 1));
            target_write_len = target_write_data.size();
            qDebug() << "Write:" << target_write_addr << ":" << target_write_len;
            qDebug() << "Write data:" << target_write_data;
            ExecuteAlgorithm(WRITE_CMD, target_write_addr, target_write_len, target_write_data);
        } else if (strncmp(msg.constData(), "vFlashDone", 10) == 0) {
            send.append("OK");
            TransmitServerRsp(send);
        } else {
            /* Not support 'v' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    default:/* Not support command, reply empty. */
        TransmitServerRsp(send);
        break;
    }
}

void Transmit::TransmitServerCmd(QByteArray msg)
{
    if (msg[0] == '\x03') {
        TransmitServerCmdDeal(msg.left(1));
    } else {
        TransmitServerCmdDeal(msg.mid(msg.indexOf('$') + 1,
                                  msg.indexOf('#') - msg.indexOf('$') - 1));
    }
}

void Transmit::TransmitServerRsp(QByteArray msg)
{
    server_rsp_queue.enqueue(TransmitPackage(msg));
}

void Transmit::run()
{
    QByteArray msg;
    close_flag = false;
    while (1) {
        if (close_flag) {
            break;
        }
        if (server_reply_flag) {
            if (!server_cmd_queue.empty()) {
                msg = server_cmd_queue.dequeue();
                current_command = msg;
                TransmitServerCmd(msg);
                server_reply_flag = false;
            }
        }
        if (!server_rsp_queue.empty()) {
            msg = server_rsp_queue.dequeue();
            emit TransmitToServer(msg);
            server_reply_flag = true;
        }
    }
}

void Transmit::TransmitClose()
{
    close_flag = true;
}
