#include "../include/transmit.h"

QString version = "V1.0.0";

extern QQueue<QByteArray> server_cmd_queue;
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

static void bin_to_hex(QByteArray data_bin, char *hex, uint32_t nbyte)
{
    uint32_t i;
    uint8_t hi;
    uint8_t lo;
    char *bin = data_bin.data();

    for(i = 0; i < nbyte; i++) {
        hi = (*bin >> 4) & 0xf;
        lo = *bin & 0xf;

        if (hi < 10) {
            *hex = '0' + hi;
        } else {
            *hex = 'a' + hi - 10;
        }

        hex++;

        if (lo < 10) {
            *hex = '0' + lo;
        } else {
            *hex = 'a' + lo - 10;
        }

        hex++;
        bin++;
    }
}

static QByteArray hex_to_bin(QByteArray data_hex, uint32_t nbyte)
{
    uint32_t i;
    uint8_t hi, lo;
    char *hex = data_hex.data();
    char bin;
    QByteArray data_bin;
    for(i = 0; i < nbyte; i++) {
        if (hex[i * 2] <= '9') {
            hi = hex[i * 2] - '0';
        } else if (hex[i * 2] <= 'F') {
            hi = hex[i * 2] - 'A' + 10;
        } else {
            hi = hex[i * 2] - 'a' + 10;
        }

        if (hex[i * 2 + 1] <= '9') {
            lo = hex[i * 2 + 1] - '0';
        } else if (hex[i * 2 + 1] <= 'F') {
            lo = hex[i * 2 + 1] - 'A' + 10;
        } else {
            lo = hex[i * 2 + 1] - 'a' + 10;
        }

        bin = (hi << 4) | lo;
        data_bin.append(bin);
    }
    return data_bin;
}

Transmit::Transmit()
{
    misa = new Misa;
    regxml = new RegXml;
}

void Transmit::TransmitInit()
{
    memxml = new MemXml;
    algorithm = new Algorithm;

    memxml->AddFlash(flash);
    algorithm->AddFlash(flash);
    algorithm->AddWorkarea(workarea);

    noack_mode = false;
    server_reply_flag = true;
    packet_size = 0x200;

    server_cmd_queue.clear();
    target_rsp_queue.clear();

    qDebug() << "Nuclei Dlink GDB Server " << version << "Command Line Version";
    fprintf(stderr, "Started by GNU MCU Eclipse\n");
}

