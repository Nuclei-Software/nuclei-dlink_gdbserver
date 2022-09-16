#include "xml.h"

Xml::Xml(QObject *parent)
    : QObject{parent}
{
}

void Xml::GetInitXml(Misa* misa, quint64 vlenb)
{
    xml.clear();
    xml.append("<?xml version=\"1.0\"?>");
    xml.append("<!DOCTYPE target SYSTEM \"gdb-target.dtd\">");
    xml.append("<target version=\"1.0\">");
    if (1 == misa->mxl) {
        xml.append("<architecture>riscv:rv32</architecture>");
    } else if (2 == misa->mxl) {
        xml.append("<architecture>riscv:rv64</architecture>");
    } else {
    }
    char temp[1024];
    foreach (QByteArray feature, features) {
        if (feature_fpu == feature) {
            if (!(misa->f | misa->d)) {
                continue;
            }
        }
        if (feature_vector == feature) {
            if (!misa->v) {
                continue;
            }
        }
        sprintf(temp, "<feature name=\"%s\">", feature.constData());
        xml.append(temp);
        foreach (reg_t reg, regs) {
            if (feature != reg.feature) {
                continue;
            }
            if (feature_cpu == reg.feature) {
                if (1 == misa->mxl) {
                    reg.bitsize = 32;
                } else if (2 == misa->mxl) {
                    reg.bitsize = 64;
                } else {
                    continue;
                }
            }
            if (feature_fpu == reg.feature) {
                if (misa->d) {
                    reg.bitsize = 64;
                } else if (misa->f) {
                    reg.bitsize = 32;
                } else {
                    continue;
                }
            }
            if (feature_csr == reg.feature) {
                if (1 == misa->mxl) {
                    reg.bitsize = 32;
                } else if (2 == misa->mxl) {
                    reg.bitsize = 64;
                } else {
                    continue;
                }
            }
            if (feature_vector == reg.feature) {
                reg.bitsize = vlenb * 8;
            }
            sprintf(temp, "<reg name=\"%s\" ", reg.name.constData());
            xml.append(temp);
            sprintf(temp, "bitsize=\"%d\" ", reg.bitsize);
            xml.append(temp);
            sprintf(temp, "regnum=\"%d\" ", reg.regnum);
            xml.append(temp);
            if (reg.save_restore) {
                xml.append("save-restore=\"yes\" ");
            } else {
                xml.append("save-restore=\"no\" ");
            }
            sprintf(temp, "type=\"%s\" ", reg.type.constData());
            xml.append(temp);
            sprintf(temp, "group=\"%s\"/>", reg.group.constData());
            xml.append(temp);
        }
        xml.append("</feature>");
    }
    xml.append("</target>");
}

quint32 Xml::GetXmlLen(void)
{
    return xml.size();
}

QByteArray Xml::GetXml(quint32 addr)
{
    return &xml[addr];
}
