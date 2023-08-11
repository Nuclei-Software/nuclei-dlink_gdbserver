#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QDebug>
#include "type.h"
#include "serial.h"
#include "misa.h"

class Target : public QObject
{
    Q_OBJECT
public:
    explicit Target(QObject *parent = nullptr);

    Serial* serial;
    Misa* misa;

    void Init();
    void Deinit();
    void SendCmd(QByteArray msg);
    QByteArray GetRsp();
    void WriteMemory(quint64 addr, QByteArray data, quint32 length);
    QByteArray ReadMemory(quint64 addr, quint32 length);
    void WriteRegister(quint32 number, quint64 value);
    quint64 ReadRegister(quint32 number);

private:
    Type* type;
};

#endif // TARGET_H
