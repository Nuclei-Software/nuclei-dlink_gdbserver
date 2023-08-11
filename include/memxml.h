#ifndef MEMXML_H
#define MEMXML_H

#include <QObject>
#include "misa.h"
#include "algorithm.h"

class MemXml : public QObject
{
    Q_OBJECT
public:
    explicit MemXml(QObject *parent = nullptr);

    Algorithm::flash_t flash;

    void InitMemXml(Misa* misa);
    quint32 GetMemXmlLen();
    QByteArray GetMemXml(quint32 addr);

private:
    QByteArray memxml;

signals:

};

#endif // MEMXML_H
