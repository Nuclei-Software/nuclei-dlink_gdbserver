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
    ~Target();
    void TargetInit();

    QString target_serial_name;
    qint32 target_serial_baud;
    QQueue<QByteArray> target_queue;

private:
    QSerialPort* target_serial_port;

    QByteArray target_msg;

signals:
    void TargetToServer(QByteArray);

public slots:
    void TargetSerialReadyRead();
    void TargetWrite(QByteArray msg);
};

#endif // TARGET_H