void Transmit::TransmitDeinit()
{
    delete memxml;
    delete algorithm;
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

bool Transmit::WaitForTargetRsp()
{
    while (target_rsp_queue.empty())
    {
        if (close_flag) {
            return false;
        }
    }
    return true;
}

QByteArray Transmit::ReadTargetMemory(quint32 memory_addr, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send, read, bin;
    char temp[1024];
    do {
        if (length < data_size) {
            data_size = length;
        }
        sprintf(temp, "m%x,%x", data_addr + memory_addr, data_size);
        send.append(temp);
        data_addr += data_size;
        length -= data_size;
        TransmitTargetCmd(send);
        send.clear();
        if (WaitForTargetRsp()) {
            read = TransmitTargetRsp(target_rsp_queue.dequeue());
            bin.append(hex_to_bin(read, data_size));
        } else {
            return NULL;
        }
    } while (length);
    return bin;
}

void Transmit::WriteTargetMemory(quint32 memory_addr, QByteArray data, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send;
    char temp[1024];
    do {
        if (length < data_size) {
            data_size = length;
        }
        sprintf(temp, "M%x,%x:", data_addr + memory_addr, data_size);
        send.append(temp);
        bin_to_hex(data.mid(data_addr), temp, data_size);
        for (quint32 i = 0; i < (data_size * 2); i++) {
            send.append(temp[i]);
        }
        data_addr += data_size;
        length -= data_size;
        TransmitTargetCmd(send);
        send.clear();
        if (WaitForTargetRsp()) {
            target_rsp_queue.dequeue();
        }
    } while(length);
}

void Transmit::ExecuteAlgorithm(quint32 cs, quint32 addr, quint32 count, QByteArray buffer)
{
    QByteArray send;
    quint32 params1, params2, params3;
    char temp[1024];

    if (WRITE_CMD == cs) {
        //download write data
        WriteTargetMemory(buffer_addr, buffer, buffer.size());
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
    sprintf(temp, "+:algorithm:%x,%x,%llx,%x,%x,%x;", loader_addr, cs, flash.spi_base, params1, params2, params3);
    send.clear();
    send.append(temp);
    TransmitTargetCmd(send);
    if (WaitForTargetRsp()) {
        target_rsp_queue.dequeue();
    }
    send.clear();
    send.append('c');
    TransmitTargetCmd(send);
    if (WaitForTargetRsp()) {
        target_rsp_queue.dequeue();
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
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'q':
        if (strncmp(msg.constData(), "qSupported:", 11) == 0) {
            noack_mode = false;
            //Set protocol and connect target
            send.clear();
            send.append("+:set:protocol:");
            send.append(protocol.toLatin1());
            send.append(';');
            TransmitTargetCmd(send);
            if (WaitForTargetRsp()) {
                recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                if (recv.contains("protocol")) {
                    qDebug() << "set protocol and connect success.";
                } else {
                    qDebug() << "set protocol and connect fail.";
                }
            }
            //Get target MISA CSR register
            send.clear();
            send.append("+:read:misa;");
            TransmitTargetCmd(send);
            if (WaitForTargetRsp()) {
                recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                if (recv.contains("misa")) {
                    quint32 target_misa;
                    sscanf(recv.constData(), "-:read:misa:%08x;", &target_misa);
                    misa->MisaInit(target_misa);
                    memxml->InitMemXml(misa);
                    qDebug() << "read misa:" << QString("%1").arg(target_misa, 4, 16);
                } else {
                    qDebug() << "read misa fail.";
                }
            }
            //Get target VLENB CSR register
            send.clear();
            send.append("+:read:vlenb;");
            TransmitTargetCmd(send);
            if (WaitForTargetRsp()) {
                recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                if (recv.contains("vlenb")) {
                    quint64 target_vlenb;
                    sscanf(recv.constData(), "-:read:vlenb:%016llx;", &target_vlenb);
                    regxml->InitRegXml(misa, target_vlenb);
                    qDebug() << "read vlenb:" << QString("%1").arg(target_vlenb, 4, 16);
                } else {
                    qDebug() << "read vlenb fail.";
                }
            }
            //deal qSupported
            send.clear();
            send.append("PacketSize=405;"
                        "QStartNoAckMode+;"
                        "qXfer:features:read+;"
                        "qXfer:memory-map:read+;"
                        "swbreak+;"
                        "hwbreak+;");
            TransmitServerRsp(send);
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
        } else if (strncmp(msg.constData(), "qRcmd,", 6) == 0) {
            TransmitTargetCmd(msg);
            if (WaitForTargetRsp()) {
                TransmitServerRsp(target_rsp_queue.dequeue());
            }
        } else {
            /* Not support 'q' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    case 'Q':
        if (strncmp(msg.constData(), "QStartNoAckMode", 15) == 0) {
            TransmitTargetCmd(msg);
            if (WaitForTargetRsp()) {
                TransmitServerRsp(target_rsp_queue.dequeue());
                noack_mode = true;
            }
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
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'G':/* `G XX...` */
        /* Write general registers. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'k':/* `k` */
        /* Kill request. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'c':/* `c [addr]` */
        /* Continue at addr, which is the address to resume. */
        /* If addr is omitted, resume at current address. */
        TransmitTargetCmd(msg);
        send.append("OK");
        TransmitServerRsp(send);
        break;
    case 'm':/* `m addr,length` */
        /* Read length addressable memory units starting at address addr. */
        /* Note that addr may not be aligned to any particular boundary. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'M':/* `M addr,length:XX...` */
        /* Write length addressable memory units starting at address addr. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'X':/* `X addr,length:XX...` */
        /* Write data to memory, where the data is transmitted in binary. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'p':/* `p n` */
        /* Read the value of register n; n is in hex. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'P':/* `P n...=r...` */
        /* Write register n... with value r... The register number n is in hexadecimal, */
        /* and r... contains two hex digits for each byte in the register (target byte order). */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 's':/* `s [addr]` */
        /* Single step, resuming at addr. If addr is omitted, resume at same address. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'z':/* `z type,addr,kind` */
        /* remove a type breakpoint or watchpoint starting at address, address of kind. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'Z':/* `Z type,addr,kind` */
        /* Insert a type breakpoint or watchpoint starting at address, address of kind. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'v':
        if (strncmp(msg.constData(), "vMustReplyEmpty", 15) == 0) {
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "vFlashErase:", 12) == 0) {
            //backup workarea
            if (workarea.backup) {
                workarea.mem = ReadTargetMemory(workarea.addr, workarea.size);
            }
            //download flash loader
            QFile loader(flash.loader_path);
            if (loader.exists()) {
                loader.open(QIODevice::ReadOnly);
                QByteArray bin = loader.readAll();
                loader_addr = workarea.addr;
                WriteTargetMemory(loader_addr, bin, bin.size());
                buffer_addr = loader_addr + bin.size();
            } else {
                qDebug() << flash.loader_path << " not found.";
            }
            ExecuteAlgorithm(PROBE_CMD, 0, 0, NULL);
            quint64 target_erase_addr;
            quint64 target_erase_len;
            sscanf(msg.constData(), "vFlashErase:%llx,%llx", &target_erase_addr, &target_erase_len);
            qDebug() << "Erase:" << QString("%1").arg(target_erase_addr, 4, 16) << ":" << QString("%1").arg(target_erase_len, 4, 16);
            ExecuteAlgorithm(ERASE_CMD, target_erase_addr, target_erase_len, NULL);
            send.append("OK");
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "vFlashWrite:", 12) == 0) {
            quint64 target_write_addr;
            quint64 target_write_len;
            QByteArray target_write_data;
            sscanf(msg.constData(), "vFlashWrite:%llx:", &target_write_addr);
            target_write_data = bin_decode(msg.mid(msg.indexOf(':', 15) + 1));
            target_write_len = target_write_data.size();
            qDebug() << "Write:" << QString("%1").arg(target_write_addr, 4, 16) << ":" << QString("%1").arg(target_write_len, 4, 16);
            ExecuteAlgorithm(WRITE_CMD, target_write_addr, target_write_len, target_write_data);
            send.append("OK");
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "vFlashDone", 10) == 0) {
            //restore workarea
            if (workarea.backup) {
                WriteTargetMemory(workarea.addr, workarea.mem, workarea.size);
            }
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
    emit TransmitToServer(TransmitPackage(msg));
    server_reply_flag = true;
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
                server_reply_flag = false;
                msg = server_cmd_queue.dequeue();
                current_command = msg;
                TransmitServerCmd(msg);
            }
        }
        if (!target_rsp_queue.empty()) {
            msg = target_rsp_queue.dequeue();
            emit TransmitToServer(TransmitPackage(msg));
        }
    }
}

void Transmit::TransmitClose()
{
    close_flag = true;
}
