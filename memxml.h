#ifndef MEMXML_H
#define MEMXML_H

#include <QObject>
#include "misa.h"

class MemXml : public QObject
{
    Q_OBJECT
public:
    explicit MemXml(QObject *parent = nullptr);
    void InitMemXml(Misa* misa);
    quint32 GetMemXmlLen(void);
    QByteArray GetMemXml(quint32 addr);

private:
    QByteArray memxml;
    quint64 max_addr;

    typedef struct {
        QByteArray type;
        quint64 start;
        quint64 length;
        quint64 blocksize;
    }flash_t;

    QList<flash_t> flashs = {
        /* type    start        length        blocksize */
        {"flash",  0x20000000,  0x10000000,   0x10000},
    };

signals:

};

#endif // MEMXML_H
