#include "../include/cpuinfo.h"

/*==== Nuclei cpuinfo ====*/
#define RV_REG_MARCHID            (65+0xf12)
#define RV_REG_MIMPID             (65+0xf13)
#define RV_REG_MICFG_INFO         (65+0xfc0)
#define RV_REG_MDCFG_INFO         (65+0xfc1)
#define RV_REG_MCFG_INFO          (65+0xfc2)
#define RV_REG_MTLBCFG_INFO       (65+0xfc3)
#define RV_REG_MIRGB_INFO         (65+0x7f7)
#define RV_REG_MPPICFG_INFO       (65+0x7f0)
#define RV_REG_MFIOCFG_INFO       (65+0x7f1)
#define BIT(ofs)                  (0x1UL << (ofs))
#define KB                        (1024)
#define MB                        (KB * 1024)
#define GB                        (MB * 1024)
#define EXTENSION_NUM             (26)
#define POWER_FOR_TWO(n)          (1 << (n))
#define EXTRACT_FIELD(val, which) (((val) & (which)) / ((which) & ~((which)-1)))
#define print_size(bytes) do {\
if (bytes / GB) {\
        CommandPrint(" %ldGB", bytes / GB);\
} else if (bytes / MB) {\
        CommandPrint(" %ldMB", bytes / MB);\
} else if (bytes / KB) {\
        CommandPrint(" %ldKB", bytes / KB);\
} else {\
        CommandPrint(" %ldByte", bytes);\
}\
} while(0);
#define show_cache_info(set, way, lsize) do {\
print_size((set) * (way) * (lsize));\
    CommandPrint("(set=%ld,", set);\
    CommandPrint("way=%ld,", way);\
    CommandPrint("lsize=%ld)\n", lsize);\
} while(0);

Cpuinfo::Cpuinfo(QObject *parent)
    : QObject{parent}
{
    type = new Type;
}

void Cpuinfo::CommandPrint(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    QString data_bin = QString::vasprintf(format, ap);
    va_end(ap);
    cpuinfo_hex.append(type->bin_to_hex(data_bin.toLatin1(), data_bin.toLatin1().size()));
}

