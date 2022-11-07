#include "../include/application.h"

Application::Application(QObject *parent)
    : QObject{parent}
{
    mainwindow = new MainWindow;
    logout = new Logout;
    transmit = new Transmit;
    target = new Target;
    server = new Server;

    logout->start();
}

void Application::ApplicationInit(int argc, char *argv[])
{
    switch (argc) {
    case 1://gui mode
        mainwindow->setWindowTitle("Nuclei Dlink GDB Server");
        mainwindow->install_message_handler();
        mainwindow->show();
        connect(mainwindow, SIGNAL(Close()), logout, SLOT(LogoutClose()));
        connect(mainwindow, SIGNAL(Close()), transmit, SLOT(TransmitClose()));
        connect(logout, SIGNAL(LogoutToUI(QString)), mainwindow, SLOT(output_log(QString)));
        connect(mainwindow, SIGNAL(Connect(QString)), this, SLOT(ApplicationConnect(QString)));
        connect(mainwindow, SIGNAL(Disconnect()), this, SLOT(ApplicationDisconnect()));
        break;
    case 3://command line mode
        if (0 == strncmp(argv[2], "-f", 2)) {
            ApplicationConnect(argv[3]);
        }
        break;
    default://help message
        qErrnoWarning("dlink_gdbserver -f ~/dlink_gdbserver.cfg");
        break;
    }
}

void Application::ApplicationConnect(QString cfg_path)
{
    QFile cfg(cfg_path);

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
            //gdb command group
            if (0 == command[0].compare("gdb")) {
                if (0 == command[1].compare("port")) {
                    server->server_port = command[2].toUShort(nullptr, 10);
                }
            }
            //serial command group
            if (0 == command[0].compare("serial")) {
                if (0 == command[1].compare("port")) {
                    target->target_serial_name = command[2];
                } else if (0 == command[1].compare("baud")) {
                    target->target_serial_baud = command[2].toUInt(nullptr, 10);
                }
            }
            //transport command group
            if (0 == command[0].compare("transport")) {
                if (0 == command[1].compare("select")) {
                    transmit->interface = command[2];
                }
            }
            //workarea command group
            if (0 == command[0].compare("workarea")) {
                if (0 == command[1].compare("addr")) {
                    transmit->workarea.addr = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("size")) {
                    transmit->workarea.size = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("backup")) {
                    if (0 == command[2].compare("true")) {
                        transmit->workarea.backup = true;
                    } else {
                        transmit->workarea.backup = false;
                    }
                }
            }
            //flash command group
            if (0 == command[0].compare("flash")) {
                if (0 == command[1].compare("spi_base")) {
                    transmit->flash.spi_base = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("xip_base")) {
                    transmit->flash.xip_base = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("xip_size")) {
                    transmit->flash.xip_size = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("block_size")) {
                    transmit->flash.block_size = command[2].toUInt(nullptr, 16);
                } else if (0 == command[1].compare("loader_path")) {
                    transmit->flash.loader_path = command[2];
                }
            }
        }
    }

    connect(transmit, SIGNAL(TransmitToTarget(QByteArray)), target, SLOT(TargetWrite(QByteArray)));
    connect(transmit, SIGNAL(TransmitToServer(QByteArray)), server, SLOT(ServerWrite(QByteArray)));

    transmit->TransmitInit();
    target->TargetInit();
    server->ServerInit();

    transmit->start();
}

void Application::ApplicationDisconnect()
{
    transmit->TransmitClose();
    transmit->TransmitDeinit();
    target->TargetDeinit();
    server->ServerDeinit();

    disconnect(transmit, 0, 0, 0);
    disconnect(target, 0, 0, 0);
    disconnect(server, 0, 0, 0);

    qDebug() << "Disconnect";
}
