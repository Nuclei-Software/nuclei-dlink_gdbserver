#include "../include/serial.h"
#include <QRegularExpression>

extern bool debug;
QList<QSerialPortInfo> info;

Serial::Serial(QObject *parent)
    : QObject{parent}
{
    SerialBaud = 115200;
    vid = 0x28e9;
    pid = 0x018a;

    SerialPort = new QSerialPort;
    connect(this, SIGNAL(readyWrite(QByteArray)), this, SLOT(ReadyWrite(QByteArray)));
}

int Serial::Init()
{
    bool flag_serial = false;
    info = QSerialPortInfo::availablePorts();

#ifndef WIN32
    QString linux_dev = "/dev/";
    if (SerialName.isEmpty() == false && SerialName.startsWith(linux_dev) == true) {
        // If serial port name startswith /dev/ just remove it
        SerialName = SerialName.remove(0, 5);
    }
#endif

    if (SerialNumber.isEmpty() && SerialName.isEmpty()) {
        int last_num = 0xFFFFFFFF;
        QString temp;
        foreach (QSerialPortInfo port, info) {
            if ((pid == port.productIdentifier()) && (vid == port.vendorIdentifier())) {
                temp = port.portName().remove(QRegularExpression("[A-Z]"));
                if (last_num == 0xFFFFFFFF) {
                    last_num = temp.remove(QRegularExpression("[a-z]")).toInt();
                    SerialName = port.portName();
                    flag_serial = true;
                } else {
                    if (last_num > temp.remove(QRegularExpression("[a-z]")).toInt()) {
                        last_num = temp.remove(QRegularExpression("[a-z]")).toInt();
                        SerialName = port.portName();
                        flag_serial = true;
                    }
                }
            }
        }
        if (flag_serial == true) {
            qDebug() << "Using auto probed dlink debug serial port " << SerialName << ".";
        }
    } else {
        if (SerialNumber.isEmpty()) {
            foreach (QSerialPortInfo port, info) {
                if ((pid == port.productIdentifier()) && (vid == port.vendorIdentifier())) {
                    if (SerialName == port.portName()) {
                        SerialNumber = port.serialNumber();
                        flag_serial = true;
                    }
                }
            }
            if (flag_serial == false) {
                qDebug() << "Can't find serial port " << SerialName << ", please check it.";
            }
        } else if (SerialName.isEmpty()) {
            foreach (QSerialPortInfo port, info) {
                if ((pid == port.productIdentifier()) && (vid == port.vendorIdentifier())) {
                    if (SerialNumber == port.serialNumber()) {
                        SerialName = port.portName();
                        flag_serial = true;
                    }
                }
            }
            if (flag_serial == false) {
                qDebug() << "Can't find a dlink device match serial number " << SerialNumber << ", please check it.";
            }
        } else {
            foreach (QSerialPortInfo port, info) {
                if ((pid == port.productIdentifier()) && (vid == port.vendorIdentifier())) {
                    if ((SerialNumber == port.serialNumber()) && (SerialName == port.portName())) {
                        flag_serial = true;
                    }
                }
            }
            if (flag_serial == false) {
                qDebug() << "Can't find serial port:" << SerialName << " serial number:" << SerialNumber << ", please check it.";
            }
        }
    }
    if (flag_serial == false) {
        qDebug() << "Here are a list of possible dlink devices matched:";
        qint32 index = 0;
        QString last_serialNumber;
        QString device_info;
        foreach (QSerialPortInfo port, info) {
            if ((pid == port.productIdentifier()) && (vid == port.vendorIdentifier())) {
                if (last_serialNumber.compare(port.serialNumber()) == 0) {
                    device_info.append(", serial port ");
                    device_info.append(port.portName());
                    qDebug() << device_info;
                    device_info.clear();
                } else {
                    device_info.append("- dlink ");
                    device_info.append(QString::number(index));
                    device_info.append(": serial number ");
                    device_info.append(port.serialNumber());
                    device_info.append(": serial port ");
                    device_info.append(port.portName());
                    last_serialNumber = port.serialNumber();
                    index += 1;
                }
            }
        }
        qDebug() << "Usually the dlink debug port is a serial port with lower number, eg. COM0 in COM0/COM1 of dlink device";
    }
#ifndef WIN32
    if (SerialName.isEmpty() == false && SerialName.startsWith(linux_dev) == false) {
        // If serial port name not startswith /dev/, just add it
        SerialName = linux_dev + SerialName;
    }
#endif

    if (flag_serial == false || SerialName.isEmpty() == true) {
        qDebug() << "No matched dlink debug serial port found, please check your serial port configuration!";
        return -1;
    } else {
        qDebug() << "Using serial port " << SerialName << " as dlink debug port!";
    }
    SerialPort->setPortName(SerialName);
    SerialPort->setBaudRate(SerialBaud);
    SerialPort->setDataBits(QSerialPort::Data8);
    SerialPort->setParity(QSerialPort::NoParity);
    SerialPort->setStopBits(QSerialPort::OneStop);
    SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    if (SerialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Opened " << SerialName << " baud:" << SerialBaud;
    } else {
        qDebug() << "Failed to open:" << SerialName;
        return -1;
    }
    connect(SerialPort, SIGNAL(readyRead()), this, SLOT(ReadyRead()));
    return 0;
}

void Serial::Deinit()
{
    disconnect(SerialPort, 0, 0, 0);
    SerialPort->close();
    qDebug() << "Close:" << SerialName;
    SerialBaud = 115200;
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
