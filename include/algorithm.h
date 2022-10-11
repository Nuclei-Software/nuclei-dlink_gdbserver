#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QObject>

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
    }flash_t;

    void AddFlash(flash_t flash);

    typedef struct {
        quint64 addr;
        quint64 size;
        bool backup;
        QByteArray mem;
    }workarea_t;

    void AddWorkarea(workarea_t workarea);

private:
    QList<flash_t> flashs;
    QList<workarea_t> workareas;

signals:

};

#endif // ALGORITHM_H
