#include "regxml.h"

RegXml::RegXml(QObject *parent)
    : QObject{parent}
{
}

void RegXml::InitRegXml(Misa* misa, quint64 vlenb)
{
    regxml.clear();
    regxml.append("<?xml version=\"1.0\"?>");
    regxml.append("<!DOCTYPE target SYSTEM \"gdb-target.dtd\">");
    regxml.append("<target version=\"1.0\">");
    if (1 == misa->mxl) {
        regxml.append("<architecture>riscv:rv32</architecture>");
    } else if (2 == misa->mxl) {
        regxml.append("<architecture>riscv:rv64</architecture>");
    } else {
    }
    char temp[1024];
    foreach (QByteArray feature, features) {
        /* feature_fpu need core support f/d */
        if (feature_fpu == feature) {
            if (!(misa->f | misa->d)) {
                continue;
            }
        }
        /* feature_vector need core support v */
        if (feature_vector == feature) {
            if (!misa->v) {
                continue;
            }
        }
        sprintf(temp, "<feature name=\"%s\">", feature.constData());
        regxml.append(temp);

        if (feature_vector == feature) {
            regxml.append("<vector id=\"bytes\" type=\"uint8\" count=\"16\"/>");
            regxml.append("<vector id=\"shorts\" type=\"uint16\" count=\"8\"/>");
            regxml.append("<vector id=\"words\" type=\"uint32\" count=\"4\"/>");
            regxml.append("<vector id=\"longs\" type=\"uint64\" count=\"2\"/>");
            regxml.append("<vector id=\"quads\" type=\"uint128\" count=\"1\"/>");
            regxml.append("<union id=\"riscv_vector\">");
            regxml.append("<field name=\"b\" type=\"bytes\"/>");
            regxml.append("<field name=\"s\" type=\"shorts\"/>");
            regxml.append("<field name=\"w\" type=\"words\"/>");
            regxml.append("<field name=\"l\" type=\"longs\"/>");
            regxml.append("<field name=\"q\" type=\"quads\"/>");
            regxml.append("</union>");
        }

        foreach (reg_t reg, regs) {
            /* ingnore other feature */
            if (feature != reg.feature) {
                continue;
            }
            /* feature_cpu register width depend on mxl */
            if (feature_cpu == reg.feature) {
                if (1 == misa->mxl) {
                    reg.bitsize = 32;
                } else if (2 == misa->mxl) {
                    reg.bitsize = 64;
                } else {
                    continue;
                }
            }
            /* feature_fpu register width depend on d/f */
            if (feature_fpu == reg.feature) {
                if (misa->d) {
                    reg.bitsize = 64;
                } else if (misa->f) {
                    reg.bitsize = 32;
                } else {
                    continue;
                }
            }
            /* feature_csr register width depend on mxl */
            if (feature_csr == reg.feature) {
                if (1 == misa->mxl) {
                    reg.bitsize = 32;
                } else if (2 == misa->mxl) {
                    reg.bitsize = 64;
                } else {
                    continue;
                }
            }
            /* feature_csr register width depend on vlenb */
            if (feature_vector == reg.feature) {
                reg.bitsize = vlenb * 8;
                reg.type = "riscv_vector";
            }
            sprintf(temp, "<reg name=\"%s\" ", reg.name.constData());
            regxml.append(temp);
            sprintf(temp, "bitsize=\"%d\" ", reg.bitsize);
            regxml.append(temp);
            sprintf(temp, "regnum=\"%d\" ", reg.regnum);
            regxml.append(temp);
            if (reg.save_restore) {
                regxml.append("save-restore=\"yes\" ");
            } else {
                regxml.append("save-restore=\"no\" ");
            }
            sprintf(temp, "type=\"%s\" ", reg.type.constData());
            regxml.append(temp);
            sprintf(temp, "group=\"%s\"/>", reg.group.constData());
            regxml.append(temp);
        }
        regxml.append("</feature>");
    }
    regxml.append("</target>");
}

quint32 RegXml::GetRegXmlLen(void)
{
    return regxml.size();
}

QByteArray RegXml::GetRegXml(quint32 addr)
{
    return &regxml[addr];
}
