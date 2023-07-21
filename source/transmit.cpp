#include "../include/transmit.h"

QString version = "V1.0.0";

extern QQueue<QByteArray> server_cmd_queue;
extern QQueue<QByteArray> target_rsp_queue;

bool debug = false;

/*==== Loader ====*/
#define ERASE_CMD           (1)
#define WRITE_CMD           (2)
#define READ_CMD            (3)
#define PROBE_CMD           (4)

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
        command_print(" %ldGB", bytes / GB);\
    } else if (bytes / MB) {\
        command_print(" %ldMB", bytes / MB);\
    } else if (bytes / KB) {\
        command_print(" %ldKB", bytes / KB);\
    } else {\
        command_print(" %ldByte", bytes);\
    }\
} while(0);
#define show_cache_info(set, way, lsize) do {\
    print_size((set) * (way) * (lsize));\
    command_print("(set=%ld,", set);\
    command_print("way=%ld,", way);\
    command_print("lsize=%ld)\n", lsize);\
} while(0);

static QByteArray bin_encode(QByteArray bin, quint32 bin_len)
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

static QByteArray bin_decode(QByteArray xbin)
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

static QByteArray bin_to_hex(QByteArray data_bin, uint32_t nbyte)
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

static QByteArray hex_to_bin(QByteArray data_hex, uint32_t nbyte)
{
    uint32_t i;
    uint8_t hi, lo;
    char *hex = data_hex.data();
    char bin;
    QByteArray data_bin;
    for(i = 0; i < nbyte; i++) {
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

Transmit::Transmit()
{
    misa = new Misa;
    regxml = new RegXml;
    workarea.addr = 0;
}

void Transmit::TransmitInit()
{
    memxml = new MemXml;
    algorithm = new Algorithm;

    memxml->AddFlash(flash);
    algorithm->AddFlash(flash);
    algorithm->AddWorkarea(workarea);

    noack_mode = false;
    server_reply_flag = true;
    packet_size = 0x200;

    server_cmd_queue.clear();
    target_rsp_queue.clear();

    qDebug() << "Nuclei Dlink GDB Server " << version << "Command Line Version";
    fprintf(stderr, "Started by GNU MCU Eclipse\n");
}

void Transmit::TransmitDeinit()
{
    delete memxml;
    delete algorithm;
    workarea.addr = 0;
}

QByteArray Transmit::TransmitPackage(QByteArray msg)
{
    QByteArray send;
    quint8 checksum = 0;
    char checksum_c[3];
    if (noack_mode) {
        send.append('$');
    } else {
        send.append('+');
        send.append('$');
    }
    send.append(msg);
    send.append('#');
    foreach (char var, msg) {
        checksum += var;
    }
    checksum &= 0xff;
    sprintf(checksum_c, "%02x", checksum);
    send.append(checksum_c);
    return send;
}

bool Transmit::WaitForTargetRsp()
{
    while (target_rsp_queue.empty())
    {
        if (close_flag) {
            return false;
        }
    }
    return true;
}

QByteArray Transmit::ReadTargetMemory(quint32 memory_addr, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send, read, bin;
    bool is_x_command = 0;
    char temp[1024];
    do {
        if (length < data_size) {
            data_size = length;
        }
        if (is_x_command) {
            sprintf(temp, "x%x,%x", data_addr + memory_addr, data_size);
        } else {
            sprintf(temp, "m%x,%x", data_addr + memory_addr, data_size);
        }
        send.append(temp);
        TransmitTargetCmd(send);
        send.clear();
        if (WaitForTargetRsp()) {
            if (is_x_command) {
                read = bin_decode(TransmitTargetRsp(target_rsp_queue.dequeue()));
                bin.append(read);
            } else {
                read = TransmitTargetRsp(target_rsp_queue.dequeue());
                bin.append(hex_to_bin(read, data_size));
            }
        } else {
            return NULL;
        }
        data_addr += data_size;
        length -= data_size;
    } while (length);
    return bin;
}

void Transmit::WriteTargetMemory(quint32 memory_addr, QByteArray data, quint32 length)
{
    quint32 data_size = packet_size;
    quint32 data_addr = 0;
    QByteArray send;
    bool is_x_command = 0;
    char temp[1024];
    do {
        if (length < data_size) {
            data_size = length;
        }
        if (is_x_command) {
            sprintf(temp, "X%x,%x:", data_addr + memory_addr, data_size);
            send.append(temp);
            send.append(bin_encode(data.mid(data_addr), data_size));
        } else {
            sprintf(temp, "M%x,%x:", data_addr + memory_addr, data_size);
            send.append(temp);
            send.append(bin_to_hex(data.mid(data_addr), data_size));
        }
        data_addr += data_size;
        length -= data_size;
        TransmitTargetCmd(send);
        send.clear();
        if (WaitForTargetRsp()) {
            target_rsp_queue.dequeue();
        }
    } while(length);
}

quint64 Transmit::ReadTargetRegister(quint32 register_number)
{
    char temp[1024];
    QByteArray send, read;
    quint64 value = 0;

    sprintf(temp, "p%x", register_number);
    send.append(temp);
    TransmitTargetCmd(send);
    if (WaitForTargetRsp()) {
        read = TransmitTargetRsp(target_rsp_queue.dequeue());
        if (read.size() <= 8) {
            value = read.mid(0, 2).toLongLong(NULL, 16);
            value |= read.mid(2, 2).toLongLong(NULL, 16) << 8;
            value |= read.mid(4, 2).toLongLong(NULL, 16) << 16;
            value |= read.mid(6, 2).toLongLong(NULL, 16) << 24;
        } else {
            value = read.mid(0, 2).toLongLong(NULL, 16);
            value |= read.mid(2, 2).toLongLong(NULL, 16) << 8;
            value |= read.mid(4, 2).toLongLong(NULL, 16) << 16;
            value |= read.mid(6, 2).toLongLong(NULL, 16) << 24;
            value |= read.mid(8, 2).toLongLong(NULL, 16) << 32;
            value |= read.mid(10, 2).toLongLong(NULL, 16) << 40;
            value |= read.mid(12, 2).toLongLong(NULL, 16) << 48;
            value |= read.mid(14, 2).toLongLong(NULL, 16) << 56;
        }
    }
    return value;
}

void Transmit::WriteTargetRegister(quint32 register_number, quint64 value)
{
    char temp[1024];
    QByteArray send;

    sprintf(temp, "P%x=%llx", register_number, value);
    send.append(temp);
    TransmitTargetCmd(send);
    if (WaitForTargetRsp()) {
        target_rsp_queue.dequeue();
    }
}

void Transmit::ExecuteAlgorithm(quint32 cs, quint32 addr, quint32 count, QByteArray buffer)
{
    QByteArray send;
    quint32 params1, params2, params3;
    char temp[1024];

    if (WRITE_CMD == cs) {
        //download write data
        WriteTargetMemory(buffer_addr, buffer, count);
    }
    //execute algorithm
    switch (cs) {
    case PROBE_CMD:
        params1 = 0;
        params2 = 0;
        params3 = 0;
        break;
    case ERASE_CMD:
        params1 = addr;
        params2 = addr + count;
        params3 = 0;
        break;
    case WRITE_CMD:
        params1 = buffer_addr;
        params2 = addr;
        params3 = count;
        break;
    default:
        params1 = 0;
        params2 = 0;
        params3 = 0;
        break;
    }
    sprintf(temp, "+:algorithm:%x,%x,%llx,%x,%x,%x;", loader_addr, cs, flash.spi_base, params1, params2, params3);
    send.clear();
    send.append(temp);
    TransmitTargetCmd(send);
    if (WaitForTargetRsp()) {
        target_rsp_queue.dequeue();
    }
    send.clear();
    send.append('c');
    TransmitTargetCmd(send);
    if (WaitForTargetRsp()) {
        target_rsp_queue.dequeue();
    }
}

void Transmit::command_print(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    QString data_bin = QString::vasprintf(format, ap);
    va_end(ap);
    cpuinfo_hex.append(bin_to_hex(data_bin.toLatin1(), data_bin.toLatin1().size()));
}

void Transmit::NucleiCpuinfo(void)
{
    QByteArray temp;
    quint64 csr_marchid, csr_mimpid, csr_mcfg, csr_micfg, csr_mdcfg,
            iregion_base, csr_mirgb, csr_mfiocfg, csr_mppicfg, csr_mtlbcfg;

    command_print("----Supported configuration information\n");
    csr_marchid = ReadTargetRegister(RV_REG_MARCHID);
    command_print("         MARCHID: %llx\n", csr_marchid);
    csr_mimpid = ReadTargetRegister(RV_REG_MIMPID);
    command_print("          MIMPID: %llx\n", csr_mimpid);
    /* ISA */
    command_print("             ISA:");
    if (1 == misa->mxl) {
        command_print(" RV32");
    } else {
        command_print(" RV64");
    }
    for (int i = 0; i < EXTENSION_NUM; i++) {
        if (misa->value & BIT(i)) {
            if ('X' == ('A' + i)) {
                command_print(" Zc XLCZ");
            } else {
                command_print(" %c", 'A' + i);
            }
        }
    }
    command_print("\n");
    /* Support */
    csr_mcfg = ReadTargetRegister(RV_REG_MCFG_INFO);
    command_print("            MCFG:");
    if (csr_mcfg & BIT(0)) {
        command_print(" TEE");
    }
    if (csr_mcfg & BIT(1)) {
        command_print(" ECC");
    }
    if (csr_mcfg & BIT(2)) {
        command_print(" ECLIC");
    }
    if (csr_mcfg & BIT(3)) {
        command_print(" PLIC");
    }
    if (csr_mcfg & BIT(4)) {
        command_print(" FIO");
    }
    if (csr_mcfg & BIT(5)) {
        command_print(" PPI");
    }
    if (csr_mcfg & BIT(6)) {
        command_print(" NICE");
    }
    if (csr_mcfg & BIT(7)) {
        command_print(" ILM");
    }
    if (csr_mcfg & BIT(8)) {
        command_print(" DLM");
    }
    if (csr_mcfg & BIT(9)) {
        command_print(" ICACHE");
    }
    if (csr_mcfg & BIT(10)) {
        command_print(" DCACHE");
    }
    if (csr_mcfg & BIT(11)) {
        command_print(" SMP");
    }
    if (csr_mcfg & BIT(12)) {
        command_print(" DSP-N1");
    }
    if (csr_mcfg & BIT(13)) {
        command_print(" DSP-N2");
    }
    if (csr_mcfg & BIT(14)) {
        command_print(" DSP-N3");
    }
    if (csr_mcfg & BIT(16)) {
        command_print(" IREGION");
    }
    if (csr_mcfg & BIT(20)) {
        command_print(" ETRACE");
    }
    command_print("\n");
    /* ILM */
    if (csr_mcfg & BIT(7)) {
        csr_micfg = ReadTargetRegister(RV_REG_MICFG_INFO);
        command_print("             ILM:");
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_micfg, 0x1F << 16) - 1) * 256);
        if (csr_micfg & BIT(21)) {
            command_print(" execute-only");
        }
        if (csr_micfg & BIT(22)) {
            command_print(" has-ecc");
        }
        command_print("\n");
    }
    /* DLM */
    if (csr_mcfg & BIT(8)) {
        csr_mdcfg = ReadTargetRegister(RV_REG_MDCFG_INFO);
        command_print("             DLM:");
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mdcfg, 0x1F << 16) - 1) * 256);
        if (csr_mdcfg & BIT(21)) {
            command_print(" has-ecc");
        }
        command_print("\n");
    }
    /* ICACHE */
    if (csr_mcfg & BIT(9)) {
        csr_micfg = ReadTargetRegister(RV_REG_MICFG_INFO);
        command_print("          ICACHE:");
        show_cache_info(POWER_FOR_TWO(EXTRACT_FIELD(csr_micfg, 0xF) +3),
                        EXTRACT_FIELD(csr_micfg, 0x7 << 4) + 1,
                        POWER_FOR_TWO(EXTRACT_FIELD(csr_micfg, 0x7 << 7) + 2));
    }
    /* DCACHE */
    if (csr_mcfg & BIT(10)) {
        csr_mdcfg = ReadTargetRegister(RV_REG_MDCFG_INFO);
        command_print("          DCACHE:");
        show_cache_info(POWER_FOR_TWO(EXTRACT_FIELD(csr_mdcfg, 0xF) +3),
                        EXTRACT_FIELD(csr_mdcfg, 0x7 << 4) + 1,
                        POWER_FOR_TWO(EXTRACT_FIELD(csr_mdcfg, 0x7 << 7) + 2));
    }
    /* IREGION */
    if (csr_mcfg & BIT(16)) {
        csr_mirgb = ReadTargetRegister(RV_REG_MIRGB_INFO);
        command_print("         IREGION:");
        iregion_base = csr_mirgb & (~0x3FF);
        command_print(" %#lx", iregion_base);
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mirgb, 0xF << 1) - 1) * KB);
        command_print("\n");
        command_print("                  Unit        Size        Address\n");
        command_print("                  INFO        64KB        %#lx\n", iregion_base);
        command_print("                  DEBUG       64KB        %#lx\n", iregion_base + 0x10000);
        if (csr_mcfg & BIT(2)) {
            command_print("                  ECLIC       64KB        %#lx\n", iregion_base + 0x20000);
        }
        command_print("                  TIMER       64KB        %#lx\n", iregion_base + 0x30000);
        if (csr_mcfg & BIT(11)) {
            command_print("                  SMP&CC      64KB        %#lx\n", iregion_base + 0x40000);
        }
        uint64_t smp_cfg = ReadTargetMemory(iregion_base + 0x40004, 4).toLongLong(NULL, 16);
        if ((csr_mcfg & BIT(2)) && (EXTRACT_FIELD(smp_cfg, 0x1F << 1) >= 2)) {
            command_print("                  CIDU        64KB        %#lx\n", iregion_base + 0x50000);
        }
        if (csr_mcfg & BIT(3)) {
            command_print("                  PLIC        64MB        %#lx\n", iregion_base + 0x4000000);
        }
        /* SMP */
        if (csr_mcfg & BIT(11)) {
            command_print("         SMP_CFG:");
            command_print(" CC_PRESENT=%ld", EXTRACT_FIELD(smp_cfg, 0x1));
            command_print(" SMP_CORE_NUM=%ld", EXTRACT_FIELD(smp_cfg, 0x1F << 1));
            command_print(" IOCP_NUM=%ld", EXTRACT_FIELD(smp_cfg, 0x3F << 7));
            command_print(" PMON_NUM=%ld\n", EXTRACT_FIELD(smp_cfg, 0x3F << 13));
        }
        /* L2CACHE */
        if (smp_cfg & 0x1) {
            command_print("         L2CACHE:");
            uint64_t cc_cfg = ReadTargetMemory(iregion_base + 0x40008, 4).toLongLong(NULL, 16);
            show_cache_info(POWER_FOR_TWO(EXTRACT_FIELD(cc_cfg, 0xF)), EXTRACT_FIELD(cc_cfg, 0x7 << 4) + 1,
                            POWER_FOR_TWO(EXTRACT_FIELD(cc_cfg, 0x7 << 7) + 2));
        }
        /* INFO */
        command_print("     INFO-Detail:\n");
        uint64_t mpasize = ReadTargetMemory(iregion_base, 4).toLongLong(NULL, 16);
        command_print("                  mpasize : %ld\n", mpasize);
        uint64_t cmo_info = ReadTargetMemory(iregion_base + 4, 4).toLongLong(NULL, 16);
        if (cmo_info & 0x1) {
            command_print("                  cbozero : %ldByte\n", POWER_FOR_TWO(EXTRACT_FIELD(cmo_info, 0xF << 6) + 2));
            command_print("                  cmo     : %ldByte\n", POWER_FOR_TWO(EXTRACT_FIELD(cmo_info, 0xF << 2) + 2));
            if (cmo_info & 0x2) {
                command_print("                  has_prefecth\n");
            }
        }
        uint64_t mcppi_cfg_lo = ReadTargetMemory(iregion_base + 80, 4).toLongLong(NULL, 16);
        uint64_t mcppi_cfg_hi = ReadTargetMemory(iregion_base + 84, 4).toLongLong(NULL, 16);
        if (mcppi_cfg_lo & 0x1) {
            if (1 == misa->mxl) {
                command_print("                  cppi    : %#lx", mcppi_cfg_lo & (~0x3FF));
            } else {
                command_print("                  cppi    : %#lx", (mcppi_cfg_hi << 32) | (mcppi_cfg_lo & (~0x3FF)));
            }
            print_size(POWER_FOR_TWO(EXTRACT_FIELD(mcppi_cfg_lo, 0xF << 1) - 1) * KB);
            command_print("\n");
        }
    }
    /* TLB */
    if (csr_mcfg & BIT(3)) {
        csr_mtlbcfg = ReadTargetRegister(RV_REG_MTLBCFG_INFO);
        if (csr_mtlbcfg) {
            command_print("            DTLB: %ld entry\n", EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 19));
            command_print("            ITLB: %ld entry\n", EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 16));
            command_print("            MTLB:");
            command_print(" %ld entry", POWER_FOR_TWO(EXTRACT_FIELD(csr_mtlbcfg, 0xF) + 3) *
                                            (EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 4) + 1) *
                                            (EXTRACT_FIELD(csr_mtlbcfg, 0x7 << 7) - 1));
            if (csr_mtlbcfg & BIT(10)) {
                command_print(" has_ecc");
            }
            command_print("\n");
        }
    }
    /* FIO */
    if (csr_mcfg & BIT(4)) {
        csr_mfiocfg = ReadTargetRegister(RV_REG_MFIOCFG_INFO);
        command_print("             FIO:");
        command_print(" %#lx", csr_mfiocfg & (~0x3FF));
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mfiocfg, 0xF << 1) - 1) * KB);
        command_print("\n");
    }
    /* PPI */
    if (csr_mcfg & BIT(5)) {
        csr_mppicfg = ReadTargetRegister(RV_REG_MPPICFG_INFO);
        command_print("             PPI:");
        command_print(" %#lx", csr_mppicfg & (~0x3FF));
        print_size(POWER_FOR_TWO(EXTRACT_FIELD(csr_mppicfg, 0xF << 1) - 1) * KB);
        command_print("\n");
    }
    command_print("----End of cpuinfo\n");
}

