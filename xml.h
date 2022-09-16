#ifndef XML_H
#define XML_H

#include <QObject>
#include "misa.h"

class Xml : public QObject
{
    Q_OBJECT
public:
    explicit Xml(QObject *parent = nullptr);
    void GetInitXml(Misa* misa, quint64 vlenb);
    quint32 GetXmlLen(void);
    QByteArray GetXml(quint32 addr);

private:
    QByteArray xml;

    QByteArray feature_cpu = {"org.gnu.gdb.riscv.cpu"};
    QByteArray feature_fpu = {"org.gnu.gdb.riscv.fpu"};
    QByteArray feature_csr = {"org.gnu.gdb.riscv.csr"};
    QByteArray feature_vector = {"org.gnu.gdb.riscv.vector"};
    QByteArray feature_virtual = {"org.gnu.gdb.riscv.virtual"};
    QByteArray feature_custom = {"org.gnu.gdb.riscv.custom"};

    typedef struct {
        QByteArray name;
        quint32 bitsize;
        quint32 regnum;
        bool save_restore;
        QByteArray type;
        QByteArray group;
        QByteArray feature;
    }reg_t;

    QList<QByteArray> features = {
        feature_cpu,
        feature_fpu,
        feature_csr,
        feature_vector,
        feature_virtual,
        feature_custom,
    };

