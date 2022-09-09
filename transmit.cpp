#include "transmit.h"
#include "mainwindow.h"

QString version = "V1.0.0";

QQueue<QString> log_queue;

Transmit::Transmit(int argc, char *argv[])
{
    window = new MainWindow;
    window->setWindowTitle("Nuclei Dlink GDB Server");
    gdb_server = new Server();
    rv_target = new Target;
    connect(gdb_server, SIGNAL(ServerToTarget(QByteArray)), rv_target, SLOT(TargetWrite(QByteArray)));
    connect(rv_target, SIGNAL(TargetToServer(QByteArray)), gdb_server, SLOT(ServerWrite(QByteArray)));
    if (argc <= 1) {
        window->install_message_handler();
        window->show();
        qDebug() << "Nuclei Dlink GDB Server " << version << "Command Line Version" << Qt::endl;
        //TODO:
        connect(window, SIGNAL(ui_connect(QString, QString, QString, QString)),
                this, SLOT(user_connect(QString, QString, QString, QString)));
        connect(window, SIGNAL(ui_close()), this, SLOT(user_close()));
    } else {
        port = "3333";
        #ifdef Q_WS_WIN32
        serial = "COM1";
        #else
        serial = "/dev/ttyUSB1";
        #endif
        baud = "115200";
        interface = TARGET_INTERFACE_JTAG;
        for (int i = 1;i < argc;i += 2) {
            if ((0 == strncmp(argv[i], "-p", strlen(argv[i]))) || \
                (0 == strncmp(argv[i], "--port", strlen(argv[i])))) {
                port = QString(argv[i + 1]);
            } else if ((0 == strncmp(argv[i], "-s", strlen(argv[i]))) || \
                       (0 == strncmp(argv[i], "--serial", strlen(argv[i])))) {
                serial = QString(argv[i + 1]);
            } else if ((0 == strncmp(argv[i], "-b", strlen(argv[i]))) || \
                       (0 == strncmp(argv[i], "--baud", strlen(argv[i])))) {
                baud = QString(argv[i + 1]);
            } else if ((0 == strncmp(argv[i], "-i", strlen(argv[i]))) || \
                       (0 == strncmp(argv[i], "--interface", strlen(argv[i])))) {
                if (0 == strncmp(argv[i + 1], "jtag", strlen(argv[i + 1]))) {
                    interface = TARGET_INTERFACE_JTAG;
                } else if (0 == strncmp(argv[i + 1], "cjtag", strlen(argv[i + 1]))) {
                    interface = TARGET_INTERFACE_CJTAG;
                }
            } else {
                qDebug() << "These are common Nuclei Dlink GDB Server commands used in various situations:" << Qt::endl;
                qDebug() << "    command        parameter        example   example";
                qDebug() << "    -p/--port      port number      -p 3333   --port 3334";
                qDebug() << "    -s/--serial    serial name      -s COM5   --serial /dev/ttyUSB1";
                qDebug() << "    -b/--baud      serial baud rate -b 115200 --baud 9600";
                qDebug() << "    -i/--interface target interface -i jtag   --interface cjtag";
                qDebug() << Qt::endl << "----END----" << Qt::endl;
            }
        }
        qDebug() << "Nuclei Dlink GDB Server " << version << "Command Line Version" << Qt::endl;
        gdb_server->server_port = port.toUShort(nullptr, 10);
        gdb_server->ServerInit();
        rv_target->target_serial_name = serial;
        rv_target->target_serial_baud = baud.toInt(nullptr, 10);
        rv_target->target_interface = interface;
        rv_target->TargetInit();
    }
}

Transmit::~Transmit()
{
    window->close();
    delete window;
    delete gdb_server;
    delete rv_target;
}

void Transmit::run()
{
    while (1) {
        if (!log_queue.empty()) {
            window->output_log(log_queue.dequeue());
        }
        if (close_flag) {
            break;
        }
    }
}

void Transmit::user_connect(QString p, QString s, QString b, QString i)
{
    gdb_server->server_port = p.toUShort(nullptr, 10);
    gdb_server->ServerInit();
    rv_target->target_serial_name = s;
    rv_target->target_serial_baud = b.toInt(nullptr, 10);
    if ("jtag" == i) {
        rv_target->target_interface = TARGET_INTERFACE_JTAG;
    } else {
        rv_target->target_interface = TARGET_INTERFACE_CJTAG;
    }
    rv_target->TargetInit();
}

void Transmit::user_close()
{
    close_flag = true;
}