void Cpuinfo::ShowInfo(Target* target, Server* server)
{
    QByteArray temp;
    quint64 csr_marchid, csr_mimpid, csr_mcfg, csr_micfg, csr_mdcfg,
        iregion_base, csr_mirgb, csr_mfiocfg, csr_mppicfg, csr_mtlbcfg;
    cpuinfo_hex.clear();
    CommandPrint("----Supported configuration information\n");
    csr_marchid = target->ReadRegister(RV_REG_MARCHID);
    CommandPrint("         MARCHID: %llx\n", csr_marchid);
    csr_mimpid = target->ReadRegister(RV_REG_MIMPID);
    CommandPrint("          MIMPID: %llx\n", csr_mimpid);
    /* ISA */
    CommandPrint("             ISA:");
    if (1 == target->misa->mxl) {
        CommandPrint(" RV32");
    } else {
        CommandPrint(" RV64");
    }
    for (int i = 0; i < EXTENSION_NUM; i++) {
        if (target->misa->value & BIT(i)) {
            if ('X' == ('A' + i)) {
                CommandPrint(" Zc XLCZ");
            } else {
                CommandPrint(" %c", 'A' + i);
            }
        }
    }
    CommandPrint("\n");
    /* Support */
    csr_mcfg = target->ReadRegister(RV_REG_MCFG_INFO);
    CommandPrint("            MCFG:");
    if (csr_mcfg & BIT(0)) {
        CommandPrint(" TEE");
    }
    if (csr_mcfg & BIT(1)) {
        CommandPrint(" ECC");
    }
    if (csr_mcfg & BIT(2)) {
        CommandPrint(" ECLIC");
    }
    if (csr_mcfg & BIT(3)) {
        CommandPrint(" PLIC");
    }
    if (csr_mcfg & BIT(4)) {
        CommandPrint(" FIO");
    }
    if (csr_mcfg & BIT(5)) {
        CommandPrint(" PPI");
    }
    if (csr_mcfg & BIT(6)) {
        CommandPrint(" NICE");
    }
    if (csr_mcfg & BIT(7)) {
        CommandPrint(" ILM");
    }
    if (csr_mcfg & BIT(8)) {
        CommandPrint(" DLM");
    }
    if (csr_mcfg & BIT(9)) {
        CommandPrint(" ICACHE");
    }
    if (csr_mcfg & BIT(10)) {
        CommandPrint(" DCACHE");
    }
    if (csr_mcfg & BIT(11)) {
        CommandPrint(" SMP");
    }
    if (csr_mcfg & BIT(12)) {
        CommandPrint(" DSP-N1");
    }
    if (csr_mcfg & BIT(13)) {
        CommandPrint(" DSP-N2");
    }
    if (csr_mcfg & BIT(14)) {
        CommandPrint(" DSP-N3");
    }
    if (csr_mcfg & BIT(16)) {
        CommandPrint(" IREGION");
    }
    if (csr_mcfg & BIT(20)) {
        CommandPrint(" ETRACE");
    }
    CommandPrint("\n");
    /* ILM */
    if (csr_mcfg & BIT(7)) {
        csr_micfg = target->ReadRegister(RV_REG_MICFG_INFO);
        CommandPrint("             ILM:");
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_micfg, 0x1F << 16) - 1) * 256);
        if (csr_micfg & BIT(21)) {
            CommandPrint(" execute-only");
        }
        if (csr_micfg & BIT(22)) {
            CommandPrint(" has-ecc");
        }
        CommandPrint("\n");
    }
    /* DLM */
    if (csr_mcfg & BIT(8)) {
        csr_mdcfg = target->ReadRegister(RV_REG_MDCFG_INFO);
        CommandPrint("             DLM:");
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mdcfg, 0x1F << 16) - 1) * 256);
        if (csr_mdcfg & BIT(21)) {
            CommandPrint(" has-ecc");
        }
        CommandPrint("\n");
    }
    /* ICACHE */
    if (csr_mcfg & BIT(9)) {
        csr_micfg = target->ReadRegister(RV_REG_MICFG_INFO);
        CommandPrint("          ICACHE:");
        show_cache_info(POWER_FOR_TWO(EXTRACT_FIELD(csr_micfg, 0xF) +3),
                        EXTRACT_FIELD(csr_micfg, 0x7 << 4) + 1,
                        POWER_FOR_TWO(EXTRACT_FIELD(csr_micfg, 0x7 << 7) + 2));
    }
    /* DCACHE */
    if (csr_mcfg & BIT(10)) {
        csr_mdcfg = target->ReadRegister(RV_REG_MDCFG_INFO);
        CommandPrint("          DCACHE:");
        show_cache_info(POWER_FOR_TWO(EXTRACT_FIELD(csr_mdcfg, 0xF) +3),
                        EXTRACT_FIELD(csr_mdcfg, 0x7 << 4) + 1,
                        POWER_FOR_TWO(EXTRACT_FIELD(csr_mdcfg, 0x7 << 7) + 2));
    }
    /* IREGION */
    if (csr_mcfg & BIT(16)) {
        csr_mirgb = target->ReadRegister(RV_REG_MIRGB_INFO);
        CommandPrint("         IREGION:");
        iregion_base = csr_mirgb & (~0x3FF);
        CommandPrint(" %#lx", iregion_base);
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mirgb, 0xF << 1) - 1) * KB);
        CommandPrint("\n");
        CommandPrint("                  Unit        Size        Address\n");
        CommandPrint("                  INFO        64KB        %#lx\n", iregion_base);
        CommandPrint("                  DEBUG       64KB        %#lx\n", iregion_base + 0x10000);
        if (csr_mcfg & BIT(2)) {
            CommandPrint("                  ECLIC       64KB        %#lx\n", iregion_base + 0x20000);
        }
        CommandPrint("                  TIMER       64KB        %#lx\n", iregion_base + 0x30000);
        if (csr_mcfg & BIT(11)) {
            CommandPrint("                  SMP&CC      64KB        %#lx\n", iregion_base + 0x40000);
        }
        uint64_t smp_cfg = target->ReadMemory(iregion_base + 0x40004, 4).toLongLong(NULL, 16);
        if ((csr_mcfg & BIT(2)) && (EXTRACT_FIELD(smp_cfg, 0x1F << 1) >= 2)) {
            CommandPrint("                  CIDU        64KB        %#lx\n", iregion_base + 0x50000);
        }
        if (csr_mcfg & BIT(3)) {
            CommandPrint("                  PLIC        64MB        %#lx\n", iregion_base + 0x4000000);
        }
        /* SMP */
        if (csr_mcfg & BIT(11)) {
            CommandPrint("         SMP_CFG:");
            CommandPrint(" CC_PRESENT=%ld", EXTRACT_FIELD(smp_cfg, 0x1));
            CommandPrint(" SMP_CORE_NUM=%ld", EXTRACT_FIELD(smp_cfg, 0x1F << 1));
            CommandPrint(" IOCP_NUM=%ld", EXTRACT_FIELD(smp_cfg, 0x3F << 7));
            CommandPrint(" PMON_NUM=%ld\n", EXTRACT_FIELD(smp_cfg, 0x3F << 13));
        }
        /* L2CACHE */
        if (smp_cfg & 0x1) {
            CommandPrint("         L2CACHE:");
            uint64_t cc_cfg = target->ReadMemory(iregion_base + 0x40008, 4).toLongLong(NULL, 16);
            show_cache_info(POWER_FOR_TWO(EXTRACT_FIELD(cc_cfg, 0xF)), EXTRACT_FIELD(cc_cfg, 0x7 << 4) + 1,
                            POWER_FOR_TWO(EXTRACT_FIELD(cc_cfg, 0x7 << 7) + 2));
        }
        /* INFO */
        CommandPrint("     INFO-Detail:\n");
        uint64_t mpasize = target->ReadMemory(iregion_base, 4).toLongLong(NULL, 16);
        CommandPrint("                  mpasize : %ld\n", mpasize);
        uint64_t cmo_info = target->ReadMemory(iregion_base + 4, 4).toLongLong(NULL, 16);
        if (cmo_info & 0x1) {
            CommandPrint("                  cbozero : %ldByte\n", POWER_FOR_TWO(EXTRACT_FIELD(cmo_info, 0xF << 6) + 2));
            CommandPrint("                  cmo     : %ldByte\n", POWER_FOR_TWO(EXTRACT_FIELD(cmo_info, 0xF << 2) + 2));
            if (cmo_info & 0x2) {
                CommandPrint("                  has_prefecth\n");
            }
        }
        uint64_t mcppi_cfg_lo = target->ReadMemory(iregion_base + 80, 4).toLongLong(NULL, 16);
        uint64_t mcppi_cfg_hi = target->ReadMemory(iregion_base + 84, 4).toLongLong(NULL, 16);
        if (mcppi_cfg_lo & 0x1) {
            if (1 == target->misa->mxl) {
                CommandPrint("                  cppi    : %#lx", mcppi_cfg_lo & (~0x3FF));
            } else {
                CommandPrint("                  cppi    : %#lx", (mcppi_cfg_hi << 32) | (mcppi_cfg_lo & (~0x3FF)));
            }
            print_size(POWER_FOR_TWO(EXTRACT_FIELD(mcppi_cfg_lo, 0xF << 1) - 1) * KB);
            CommandPrint("\n");
        }
    }
    /* TLB */
    if (csr_mcfg & BIT(3)) {
        csr_mtlbcfg = target->ReadRegister(RV_REG_MTLBCFG_INFO);
        if (csr_mtlbcfg) {
            CommandPrint("            DTLB: %ld entry\n", EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 19));
            CommandPrint("            ITLB: %ld entry\n", EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 16));
            CommandPrint("            MTLB:");
            CommandPrint(" %ld entry", POWER_FOR_TWO(EXTRACT_FIELD(csr_mtlbcfg, 0xF) + 3) *
                                            (EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 4) + 1) *
                                            (EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 7) - 1));
            if (csr_mtlbcfg & BIT(10)) {
                CommandPrint(" has_ecc");
            }
            CommandPrint("\n");
        }
    }
    /* FIO */
    if (csr_mcfg & BIT(4)) {
        csr_mfiocfg = target->ReadRegister(RV_REG_MFIOCFG_INFO);
        CommandPrint("             FIO:");
        CommandPrint(" %#lx", csr_mfiocfg & (~0x3FF));
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mfiocfg, 0xF << 1) - 1) * KB);
        CommandPrint("\n");
    }
    /* PPI */
    if (csr_mcfg & BIT(5)) {
        csr_mppicfg = target->ReadRegister(RV_REG_MPPICFG_INFO);
        CommandPrint("             PPI:");
        CommandPrint(" %#lx", csr_mppicfg & (~0x3FF));
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mppicfg, 0xF << 1) - 1) * KB);
        CommandPrint("\n");
    }
    CommandPrint("----End of cpuinfo\n");
    server->Write(cpuinfo_hex);
}
