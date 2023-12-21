#include "../include/type.h"

extern bool noack_mode;

Type::Type(QObject *parent)
    : QObject{parent}
{}

QByteArray Type::pack(QByteArray msg)
{
    QByteArray temp;
    quint8 checksum = 0;
    char checksum_c[3];
    if (noack_mode) {
        temp.append('$');
    } else {
        temp.append('+');
        temp.append('$');
    }
    temp.append(msg);
    temp.append('#');
    foreach (char var, msg) {
        checksum += var;
    }
    checksum &= 0xff;
    sprintf(checksum_c, "%02x", checksum);
    temp.append(checksum_c);
    return temp;
}

QByteArray Type::unpack(QByteArray msg)
{
    QByteArray temp;
    if (msg.contains('$') && msg.contains('#')) {
        temp = msg.mid(1, msg.indexOf('#') - 1);
    } else {
        temp = msg;
    }
    return temp;
}

QByteArray Type::bin_encode(QByteArray bin, quint32 bin_len)
{
    QByteArray xbin;
    quint32 i;

    for(i = 0; i < bin_len; i++) {
        if ((bin[i] == '#') || (bin[i] == '$') || (bin[i] == '}') || (bin[i] == '*')) {
            xbin.append(0x7d);
            xbin.append(bin[i] ^ 0x20);
        } else {
            xbin.append(bin[i]);
        }
    }
    return xbin;
}

QByteArray Type::bin_decode(QByteArray xbin)
{
    QByteArray bin;
    quint32 i;
    bool escape_found = false;

    for(i = 0; i < xbin.size(); i++) {
        if (xbin[i] == 0x7d) {
            escape_found = true;
        } else {
            if (escape_found) {
                bin.append(xbin[i] ^ 0x20);
                escape_found = false;
            } else {
                bin.append(xbin[i]);
            }
        }
    }
    return bin;
}

QByteArray Type::bin_to_hex(QByteArray data_bin, uint32_t nbyte)
{
    uint32_t i;
    uint8_t hi;
    uint8_t lo;
    QByteArray hex;
    char *bin = data_bin.data();

    for(i = 0; i < nbyte; i++) {
        hi = (*bin >> 4) & 0xf;
        lo = *bin & 0xf;
        if (hi < 10) {
            hex.append('0' + hi);
        } else {
            hex.append('a' + hi - 10);
        }
        if (lo < 10) {
            hex.append('0' + lo);
        } else {
            hex.append('a' + lo - 10);
        }
        bin++;
    }
    return hex;
}

QByteArray Type::uint32_to_bin_le(quint32 data)
{
    QByteArray bin;

    bin.append(data & 0xff);
    bin.append((data >> 8) & 0xff);
    bin.append((data >> 16) & 0xff);
    bin.append((data >> 24) & 0xff);

    return bin;
}

QByteArray Type::uint64_to_bin_le(quint64 data)
{
    QByteArray bin;

    bin.append(data & 0xff);
    bin.append((data >> 8) & 0xff);
    bin.append((data >> 16) & 0xff);
    bin.append((data >> 24) & 0xff);
    bin.append((data >> 32) & 0xff);
    bin.append((data >> 40) & 0xff);
    bin.append((data >> 48) & 0xff);
    bin.append((data >> 56) & 0xff);

    return bin;
}

QByteArray Type::hex_to_bin(QByteArray data_hex, uint32_t nbyte)
{
    uint32_t i;
    uint8_t hi, lo;
    char *hex = data_hex.data();
    char bin;
    QByteArray data_bin;
    for(i = 0; i < (nbyte / 2); i++) {
        if (hex[i * 2] <= '9') {
            hi = hex[i * 2] - '0';
        } else if (hex[i * 2] <= 'F') {
            hi = hex[i * 2] - 'A' + 10;
        } else {
            hi = hex[i * 2] - 'a' + 10;
        }
        if (hex[i * 2 + 1] <= '9') {
            lo = hex[i * 2 + 1] - '0';
        } else if (hex[i * 2 + 1] <= 'F') {
            lo = hex[i * 2 + 1] - 'A' + 10;
        } else {
            lo = hex[i * 2 + 1] - 'a' + 10;
        }
        bin = (hi << 4) | lo;
        data_bin.append(bin);
    }
    return data_bin;
}

quint32 Type::bin_to_uint32_le(QByteArray bin)
{
    quint32 data;

    data  = 0xFF & (quint32)bin[0];
    data |= 0xFF00 & ((quint32)bin[1] << 8);
    data |= 0xFF0000 & ((quint32)bin[2] << 16);
    data |= 0xFF000000 & ((quint32)bin[3] << 24);
    return data;
}

quint64 Type::bin_to_uint64_le(QByteArray bin)
{
    quint64 data;

    data  = 0xFF & (quint64)bin[0];
    data |= 0xFF00 & ((quint64)bin[1] << 8);
    data |= 0xFF0000 & ((quint64)bin[2] << 16);
    data |= 0xFF000000 & ((quint64)bin[3] << 24);
    data |= 0xFF00000000 & ((quint64)bin[4] << 32);
    data |= 0xFF0000000000 & ((quint64)bin[5] << 40);
    data |= 0xFF000000000000 & ((quint64)bin[6] << 48);
    data |= 0xFF00000000000000 & ((quint64)bin[7] << 56);
    return data;
}
