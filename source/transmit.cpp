#include "../include/transmit.h"

QString version = "V1.0.0";
extern bool debug;
extern bool noack_mode;
extern quint64 target_packet_max;

Transmit::Transmit()
{
    type = new Type;
    server = new Server;
    target = new Target;
    algorithm = new Algorithm;
    memxml = new MemXml;
    regxml = new RegXml;
    cpuinfo = new Cpuinfo;
    etrace = new Etrace;
}

void Transmit::Reset()
{
    close_flag = false;
    algorithm->flash.spi_base = 0;
    algorithm->flash.xip_base = 0;
    algorithm->flash.xip_size = 0;
    algorithm->flash.block_size = 0;
    algorithm->flash.loader_path = NULL;
    algorithm->workarea.size = 0;
    algorithm->workarea.addr = 0;
    algorithm->workarea.backup = false;
    algorithm->workarea.mem = NULL;
    target_run_flag = false;
}

void Transmit::Init()
{
    server->Init();
    target->Init();
    qDebug() << "Nuclei Dlink GDB Server " << version << "Command Line Version";
    fprintf(stderr, "Started by GNU MCU Eclipse\n");
}

void Transmit::Deinit()
{
    server->Deinit();
    target->Deinit();
    etrace->etrace_addr = 0;
    etrace->buffer_addr = 0;
    etrace->buffer_size = 0;
    etrace->timeout = 0;
    etrace->wrap = 0;
}

