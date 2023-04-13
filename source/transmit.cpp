#include "../include/transmit.h"

QString version = "V1.0.0";

extern QQueue<QByteArray> server_cmd_queue;
extern QQueue<QByteArray> target_rsp_queue;

bool debug = false;

/*==== Loader ====*/
#define ERASE_CMD           (1)
#define WRITE_CMD           (2)
#define READ_CMD            (3)
#define PROBE_CMD           (4)

static QByteArray bin_encode(QByteArray bin, quint32 bin_len)
{
    QByteArray xbin;
    quint32 i;

    for(i = 0; i < bin_len; i++) {
        if ((bin[i] == '#') || (bin[i] == '$') || (bin[i] == '}') || (bin[i] == '*')) {
            xbin.append(0x7d);
            xbin.append(bin[i] ^ 0x20);
        } else {
            xbin.append(bin[i]);
        }
    }
    return xbin;
}

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

static QByteArray bin_to_hex(QByteArray data_bin, uint32_t nbyte)
{
    uint32_t i;
    uint8_t hi;
    uint8_t lo;
    QByteArray hex;
    char *bin = data_bin.data();

    for(i = 0; i < nbyte; i++) {
        hi = (*bin >> 4) & 0xf;
        lo = *bin & 0xf;
        if (hi < 10) {
            hex.append('0' + hi);
        } else {
            hex.append('a' + hi - 10);
        }
        if (lo < 10) {
            hex.append('0' + lo);
        } else {
            hex.append('a' + lo - 10);
        }
        bin++;
    }
    return hex;
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
    workarea.addr = 0;
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
    workarea.addr = 0;
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
    bool is_x_command = 0;
    char temp[1024];
    do {
        if (length < data_size) {
            data_size = length;
        }
        if (is_x_command) {
            sprintf(temp, "x%x,%x", data_addr + memory_addr, data_size);
        } else {
            sprintf(temp, "m%x,%x", data_addr + memory_addr, data_size);
        }
        send.append(temp);
        TransmitTargetCmd(send);
        send.clear();
        if (WaitForTargetRsp()) {
            if (is_x_command) {
                read = bin_decode(TransmitTargetRsp(target_rsp_queue.dequeue()));
                bin.append(read);
            } else {
                read = TransmitTargetRsp(target_rsp_queue.dequeue());
                bin.append(hex_to_bin(read, data_size));
            }
        } else {
            return NULL;
        }
        data_addr += data_size;
        length -= data_size;
    } while (length);
    return bin;
}

void Transmit::WriteTargetMemory(quint32 memory_addr, QByteArray data, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send;
    bool is_x_command = 0;
    char temp[1024];
    do {
        if (length < data_size) {
            data_size = length;
        }
        if (is_x_command) {
            sprintf(temp, "X%x,%x:", data_addr + memory_addr, data_size);
            send.append(temp);
            send.append(bin_encode(data.mid(data_addr), data_size));
        } else {
            sprintf(temp, "M%x,%x:", data_addr + memory_addr, data_size);
            send.append(temp);
            send.append(bin_to_hex(data.mid(data_addr), data_size));
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
        WriteTargetMemory(buffer_addr, buffer, count);
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
    QByteArray send, recv, cache;
    quint64 addr, len, temp;
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
                    sscanf(recv.constData(), "-:read:misa:%08llx;", &temp);
                    misa->MisaInit(temp);
                    memxml->InitMemXml(misa);
                    qDebug() << "read misa:" << QString("%1").arg(temp, 4, 16);
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
                    sscanf(recv.constData(), "-:read:vlenb:%016llx;", &temp);
                    regxml->InitRegXml(misa, temp);
                    qDebug() << "read vlenb:" << QString("%1").arg(temp, 4, 16);
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
            sscanf(msg.constData(), "qXfer:memory-map:read::%llx,%llx", &addr, &len);
            temp = memxml->GetMemXmlLen();
            if (len >= (temp - addr)) {
                len = temp - addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(memxml->GetMemXml(addr).constData(), len);
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "qXfer:features:read:target.xml:", 31) == 0) {
            sscanf(msg.constData(), "qXfer:features:read:target.xml:%llx,%llx", &addr, &len);
            temp = regxml->GetRegXmlLen();
            if (len >= (temp - addr)) {
                len = temp - addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(regxml->GetRegXml(addr).constData(), len);
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
            if (workarea.addr) {
                //backup workarea
                if (workarea.backup) {
                    workarea.mem = ReadTargetMemory(workarea.addr, workarea.size);
                }
                //download flash loader
                QFile loader(flash.loader_path);
                if (loader.exists()) {
                    loader.open(QIODevice::ReadOnly);
                    cache = loader.readAll();
                    loader_addr = workarea.addr;
                    WriteTargetMemory(loader_addr, cache, cache.size());
                    buffer_addr = loader_addr + cache.size();
                } else {
                    qDebug() << flash.loader_path << " not found.";
                }
                ExecuteAlgorithm(PROBE_CMD, 0, 0, NULL);
                sscanf(msg.constData(), "vFlashErase:%llx,%llx", &addr, &len);
                qDebug() << "Erase:" << QString("%1").arg(addr, 4, 16) << ":" << QString("%1").arg(len, 4, 16);
                ExecuteAlgorithm(ERASE_CMD, addr, len, NULL);
                send.append("OK");
                TransmitServerRsp(send);
            } else {
                char temp_buf[1024];
                sprintf(temp_buf, "vFlashInit:%llx,%llx;", flash.spi_base, flash.block_size);
                send.append(temp_buf);
                TransmitTargetCmd(send);
                if (WaitForTargetRsp()) {
                    recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                    if (recv.contains("OK")) {
                        qDebug() << "flash init ok.";
                    } else {
                        qDebug() << "flash init fail.";
                    }
                }
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
        } else if (strncmp(msg.constData(), "vFlashWrite:", 12) == 0) {
            if (workarea.addr) {
                sscanf(msg.constData(), "vFlashWrite:%llx:", &addr);
                cache = bin_decode(msg.mid(msg.indexOf(':', 15) + 1));
                len = cache.size();
                qDebug() << "Write:" << QString("%1").arg(addr, 4, 16) << ":" << QString("%1").arg(len, 4, 16);
                ExecuteAlgorithm(WRITE_CMD, addr, len, cache);
                send.append("OK");
                TransmitServerRsp(send);
            } else {
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
        } else if (strncmp(msg.constData(), "vFlashDone", 10) == 0) {
            if (workarea.addr) {
                //restore workarea
                if (workarea.backup) {
                    WriteTargetMemory(workarea.addr, workarea.mem, workarea.size);
                }
                send.append("OK");
                TransmitServerRsp(send);
            } else {
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
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
