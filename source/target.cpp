#include "../include/target.h"

QQueue<QByteArray> target_rsp_queue;

Target::Target(QObject *parent) : QObject(parent)
{
    target_serial_port = new QSerialPort;
}

void Target::TargetInit()
{
    target_serial_port->setPortName(target_serial_name);
    target_serial_port->setBaudRate(target_serial_baud);
    target_serial_port->setDataBits(QSerialPort::Data8);
    target_serial_port->setParity(QSerialPort::NoParity);
    target_serial_port->setStopBits(QSerialPort::OneStop);
    target_serial_port->setFlowControl(QSerialPort::NoFlowControl);
    if (target_serial_port->open(QIODevice::ReadWrite)) {
        qDebug() << "Open:" << target_serial_name << " baud:" << target_serial_baud;
    } else {
        qDebug() << "Fail to open:" << target_serial_name;
        return;
    }
    connect(target_serial_port, SIGNAL(readyRead()), this, SLOT(TargetSerialReadyRead()));
}

void Target::TargetDeinit()
{
    target_serial_port->close();
    qDebug() << "Close:" << target_serial_name;
}

void Target::TargetSerialReadyRead()
{
    target_msg.append(target_serial_port->readAll());
    if ((target_msg.contains('$')) && (target_msg.contains('#')) && (target_msg.contains('|'))) {
        foreach (QByteArray msg, target_msg.split('|')) {
            if ((msg.contains('$')) && (msg.contains('#'))) {
                qDebug() << "T->:" << msg;
                target_rsp_queue.enqueue(msg.mid(msg.indexOf('$') + 1,
                                                 msg.indexOf('#') - msg.indexOf('$') - 1));
            }
        }
        target_msg.clear();
    }
}

void Target::TargetWrite(QByteArray msg)
{
    qDebug() << "->T:" << msg;
    target_serial_port->write(msg);
}