void Transmit::ServerCmdDeal(QByteArray msg)
{
    QByteArray send, recv, cache;
    quint64 addr, len, value;
    char temp[1024];
    if (msg[0] == '\x03') { /* Ctrl+C command */
        target->SendCmd(msg);
        server->Write(target->GetRsp());
    } else {
        switch (msg[0]) {
        case 'q':
            if (msg.contains("qSupported")) {
                noack_mode = false;
                //deal qSupported
                send.clear();
                sprintf(temp, "PacketSize=%llx;", target_packet_max+5);
                send.append(temp);
                send.append("QStartNoAckMode+;");
                send.append("qXfer:features:read+;");
                send.append("qXfer:memory-map:read+;");
                send.append("swbreak+;");
                send.append("hwbreak+;");
                server->Write(send);
                //Set protocol and connect target
                send.clear();
                send.append("+:set:protocol:");
                send.append(protocol.toLatin1());
                send.append(';');
                target->SendCmd(send);
                recv = target->GetRsp();
                if (recv.contains("protocol")) {
                    qDebug() << "set protocol and connect success.";
                } else {
                    qDebug() << "set protocol and connect fail.";
                }
                //Get target MISA CSR register
                send.clear();
                send.append("+:read:misa;");
                target->SendCmd(send);
                recv = target->GetRsp();
                if (recv.contains("misa")) {
                    sscanf(recv.constData(), "-:read:misa:%08llx;", &value);
                    target->misa->MisaInit(value);
                    memxml->flash = algorithm->flash;
                    memxml->InitMemXml(target->misa);
                    qDebug() << "read misa:" << QString("%1").arg(value, 4, 16);
                } else {
                    qDebug() << "read misa fail.";
                }
                //Get target VLENB CSR register
                send.clear();
                send.append("+:read:vlenb;");
                target->SendCmd(send);
                recv = target->GetRsp();
                if (recv.contains("vlenb")) {
                    sscanf(recv.constData(), "-:read:vlenb:%016llx;", &value);
                    regxml->InitRegXml(target->misa, value);
                    qDebug() << "read vlenb:" << QString("%1").arg(value, 4, 16);
                } else {
                    qDebug() << "read vlenb fail.";
                }
            } else if (msg.contains("qXfer:memory-map:read::")) {
                sscanf(msg.constData(), "qXfer:memory-map:read::%llx,%llx", &addr, &len);
                value = memxml->GetMemXmlLen();
                if (len >= (value - addr)) {
                    len = value - addr;
                    send.append('l');
                } else {
                    send.append('m');
                }
                send.append(memxml->GetMemXml(addr).constData(), len);
                server->Write(send);
            } else if (msg.contains("qXfer:features:read:target.xml:")) {
                sscanf(msg.constData(), "qXfer:features:read:target.xml:%llx,%llx", &addr, &len);
                value = regxml->GetRegXmlLen();
                if (len >= (value - addr)) {
                    len = value - addr;
                    send.append('l');
                } else {
                    send.append('m');
                }
                send.append(regxml->GetRegXml(addr).constData(), len);
                server->Write(send);
            } else if (msg.contains("qRcmd,")) {
                bool command_error = false;
                cache = type->hex_to_bin(msg.mid(msg.indexOf(',') + 1), msg.length() - msg.indexOf(','));
                QList<QByteArray> commands = cache.split(' ');
                if (cache.contains("nuclei")) {
                    if (cache.contains("cpuinfo")) {
                        cpuinfo->ShowInfo(target, server);
                    } else if (cache.contains("etrace") && cache.contains("config")) {
                        if (commands.count() == 8) {
                            etrace->etrace_addr = commands[3].toLongLong();
                            etrace->buffer_addr = commands[4].toLongLong();
                            etrace->buffer_size = commands[5].toLongLong();
                            etrace->timeout = commands[6].toLongLong();
                            etrace->wrap = commands[7].toLongLong();
                            etrace->Config(target);
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("enable")) {
                        if (commands.count() == 3) {
                            etrace->Enable(target);
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("disable")) {
                        if (commands.count() == 3) {
                            etrace->Disable(target);
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("start")) {
                        if (commands.count() == 3) {
                            etrace->Start(target);
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("stop")) {
                        if (commands.count() == 3) {
                            etrace->Stop(target);
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("dump")) {
                        if (commands.count() == 4) {
                            etrace->Dump(target, commands[3].data());
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("clear")) {
                        if (commands.count() == 3) {
                            etrace->Clear(target);
                        } else {
                            command_error = true;
                        }
                    } else if (cache.contains("etrace") && cache.contains("info")) {
                        if (commands.count() == 3) {
                            etrace->Info(target, server);
                        } else {
                            command_error = true;
                        }
                    } else {
                        command_error = true;
                    }
                    if (command_error) {
                        send.append("nuclei cpuinfo\n");
                        send.append("nuclei etrace config etrace_addr buffer_addr buffer_size timeout wrap\n");
                        send.append("nuclei etrace enable\n");
                        send.append("nuclei etrace disable\n");
                        send.append("nuclei etrace start\n");
                        send.append("nuclei etrace stop\n");
                        send.append("nuclei etrace dump file_path\n");
                        send.append("nuclei etrace clear\n");
                        send.append("nuclei etrace info\n");
                        server->Write(type->bin_to_hex(send, send.size()));
                    }
                } else {
                    target->SendCmd(msg);
                    server->Write(target->GetRsp());
                }
            } else {
                /* Not support 'q' command, reply empty. */
                server->Write(send);
            }
            break;
        case 'Q':
            if (msg.contains("QStartNoAckMode")) {
                target->SendCmd(msg);
                send.append("OK");
                server->Write(send);
                target->GetRsp();
                noack_mode = true;
            } else {
                /* Not support 'Q' command, reply empty. */
                server->Write(send);
            }
            break;
        case 'H':/* `H op thread-id` */
            /* Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
            server->Write(send);
            break;
        case '?':/* `?` */
            /* Indicate the reason the target halted. The reply is the same as for step and continue. */
            send.append("S02");
            server->Write(send);
            break;
        case 'g':/* `g` */
            /* Read general registers. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'G':/* `G XX...` */
            /* Write general registers. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'k':/* `k` */
            /* Kill request. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'c':/* `c [addr]` */
            /* Continue at addr, which is the address to resume. */
            /* If addr is omitted, resume at current address. */
            target->SendCmd(msg);
            target_run_flag = true;
            break;
        case 'm':/* `m addr,length` */
            /* Read length addressable memory units starting at address addr. */
            /* Note that addr may not be aligned to any particular boundary. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'M':/* `M addr,length:XX...` */
            /* Write length addressable memory units starting at address addr. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'X':/* `X addr,length:XX...` */
            /* Write data to memory, where the data is transmitted in binary. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'p':/* `p n` */
            /* Read the value of register n; n is in hex. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'P':/* `P n...=r...` */
            /* Write register n... with value r... The register number n is in hexadecimal, */
            /* and r... contains two hex digits for each byte in the register (target byte order). */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 's':/* `s [addr]` */
            /* Single step, resuming at addr. If addr is omitted, resume at same address. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'z':/* `z type,addr,kind` */
            /* remove a type breakpoint or watchpoint starting at address, address of kind. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'Z':/* `Z type,addr,kind` */
            /* Insert a type breakpoint or watchpoint starting at address, address of kind. */
            target->SendCmd(msg);
            server->Write(target->GetRsp());
            break;
        case 'v':
            if (msg.contains("vMustReplyEmpty")) {
                server->Write(send);
            } else if (msg.contains("vFlashErase:")) {
                if (algorithm->workarea.addr) {
                    algorithm->BackupWorkarea(target);
                    algorithm->DownloadLoader(target);
                    algorithm->Execute(target, PROBE_CMD, 0, 0, NULL);
                    sscanf(msg.constData(), "vFlashErase:%llx,%llx", &addr, &len);
                    qDebug() << "Erase:" << QString("%1").arg(addr, 4, 16) << ":" << QString("%1").arg(len, 4, 16);
                    algorithm->Execute(target, ERASE_CMD, addr, len, NULL);
                    send.append("OK");
                    server->Write(send);
                } else {
                    sprintf(temp, "vFlashInit:%llx,%llx;", algorithm->flash.spi_base, algorithm->flash.block_size);
                    send.append(temp);
                    target->SendCmd(send);
                    recv = target->GetRsp();
                    if (recv.contains("OK")) {
                        qDebug() << "flash init ok.";
                    } else {
                        qDebug() << "flash init fail.";
                    }
                    target->SendCmd(msg);
                    server->Write(target->GetRsp());
                }
            } else if (msg.contains("vFlashWrite:")) {
                if (algorithm->workarea.addr) {
                    sscanf(msg.constData(), "vFlashWrite:%llx:", &addr);
                    cache = type->bin_decode(msg.mid(msg.indexOf(':', 15) + 1));
                    len = cache.size();
                    qDebug() << "Write:" << QString("%1").arg(addr, 4, 16) << ":" << QString("%1").arg(len, 4, 16);
                    algorithm->Execute(target, WRITE_CMD, addr, len, cache);
                    send.append("OK");
                    server->Write(send);
                } else {
                    target->SendCmd(msg);
                    server->Write(target->GetRsp());
                }
            } else if (msg.contains("vFlashDone")) {
                if (algorithm->workarea.addr) {
                    algorithm->RestoreWorkarea(target);
                    send.append("OK");
                    server->Write(send);
                } else {
                    target->SendCmd(msg);
                    server->Write(target->GetRsp());
                }
            } else {
                /* Not support 'v' command, reply empty. */
                server->Write(send);
            }
            break;
        default:/* Not support command, reply empty. */
            server->Write(send);
            break;
        }
    }
}

void Transmit::run()
{
    QByteArray msg;

    while (1) {
        if (close_flag) {
            break;
        }
        msg = server->Read();
        if (msg.size()) {
            ServerCmdDeal(msg);
        }
        if (target_run_flag) {
            msg = target->serial->Read();
            if (msg.size()) {
                server->Write(type->unpack(msg));
                target_run_flag = false;
            }
        }
    }
}

void Transmit::Close()
{
    close_flag = true;
}
