#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>

class Serial : public QObject
{
    Q_OBJECT
public:
    explicit Serial(QObject *parent = nullptr);

    void Init();
    void Deinit();
    void Write(QByteArray msg);
    QByteArray Read();

    QString SerialName;
    qint32 SerialBaud;
    QString SerialNumber;
    quint16 vid;
    quint16 pid;

private:
    QSerialPort* SerialPort;
    QByteArray Message;

signals:
    void readyWrite(QByteArray msg);

private slots:
    void ReadyRead();
    void ReadyWrite(QByteArray msg);
};

#endif // SERIAL_H
