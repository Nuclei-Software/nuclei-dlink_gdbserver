#include "../include/application_console.h"

QByteArray version = "v1.0.0";
bool debug = false;
bool noack_mode = false;
quint64 target_packet_max = 0x400;

Application::Application(QObject *parent)
    : QObject{parent}
{
    transmit = new Transmit;
}

void Application::Init(int argc, char *argv[])
{
    if (argc == 2) {
        for (int i = 0; i < argc; i++) {
            if (strncmp(argv[i], "-v", 2) == 0)
            {
                qDebug("Dlink GDB Server: %s Version", version.constData());
                exit(0);
            }
        }
    } else if (argc == 3) {
        for (int i = 0; i < argc; i++) {
            if (strncmp(argv[i], "-f", 2) == 0) {
                //start app
                Connect(argv[i + 1]);
                break;
            } else if (strncmp(argv[i], "-v", 2) == 0)
            {
                qDebug("Dlink GDB Server: %s Version", version.constData());
            }
        }
    } else {
        qDebug() << "Command Error:";
        qDebug() << "Example: ./dlink_gdbserver_console -v";
        qDebug() << "Example: ./dlink_gdbserver_console -f dlink_gdbserver.cfg";
        exit(-1);
    }
}

void Application::Connect(QString cfg_path)
{
    QFile cfg(cfg_path);

    transmit->Reset();

    if (cfg.exists()) {
        cfg.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!cfg.atEnd()) {
            QString line = cfg.readLine();
            line.remove('\n');
            if (line.contains('#')) {
                continue;
            }
            //split cfg line
            QList<QString> command = line.split(' ');
            if (0 == command[0].compare("debug")) {
                if (0 == command[1].compare("true")) {
                    debug = true;
                } else {
                    debug = false;
                }
            }
            //gdb command group
            if (0 == command[0].compare("gdb")) {
                if (0 == command[1].compare("port")) {
                    transmit->server->Port = command[2].toUShort(nullptr, 10);
                }
            }
            //serial command group
            if (0 == command[0].compare("serial")) {
                if (0 == command[1].compare("port")) {
                    transmit->target->serial->SerialName = command[2];
                } else if (0 == command[1].compare("baud")) {
                    transmit->target->serial->SerialBaud = command[2].toUInt(nullptr, 10);
                } else if (0 == command[1].compare("number")) {
                    transmit->target->serial->SerialNumber = command[2];
                }
            }
            //transport command group
            if (0 == command[0].compare("transport")) {
                if (0 == command[1].compare("select")) {
                    transmit->protocol = command[2];
                }
            }
            //workarea command group
            if (0 == command[0].compare("workarea")) {
                if (0 == command[1].compare("addr")) {
                    transmit->algorithm->workarea.addr = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("size")) {
                    transmit->algorithm->workarea.size = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("backup")) {
                    if (0 == command[2].compare("true")) {
                        transmit->algorithm->workarea.backup = true;
                    } else {
                        transmit->algorithm->workarea.backup = false;
                    }
                }
            }
            //flash command group
            if (0 == command[0].compare("flash")) {
                if (0 == command[1].compare("spi_base")) {
                    transmit->algorithm->flash.spi_base = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("xip_base")) {
                    transmit->algorithm->flash.xip_base = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("xip_size")) {
                    transmit->algorithm->flash.xip_size = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("block_size")) {
                    transmit->algorithm->flash.block_size = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("loader_path")) {
                    if (QFile::exists(command[2])) {
                        transmit->algorithm->flash.loader_path = command[2];
                    } else {
#ifdef WIN32
                        transmit->algorithm->flash.loader_path = cfg_path.mid(0, cfg_path.lastIndexOf('/') + 1) +
                                                    command[2].mid(command[2].lastIndexOf('\\') + 1);
#else
                        transmit->algorithm->flash.loader_path = cfg_path.mid(0, cfg_path.lastIndexOf('/')) +
                                                    command[2].mid(command[2].lastIndexOf('/'));
#endif
                    }
                }
            }
        }
    } else {
        qDebug() << cfg_path;
        qDebug() << "dlink_gdbserver.cfg not found!";
        exit(-1);
    }

    transmit->Init();
    transmit->start();
}

void Application::Disconnect()
{
    transmit->Close();
    transmit->Deinit();

    disconnect(transmit, 0, 0, 0);

    qDebug() << "Disconnect";
}
