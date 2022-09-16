#include "misa.h"

Misa::Misa(QObject *parent)
    : QObject{parent}
{
}

void Misa::MisaInit(quint32 misa)
{
    a   = (misa & (0x1 << 0 )) >> 0 ;
    b   = (misa & (0x1 << 1 )) >> 1 ;
    c   = (misa & (0x1 << 2 )) >> 2 ;
    d   = (misa & (0x1 << 3 )) >> 3 ;
    e   = (misa & (0x1 << 4 )) >> 4 ;
    f   = (misa & (0x1 << 5 )) >> 5 ;
    g   = (misa & (0x1 << 6 )) >> 6 ;
    h   = (misa & (0x1 << 7 )) >> 7 ;
    i   = (misa & (0x1 << 8 )) >> 8 ;
    j   = (misa & (0x1 << 9 )) >> 9 ;
    l   = (misa & (0x1 << 11)) >> 11;
    m   = (misa & (0x1 << 12)) >> 12;
    n   = (misa & (0x1 << 13)) >> 13;
    p   = (misa & (0x1 << 15)) >> 15;
    q   = (misa & (0x1 << 16)) >> 16;
    s   = (misa & (0x1 << 18)) >> 18;
    t   = (misa & (0x1 << 19)) >> 19;
    u   = (misa & (0x1 << 20)) >> 20;
    v   = (misa & (0x1 << 21)) >> 21;
    x   = (misa & (0x1 << 23)) >> 23;
    mxl = (misa & (0x3 << 30)) >> 30;
}
