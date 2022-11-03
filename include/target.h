#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QQueue>

class Target : public QObject
{
    Q_OBJECT
public:
    explicit Target(QObject *parent = nullptr);
    void TargetInit();
    void TargetDeinit();

    QString target_serial_name;
    qint32 target_serial_baud;

private:
    QSerialPort* target_serial_port;
    QByteArray target_msg;

private slots:
    void TargetSerialReadyRead();

public slots:
    void TargetWrite(QByteArray msg);
};

#endif // TARGET_H