QByteArray Transmit::TransmitTargetRsp(QByteArray msg)
{
    return msg.mid(msg.indexOf('$') + 1, msg.indexOf('#') - msg.indexOf('$') - 1);
}

void Transmit::TransmitTargetCmd(QByteArray msg)
{
    emit TransmitToTarget(TransmitPackage(msg));
}

void Transmit::TransmitServerCmdDeal(QByteArray msg)
{
    QByteArray send, recv, cache;
    quint64 addr, len, temp;
    switch (msg[0]) {
    case '\x03':/* Ctrl+C command */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'q':
        if (strncmp(msg.constData(), "qSupported:", 11) == 0) {
            noack_mode = false;
            //Set protocol and connect target
            send.clear();
            send.append("+:set:protocol:");
            send.append(protocol.toLatin1());
            send.append(';');
            TransmitTargetCmd(send);
            if (WaitForTargetRsp()) {
                recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                if (recv.contains("protocol")) {
                    qDebug() << "set protocol and connect success.";
                } else {
                    qDebug() << "set protocol and connect fail.";
                }
            }
            //Get target MISA CSR register
            send.clear();
            send.append("+:read:misa;");
            TransmitTargetCmd(send);
            if (WaitForTargetRsp()) {
                recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                if (recv.contains("misa")) {
                    sscanf(recv.constData(), "-:read:misa:%08llx;", &temp);
                    misa->MisaInit(temp);
                    memxml->InitMemXml(misa);
                    qDebug() << "read misa:" << QString("%1").arg(temp, 4, 16);
                } else {
                    qDebug() << "read misa fail.";
                }
            }
            //Get target VLENB CSR register
            send.clear();
            send.append("+:read:vlenb;");
            TransmitTargetCmd(send);
            if (WaitForTargetRsp()) {
                recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                if (recv.contains("vlenb")) {
                    sscanf(recv.constData(), "-:read:vlenb:%016llx;", &temp);
                    regxml->InitRegXml(misa, temp);
                    qDebug() << "read vlenb:" << QString("%1").arg(temp, 4, 16);
                } else {
                    qDebug() << "read vlenb fail.";
                }
            }
            //deal qSupported
            send.clear();
            send.append("PacketSize=405;"
                        "QStartNoAckMode+;"
                        "qXfer:features:read+;"
                        "qXfer:memory-map:read+;"
                        "swbreak+;"
                        "hwbreak+;");
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "qXfer:memory-map:read::", 23) == 0) {
            sscanf(msg.constData(), "qXfer:memory-map:read::%llx,%llx", &addr, &len);
            temp = memxml->GetMemXmlLen();
            if (len >= (temp - addr)) {
                len = temp - addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(memxml->GetMemXml(addr).constData(), len);
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "qXfer:features:read:target.xml:", 31) == 0) {
            sscanf(msg.constData(), "qXfer:features:read:target.xml:%llx,%llx", &addr, &len);
            temp = regxml->GetRegXmlLen();
            if (len >= (temp - addr)) {
                len = temp - addr;
                send.append('l');
            } else {
                send.append('m');
            }
            send.append(regxml->GetRegXml(addr).constData(), len);
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "qRcmd,", 6) == 0) {
            cache = hex_to_bin(msg.mid(msg.indexOf(',') + 1), msg.length() - msg.indexOf(','));
            if (strncmp(cache.constData(), "nuclei cpuinfo", strlen("nuclei cpuinfo")) == 0) {
                cpuinfo_hex.clear();
                NucleiCpuinfo();
                TransmitServerRsp(cpuinfo_hex);
            } else {
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
        } else {
            /* Not support 'q' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    case 'Q':
        if (strncmp(msg.constData(), "QStartNoAckMode", 15) == 0) {
            TransmitTargetCmd(msg);
            if (WaitForTargetRsp()) {
                TransmitServerRsp(target_rsp_queue.dequeue());
                noack_mode = true;
            }
        } else {
            /* Not support 'Q' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    case 'H':/* `H op thread-id` */
        /* Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
        TransmitServerRsp(send);
        break;
    case '?':/* `?` */
        /* Indicate the reason the target halted. The reply is the same as for step and continue. */
        send.append("S02");
        TransmitServerRsp(send);
        break;
    case 'g':/* `g` */
        /* Read general registers. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'G':/* `G XX...` */
        /* Write general registers. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'k':/* `k` */
        /* Kill request. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'c':/* `c [addr]` */
        /* Continue at addr, which is the address to resume. */
        /* If addr is omitted, resume at current address. */
        TransmitTargetCmd(msg);
        send.append("OK");
        TransmitServerRsp(send);
        break;
    case 'm':/* `m addr,length` */
        /* Read length addressable memory units starting at address addr. */
        /* Note that addr may not be aligned to any particular boundary. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'M':/* `M addr,length:XX...` */
        /* Write length addressable memory units starting at address addr. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'X':/* `X addr,length:XX...` */
        /* Write data to memory, where the data is transmitted in binary. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'p':/* `p n` */
        /* Read the value of register n; n is in hex. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'P':/* `P n...=r...` */
        /* Write register n... with value r... The register number n is in hexadecimal, */
        /* and r... contains two hex digits for each byte in the register (target byte order). */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 's':/* `s [addr]` */
        /* Single step, resuming at addr. If addr is omitted, resume at same address. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'z':/* `z type,addr,kind` */
        /* remove a type breakpoint or watchpoint starting at address, address of kind. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'Z':/* `Z type,addr,kind` */
        /* Insert a type breakpoint or watchpoint starting at address, address of kind. */
        TransmitTargetCmd(msg);
        if (WaitForTargetRsp()) {
            TransmitServerRsp(target_rsp_queue.dequeue());
        }
        break;
    case 'v':
        if (strncmp(msg.constData(), "vMustReplyEmpty", 15) == 0) {
            TransmitServerRsp(send);
        } else if (strncmp(msg.constData(), "vFlashErase:", 12) == 0) {
            if (workarea.addr) {
                //backup workarea
                if (workarea.backup) {
                    workarea.mem = ReadTargetMemory(workarea.addr, workarea.size);
                }
                //download flash loader
                QFile loader(flash.loader_path);
                if (loader.exists()) {
                    loader.open(QIODevice::ReadOnly);
                    cache = loader.readAll();
                    loader_addr = workarea.addr;
                    WriteTargetMemory(loader_addr, cache, cache.size());
                    buffer_addr = loader_addr + cache.size();
                } else {
                    qDebug() << flash.loader_path << " not found.";
                }
                ExecuteAlgorithm(PROBE_CMD, 0, 0, NULL);
                sscanf(msg.constData(), "vFlashErase:%llx,%llx", &addr, &len);
                qDebug() << "Erase:" << QString("%1").arg(addr, 4, 16) << ":" << QString("%1").arg(len, 4, 16);
                ExecuteAlgorithm(ERASE_CMD, addr, len, NULL);
                send.append("OK");
                TransmitServerRsp(send);
            } else {
                char temp_buf[1024];
                sprintf(temp_buf, "vFlashInit:%llx,%llx;", flash.spi_base, flash.block_size);
                send.append(temp_buf);
                TransmitTargetCmd(send);
                if (WaitForTargetRsp()) {
                    recv = TransmitTargetRsp(target_rsp_queue.dequeue());
                    if (recv.contains("OK")) {
                        qDebug() << "flash init ok.";
                    } else {
                        qDebug() << "flash init fail.";
                    }
                }
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
        } else if (strncmp(msg.constData(), "vFlashWrite:", 12) == 0) {
            if (workarea.addr) {
                sscanf(msg.constData(), "vFlashWrite:%llx:", &addr);
                cache = bin_decode(msg.mid(msg.indexOf(':', 15) + 1));
                len = cache.size();
                qDebug() << "Write:" << QString("%1").arg(addr, 4, 16) << ":" << QString("%1").arg(len, 4, 16);
                ExecuteAlgorithm(WRITE_CMD, addr, len, cache);
                send.append("OK");
                TransmitServerRsp(send);
            } else {
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
        } else if (strncmp(msg.constData(), "vFlashDone", 10) == 0) {
            if (workarea.addr) {
                //restore workarea
                if (workarea.backup) {
                    WriteTargetMemory(workarea.addr, workarea.mem, workarea.size);
                }
                send.append("OK");
                TransmitServerRsp(send);
            } else {
                TransmitTargetCmd(msg);
                if (WaitForTargetRsp()) {
                    TransmitServerRsp(target_rsp_queue.dequeue());
                }
            }
        } else {
            /* Not support 'v' command, reply empty. */
            TransmitServerRsp(send);
        }
        break;
    default:/* Not support command, reply empty. */
        TransmitServerRsp(send);
        break;
    }
}

void Transmit::TransmitServerCmd(QByteArray msg)
{
    if (msg[0] == '\x03') {
        TransmitServerCmdDeal(msg.left(1));
    } else {
        TransmitServerCmdDeal(msg.mid(msg.indexOf('$') + 1,
                                  msg.indexOf('#') - msg.indexOf('$') - 1));
    }
}

void Transmit::TransmitServerRsp(QByteArray msg)
{
    emit TransmitToServer(TransmitPackage(msg));
    server_reply_flag = true;
}

void Transmit::run()
{
    QByteArray msg;
    close_flag = false;
    while (1) {
        if (close_flag) {
            break;
        }
        if (server_reply_flag) {
            if (!server_cmd_queue.empty()) {
                server_reply_flag = false;
                msg = server_cmd_queue.dequeue();
                current_command = msg;
                TransmitServerCmd(msg);
            }
        }
        if (!target_rsp_queue.empty()) {
            msg = target_rsp_queue.dequeue();
            emit TransmitToServer(TransmitPackage(msg));
        }
    }
}

void Transmit::TransmitClose()
{
    close_flag = true;
}
