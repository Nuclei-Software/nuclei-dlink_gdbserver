#include "../include/application.h"

QByteArray version = "v0.9.0";
bool debug = false;
bool noack_mode = false;
quint64 target_packet_max = 0x400;

Application::Application(QObject *parent)
    : QObject{parent}
{
    mainwindow = new MainWindow;
    logout = new Logout;
    transmit = new Transmit;
    logout->start();
}

void Application::Init(int argc, char *argv[])
{
    mainwindow->setWindowTitle("Nuclei Dlink GDB Server");
    mainwindow->install_message_handler();
    mainwindow->show();
    connect(mainwindow, SIGNAL(Close()), logout, SLOT(Close()));
    connect(mainwindow, SIGNAL(Close()), transmit, SLOT(Close()));
    connect(logout, SIGNAL(Toui(QString)), mainwindow, SLOT(output_log(QString)));
    connect(mainwindow, SIGNAL(Connect(QString)), this, SLOT(Connect(QString)));
    connect(mainwindow, SIGNAL(Disconnect()), this, SLOT(Disconnect()));
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
    transmit->Deinit();
    disconnect(transmit, 0, 0, 0);
    transmit->Close();

    qDebug() << "Disconnect";
}
