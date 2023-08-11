#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QFile>
#include <QObject>
#include "target.h"

/*==== Loader ====*/
#define ERASE_CMD           (1)
#define WRITE_CMD           (2)
#define READ_CMD            (3)
#define PROBE_CMD           (4)

class Algorithm : public QObject
{
    Q_OBJECT
public:
    explicit Algorithm(QObject *parent = nullptr);

    typedef struct {
        quint64 spi_base;
        quint64 xip_base;
        quint64 xip_size;
        quint64 block_size;
        QString loader_path;
    } flash_t;
    typedef struct {
        quint64 addr;
        quint64 size;
        bool backup;
        QByteArray mem;
    } workarea_t;

    flash_t flash;
    workarea_t workarea;

    void BackupWorkarea(Target* target);
    void RestoreWorkarea(Target* target);
    void DownloadLoader(Target* target);
    void Execute(Target* target, quint32 cs, quint32 addr, quint32 count, QByteArray buffer);

private:
    quint64 buffer_addr;
};

#endif // ALGORITHM_H
