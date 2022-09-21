#include "memxml.h"

MemXml::MemXml(QObject *parent)
    : QObject{parent}
{

}

void MemXml::InitMemXml(Misa* misa)
{
    if (1 == misa->mxl) {
        quint32 zero = 0;
        max_addr = ~zero;
    } else if (2 == misa->mxl) {
        quint64 zero = 0;
        max_addr = ~zero;
    }

    char temp[1024];
    quint64 mem_addr = 0;
    memxml.clear();
    memxml.append("<memory-map>");
    foreach (flash_t flash, flashs) {
        if (mem_addr < flash.start) {
            sprintf(temp, "<memory type=\"%s\" start=\"0x%llx\" length=\"0x%llx\"/>",
                    "ram",
                    mem_addr,
                    flash.start);
            memxml.append(temp);
            mem_addr += flash.start;
        }
        sprintf(temp, "<memory type=\"%s\" start=\"0x%llx\" length=\"0x%llx\"><property name=\"blocksize\">0x%llx</property></memory>",
                "flash",
                flash.start,
                flash.length,
                flash.blocksize);
        memxml.append(temp);
        mem_addr += flash.length;
    }
    sprintf(temp, "<memory type=\"%s\" start=\"0x%llx\" length=\"0x%llx\"/>",
            "ram",
            mem_addr,
            max_addr - mem_addr + 1);
    memxml.append(temp);
    memxml.append("</memory-map>");
}

quint32 MemXml::GetMemXmlLen(void)
{
    return memxml.size();
}

QByteArray MemXml::GetMemXml(quint32 addr)
{
    return &memxml[addr];
}
