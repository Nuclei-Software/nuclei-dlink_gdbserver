#ifndef MEMXML_H
#define MEMXML_H

#include <QObject>
#include "../include/misa.h"
#include "../include/algorithm.h"

class MemXml : public QObject
{
    Q_OBJECT
public:
    explicit MemXml(QObject *parent = nullptr);

    void AddFlash(Algorithm::flash_t flash);
    void InitMemXml(Misa* misa);
    quint32 GetMemXmlLen(void);
    QByteArray GetMemXml(quint32 addr);

private:
    QByteArray memxml;
    QList<Algorithm::flash_t> flashs;

signals:

};

#endif // MEMXML_H
