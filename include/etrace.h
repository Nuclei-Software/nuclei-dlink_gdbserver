#ifndef ETRACE_H
#define ETRACE_H

#include <QObject>
#include <QFile>
#include "target.h"
#include "type.h"

class Etrace : public QObject
{
    Q_OBJECT
public:
    explicit Etrace(QObject *parent = nullptr);

    quint64 etrace_addr;
    quint64 buffer_addr;
    quint64 buffer_size;
    quint64 timeout;
    quint64 wrap;

    void Config(Target* target);
    void Enable(Target* target);
    void Disable(Target* target);
    void Start(Target* target);
    void Stop(Target* target);
    void Dump(Target* target, QString file_path);

private:
    Type* type;
};

#endif // ETRACE_H