    QList<reg_t> regs = {
        /* name    bitsize    regnum    save-restore    type      group         feature */
        {"zero",   32,        0,        true,           "int",    "general",    feature_cpu},
        {"ra",     32,        1,        true,           "int",    "general",    feature_cpu},
        {"sp",     32,        2,        true,           "int",    "general",    feature_cpu},
        {"gp",     32,        3,        true,           "int",    "general",    feature_cpu},
        {"tp",     32,        4,        true,           "int",    "general",    feature_cpu},
        {"t0",     32,        5,        true,           "int",    "general",    feature_cpu},
        {"t1",     32,        6,        true,           "int",    "general",    feature_cpu},
        {"t2",     32,        7,        true,           "int",    "general",    feature_cpu},
        {"fp",     32,        8,        true,           "int",    "general",    feature_cpu},
        {"s1",     32,        9,        true,           "int",    "general",    feature_cpu},
        {"a0",     32,        10,       true,           "int",    "general",    feature_cpu},
        {"a1",     32,        11,       true,           "int",    "general",    feature_cpu},
        {"a2",     32,        12,       true,           "int",    "general",    feature_cpu},
        {"a3",     32,        13,       true,           "int",    "general",    feature_cpu},
        {"a4",     32,        14,       true,           "int",    "general",    feature_cpu},
        {"a5",     32,        15,       true,           "int",    "general",    feature_cpu},
        {"a6",     32,        16,       true,           "int",    "general",    feature_cpu},
        {"a7",     32,        17,       true,           "int",    "general",    feature_cpu},
        {"s2",     32,        18,       true,           "int",    "general",    feature_cpu},
        {"s3",     32,        19,       true,           "int",    "general",    feature_cpu},
        {"s4",     32,        20,       true,           "int",    "general",    feature_cpu},
        {"s5",     32,        21,       true,           "int",    "general",    feature_cpu},
        {"s6",     32,        22,       true,           "int",    "general",    feature_cpu},
        {"s7",     32,        23,       true,           "int",    "general",    feature_cpu},
        {"s8",     32,        24,       true,           "int",    "general",    feature_cpu},
        {"s9",     32,        25,       true,           "int",    "general",    feature_cpu},
        {"s10",    32,        26,       true,           "int",    "general",    feature_cpu},
        {"s11",    32,        27,       true,           "int",    "general",    feature_cpu},
        {"t3",     32,        28,       true,           "int",    "general",    feature_cpu},
        {"t4",     32,        29,       true,           "int",    "general",    feature_cpu},
        {"t5",     32,        30,       true,           "int",    "general",    feature_cpu},
        {"t6",     32,        31,       true,           "int",    "general",    feature_cpu},
        {"pc",     32,        32,       true,           "int",    "general",    feature_cpu},
        /* name    bitsize    regnum    save-restore    type      group         feature */
        {"ft0",    32,        33,       true,           "int",    "float",      feature_fpu},
        {"ft1",    32,        34,       true,           "int",    "float",      feature_fpu},
        {"ft2",    32,        35,       true,           "int",    "float",      feature_fpu},
        {"ft3",    32,        36,       true,           "int",    "float",      feature_fpu},
        {"ft4",    32,        37,       true,           "int",    "float",      feature_fpu},
        {"ft5",    32,        38,       true,           "int",    "float",      feature_fpu},
        {"ft6",    32,        39,       true,           "int",    "float",      feature_fpu},
        {"ft7",    32,        40,       true,           "int",    "float",      feature_fpu},
        {"fs0",    32,        41,       true,           "int",    "float",      feature_fpu},
        {"fs1",    32,        42,       true,           "int",    "float",      feature_fpu},
        {"fa0",    32,        43,       true,           "int",    "float",      feature_fpu},
        {"fa1",    32,        44,       true,           "int",    "float",      feature_fpu},
        {"fa2",    32,        45,       true,           "int",    "float",      feature_fpu},
        {"fa3",    32,        46,       true,           "int",    "float",      feature_fpu},
        {"fa4",    32,        47,       true,           "int",    "float",      feature_fpu},
        {"fa5",    32,        48,       true,           "int",    "float",      feature_fpu},
        {"fa6",    32,        49,       true,           "int",    "float",      feature_fpu},
        {"fa7",    32,        50,       true,           "int",    "float",      feature_fpu},
        {"fs2",    32,        51,       true,           "int",    "float",      feature_fpu},
        {"fs3",    32,        52,       true,           "int",    "float",      feature_fpu},
        {"fs4",    32,        53,       true,           "int",    "float",      feature_fpu},
        {"fs5",    32,        54,       true,           "int",    "float",      feature_fpu},
        {"fs6",    32,        55,       true,           "int",    "float",      feature_fpu},
        {"fs7",    32,        56,       true,           "int",    "float",      feature_fpu},
        {"fs8",    32,        57,       true,           "int",    "float",      feature_fpu},
        {"fs9",    32,        58,       true,           "int",    "float",      feature_fpu},
        {"fs10",   32,        59,       true,           "int",    "float",      feature_fpu},
        {"fs11",   32,        60,       true,           "int",    "float",      feature_fpu},
        {"ft8",    32,        61,       true,           "int",    "float",      feature_fpu},
        {"ft9",    32,        62,       true,           "int",    "float",      feature_fpu},
        {"ft10",   32,        63,       true,           "int",    "float",      feature_fpu},
        {"ft11",   32,        64,       true,           "int",    "float",      feature_fpu},
        /* name    bitsize    regnum    save-restore    type      group         feature */
        {"vstart", 32,        73,       false,          "int",    "csr",        feature_csr},
        {"vxsat",  32,        74,       false,          "int",    "csr",        feature_csr},
        {"vxrm",   32,        75,       false,          "int",    "csr",        feature_csr},
        {"vcsr",   32,        81,       false,          "int",    "csr",        feature_csr},
        {"vl",     32,        3169,     false,          "int",    "csr",        feature_csr},
        {"vtype",  32,        3170,     false,          "int",    "csr",        feature_csr},
        {"vlenb",  32,        3171,     false,          "int",    "csr",        feature_csr},
        /* name    bitsize    regnum    save-restore    type      group         feature */
        {"v0",     32,        4162,     false,          "int",    "vector",     feature_vector},
        {"v1",     32,        4163,     false,          "int",    "vector",     feature_vector},
        {"v2",     32,        4164,     false,          "int",    "vector",     feature_vector},
        {"v3",     32,        4165,     false,          "int",    "vector",     feature_vector},
        {"v4",     32,        4166,     false,          "int",    "vector",     feature_vector},
        {"v5",     32,        4167,     false,          "int",    "vector",     feature_vector},
        {"v6",     32,        4168,     false,          "int",    "vector",     feature_vector},
        {"v7",     32,        4169,     false,          "int",    "vector",     feature_vector},
        {"v8",     32,        4170,     false,          "int",    "vector",     feature_vector},
        {"v9",     32,        4171,     false,          "int",    "vector",     feature_vector},
        {"v10",    32,        4172,     false,          "int",    "vector",     feature_vector},
        {"v11",    32,        4173,     false,          "int",    "vector",     feature_vector},
        {"v12",    32,        4174,     false,          "int",    "vector",     feature_vector},
        {"v13",    32,        4175,     false,          "int",    "vector",     feature_vector},
        {"v14",    32,        4176,     false,          "int",    "vector",     feature_vector},
        {"v15",    32,        4177,     false,          "int",    "vector",     feature_vector},
        {"v16",    32,        4178,     false,          "int",    "vector",     feature_vector},
        {"v17",    32,        4179,     false,          "int",    "vector",     feature_vector},
        {"v18",    32,        4181,     false,          "int",    "vector",     feature_vector},
        {"v19",    32,        4182,     false,          "int",    "vector",     feature_vector},
        {"v20",    32,        4182,     false,          "int",    "vector",     feature_vector},
        {"v21",    32,        4183,     false,          "int",    "vector",     feature_vector},
        {"v22",    32,        4184,     false,          "int",    "vector",     feature_vector},
        {"v23",    32,        4185,     false,          "int",    "vector",     feature_vector},
        {"v24",    32,        4186,     false,          "int",    "vector",     feature_vector},
        {"v25",    32,        4187,     false,          "int",    "vector",     feature_vector},
        {"v26",    32,        4188,     false,          "int",    "vector",     feature_vector},
        {"v27",    32,        4189,     false,          "int",    "vector",     feature_vector},
        {"v28",    32,        4190,     false,          "int",    "vector",     feature_vector},
        {"v29",    32,        4191,     false,          "int",    "vector",     feature_vector},
        {"v30",    32,        4192,     false,          "int",    "vector",     feature_vector},
        {"v31",    32,        4193,     false,          "int",    "vector",     feature_vector},
    };

signals:

};

#endif // XML_H
