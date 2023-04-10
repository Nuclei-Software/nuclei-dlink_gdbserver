#include "../include/target.h"

extern bool debug;

QQueue<QByteArray> target_rsp_queue;
QList<QSerialPortInfo> info;

Target::Target(QObject *parent) : QObject(parent)
{
    target_serial_port = new QSerialPort;
}

void Target::TargetInit()
{
    bool flag_serial = false;
    info = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo port, info) {
        if ((0x018a == port.productIdentifier()) && (0x28e9 == port.vendorIdentifier())) {
            if (target_serial_number.isEmpty()) {
                if (target_serial_name.isEmpty()) {
                    target_serial_name = port.portName();
                    target_serial_number = port.serialNumber();
                    flag_serial = true;
                    break;
                } else {
                    if (target_serial_name == port.portName()) {
                        target_serial_name = port.portName();
                        target_serial_number = port.serialNumber();
                        flag_serial = true;
                        break;
                    }
                }
            } else if (target_serial_name.isEmpty()) {
                if (target_serial_number == port.serialNumber()) {
                    target_serial_name = port.portName();
                    target_serial_number = port.serialNumber();
                    flag_serial = true;
                    break;
                }
            } else {
                if (((target_serial_number == port.serialNumber()) && (target_serial_name != port.portName())) ||
                    ((target_serial_number != port.serialNumber()) && (target_serial_name == port.portName()))) {
                    qDebug() << target_serial_name << " and " << target_serial_number << "is not match.";
                    return;
                }
                if ((target_serial_number == port.serialNumber()) && (target_serial_name == port.portName())) {
                    flag_serial = true;
                    break;
                }
            }
        }
    }
    if (flag_serial) {
        qDebug() << "portName:" << target_serial_name << " serialNumber:" << target_serial_number;
    } else {
        qDebug() << "Not found Dlink, Try the following device:";
        QString last_number;
        foreach (QSerialPortInfo port, info) {
            if ((0x018a == port.productIdentifier()) && (0x28e9 == port.vendorIdentifier())) {
                if (last_number != port.serialNumber()) {
                    qDebug() << "portName " << port.portName() << " serialNumber " << port.serialNumber();
                    last_number = port.serialNumber();
                }
            }
        }
        return;
    }
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
    target_serial_name.clear();
    target_serial_number.clear();
    target_serial_port->close();
    qDebug() << "Close:" << target_serial_name;
}

void Target::TargetSerialReadyRead()
{
    target_msg.append(target_serial_port->readAll());
    if ((target_msg.contains('$')) && (target_msg.contains('#')) && (target_msg.contains('|'))) {
        foreach (QByteArray msg, target_msg.split('|')) {
            if ((msg.contains('$')) && (msg.contains('#'))) {
                if (debug) {
                    qDebug() << "T->:" << msg;
                }
                target_rsp_queue.enqueue(msg.mid(msg.indexOf('$') + 1,
                                                 msg.indexOf('#') - msg.indexOf('$') - 1));
            }
        }
        target_msg.clear();
    }
}

void Target::TargetWrite(QByteArray msg)
{
    if (debug) {
        qDebug() << "->T:" << msg;
    }
    target_serial_port->write(msg);
}
