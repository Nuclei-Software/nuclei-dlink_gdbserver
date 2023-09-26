#ifndef TYPE_H
#define TYPE_H

#include <QObject>

class Type : public QObject
{
    Q_OBJECT
public:
    explicit Type(QObject *parent = nullptr);

    QByteArray pack(QByteArray msg);
    QByteArray unpack(QByteArray msg);
    QByteArray bin_encode(QByteArray bin, quint32 bin_len);
    QByteArray bin_decode(QByteArray xbin);
    QByteArray bin_to_hex(QByteArray data_bin, uint32_t nbyte);
    QByteArray uint32_to_hex_le(quint32 data);
    QByteArray uint64_to_hex_le(quint64 data);
    QByteArray hex_to_bin(QByteArray data_hex, uint32_t nbyte);
    quint32 hex_to_uint32_le(QByteArray hex);
    quint64 hex_to_uint64_le(QByteArray hex);
};

#endif // TYPE_H