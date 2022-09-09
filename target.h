#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QQueue>

typedef enum {
    TARGET_INTERFACE_JTAG,
    TARGET_INTERFACE_CJTAG,
    TARGET_INTERFACE_TWDI,
    TARGET_INTERFACE_NW,
    TARGET_INTERFACE_MAX,
} target_interface_t;

class Target : public QObject
{
    Q_OBJECT
public:
    explicit Target(QObject *parent = nullptr);
    ~Target();
    void TargetInit();

    QString target_serial_name;
    qint32 target_serial_baud;
    target_interface_t target_interface;
    QQueue<QByteArray> target_queue;

private:
    QSerialPort* target_serial_port;

signals:
    void TargetToServer(QByteArray);

public slots:
    void TargetSerialReadyRead();
    void TargetWrite(QByteArray msg);
};

#endif // TARGET_H
