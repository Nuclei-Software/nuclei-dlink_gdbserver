#ifndef CPUINFO_H
#define CPUINFO_H

#include <QObject>
#include "type.h"
#include "target.h"
#include "server.h"

class Cpuinfo : public QObject
{
    Q_OBJECT
public:
    explicit Cpuinfo(QObject *parent = nullptr);

    void CommandPrint(const char *format, ...);
    void ShowInfo(Target* target, Server* server);

private:
    Type* type;
    QByteArray cpuinfo_hex;
};

#endif // CPUINFO_H
