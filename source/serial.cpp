#include "../include/serial.h"

extern bool debug;
QList<QSerialPortInfo> info;

Serial::Serial(QObject *parent)
    : QObject{parent}
{
    SerialPort = new QSerialPort;
    connect(this, SIGNAL(readyWrite(QByteArray)), this, SLOT(ReadyWrite(QByteArray)));
}

void Serial::Init()
{
    bool flag_serial = false;
    info = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo port, info) {
        if ((0x018a == port.productIdentifier()) && (0x28e9 == port.vendorIdentifier())) {
            if (SerialNumber.isEmpty()) {
                if (SerialName.isEmpty()) {
                    SerialName = port.portName();
                    SerialNumber = port.serialNumber();
                    flag_serial = true;
                    break;
                } else {
                    if (SerialName == port.portName()) {
                        SerialName = port.portName();
                        SerialNumber = port.serialNumber();
                        flag_serial = true;
                        break;
                    }
                }
            } else if (SerialName.isEmpty()) {
                if (SerialNumber == port.serialNumber()) {
                    SerialName = port.portName();
                    SerialNumber = port.serialNumber();
                    flag_serial = true;
                    break;
                }
            } else {
                if (((SerialNumber == port.serialNumber()) && (SerialName != port.portName())) ||
                    ((SerialNumber != port.serialNumber()) && (SerialName == port.portName()))) {
                    qDebug() << SerialName << " and " << SerialNumber << "is not match.";
                    return;
                }
                if ((SerialNumber == port.serialNumber()) && (SerialName == port.portName())) {
                    flag_serial = true;
                    break;
                }
            }
        }
    }
    if (flag_serial) {
        qDebug() << "portName:" << SerialName << " serialNumber:" << SerialNumber;
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
    SerialPort->setPortName(SerialName);
    SerialPort->setBaudRate(SerialBaud);
    SerialPort->setDataBits(QSerialPort::Data8);
    SerialPort->setParity(QSerialPort::NoParity);
    SerialPort->setStopBits(QSerialPort::OneStop);
    SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    if (SerialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Open:" << SerialName << " baud:" << SerialBaud;
    } else {
        qDebug() << "Fail to open:" << SerialName;
        return;
    }
    connect(SerialPort, SIGNAL(readyRead()), this, SLOT(ReadyRead()));
}

void Serial::Deinit()
{
    disconnect(SerialPort, 0, 0, 0);
    SerialPort->close();
    qDebug() << "Close:" << SerialName;
    SerialBaud = 0;
    SerialName.clear();
    SerialNumber.clear();
    Message.clear();
}

void Serial::ReadyRead()
{
    Message.append(SerialPort->readAll());
}

void Serial::ReadyWrite(QByteArray msg)
{
    SerialPort->write(msg);
}

void Serial::Write(QByteArray msg)
{
    emit readyWrite(msg);
}

QByteArray Serial::Read()
{
    QByteArray Temp;
    if (Message.size()) {
        if (Message.contains('$') && Message.contains('#')) {
            while(1) {
                if (Message[0] == '$') {
                    Temp = Message.mid(0, Message.indexOf('#') + 1);
                    Message.remove(0, Message.indexOf('#') + 4);
                    break;
                }
                Message.removeFirst();
            }
        }
    }
    return Temp;
}
