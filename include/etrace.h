#ifndef ETRACE_H
#define ETRACE_H

#include <QObject>
#include <QFile>
#include "target.h"
#include "server.h"
#include "type.h"

class Etrace : public QObject
{
    Q_OBJECT
public:
    explicit Etrace(QObject *parent = nullptr);

    quint64 etrace_addr;
    quint64 buffer_addr;
    quint64 buffer_size;
    quint64 wrap;

    void CommandPrint(const char *format, ...);
    void Config(Target* target);
    void Enable(Target* target);
    void Disable(Target* target);
    void Start(Target* target);
    void Stop(Target* target);
    void Dump(Target* target, QString file_path);
    void Clear(Target* target);
    void Info(Target* target, Server* server);

private:
    Type* type;
    QByteArray info_hex;
    bool start_flag;
};

#endif // ETRACE_H
