#include "../include/etrace.h"

#define get_field(reg, mask) (((reg) & (mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(mask)) | (((val) * ((mask) & ~((mask) << 1))) & (mask)))
#define field_value(mask, val) set_field((quint64) 0, mask, val)

#define CSR_MCONTROL_TYPE(XLEN)             (0xf * (1ULL<<(XLEN + -4)))
#define CSR_TDATA1_TYPE_MCONTROL            (2)
#define CSR_MCONTROL_ACTION                 (0xf000)
#define CSR_MCONTROL_ACTION_TRACE_ON        (2)
#define CSR_MCONTROL_ACTION_TRACE_OFF       (3)
#define CSR_MCONTROL_M                      (0x40)
#define CSR_MCONTROL_S                      (0x10)
#define CSR_MCONTROL_U                      (8)
#define CSR_MCONTROL_EXECUTE                (4)
#define GDB_REGNO_TSELECT                   (0x7a0+65)
#define GDB_REGNO_TDATA1                    (0x7a1+65)
#define GDB_REGNO_TDATA2                    (0x7a2+65)
#define GDB_REGNO_DPC                       (0x7b1+65)

#define ETRACE_BASE_HI      (0x00)
#define ETRACE_BASE_LO      (0x04)
#define ETRACE_WLEN         (0x08)
#define ETRACE_ENA          (0x0c)
#define ETRACE_INTERRUPT    (0x10)
#define ETRACE_MAXTIME      (0x14)
#define ETRACE_EARLY        (0x18)
#define ETRACE_ATOVF        (0x1c)
#define ETRACE_ENDOFFSET    (0x20)
#define ETRACE_FLG          (0x24)
#define ETRACE_ONGOING      (0x28)
#define ETRACE_TIMEOUT      (0x2c)
#define ETRACE_IDLE         (0x30)
#define ETRACE_FIFO         (0x34)
#define ETRACE_ATBDW        (0x38)
#define ETRACE_WRAP         (0x3c)
#define ETRACE_COMPACT      (0x40)

Etrace::Etrace(QObject *parent)
    : QObject{parent}
{
    type = new Type;
}

void Etrace::CommandPrint(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    QString data_bin = QString::vasprintf(format, ap);
    va_end(ap);
    info_hex.append(type->bin_to_hex(data_bin.toLatin1(), data_bin.toLatin1().size()));
}

void Etrace::Config(Target* target)
{
    target->WriteMemory(etrace_addr + ETRACE_BASE_HI,
                        type->uint32_to_hex_le(buffer_addr >> 32),
                        4);
    target->WriteMemory(etrace_addr + ETRACE_BASE_LO,
                        type->uint32_to_hex_le(buffer_addr),
                        4);
    target->WriteMemory(etrace_addr + ETRACE_WLEN,
                        type->uint32_to_hex_le(buffer_size),
                        4);
    target->WriteMemory(etrace_addr + ETRACE_TIMEOUT,
                        type->uint32_to_hex_le(timeout),
                        4);
    target->WriteMemory(etrace_addr + ETRACE_WRAP,
                        type->uint32_to_hex_le(wrap),
                        4);
}

void Etrace::Enable(Target* target)
{
    quint64 dpc_rb;
    quint64 tdata1;
    quint64 xlen;

    if (1 == target->misa->mxl) {
        xlen = 32;
    } else {
        xlen = 64;
    }
    tdata1 = field_value(CSR_MCONTROL_TYPE(xlen), CSR_TDATA1_TYPE_MCONTROL) |
             field_value(CSR_MCONTROL_ACTION, CSR_MCONTROL_ACTION_TRACE_ON) |
             field_value(CSR_MCONTROL_M, 1) |
             field_value(CSR_MCONTROL_S, 1) |
             field_value(CSR_MCONTROL_U, 1) |
             field_value(CSR_MCONTROL_EXECUTE, 1);
    target->WriteRegister(GDB_REGNO_TSELECT, 0);
    target->WriteRegister(GDB_REGNO_TDATA1, tdata1);
    dpc_rb = target->ReadRegister(GDB_REGNO_DPC);
    target->WriteRegister(GDB_REGNO_TDATA2, dpc_rb);
}

void Etrace::Disable(Target* target)
{
    quint64 dpc_rb;
    quint64 tdata1;
    quint64 xlen;

    if (1 == target->misa->mxl) {
        xlen = 32;
    } else {
        xlen = 64;
    }
    tdata1 = field_value(CSR_MCONTROL_TYPE(xlen), CSR_TDATA1_TYPE_MCONTROL) |
             field_value(CSR_MCONTROL_ACTION, CSR_MCONTROL_ACTION_TRACE_OFF) |
             field_value(CSR_MCONTROL_M, 1) |
             field_value(CSR_MCONTROL_S, 1) |
             field_value(CSR_MCONTROL_U, 1) |
             field_value(CSR_MCONTROL_EXECUTE, 1);
    target->WriteRegister(GDB_REGNO_TSELECT, 1);
    target->WriteRegister(GDB_REGNO_TDATA1, tdata1);
    dpc_rb = target->ReadRegister(GDB_REGNO_DPC);
    target->WriteRegister(GDB_REGNO_TDATA2, dpc_rb);
}

void Etrace::Start(Target* target)
{
    target->WriteMemory(etrace_addr + ETRACE_WRAP,
                        type->uint32_to_hex_le(1),
                        4);
}

void Etrace::Stop(Target* target)
{
    QByteArray read;
    quint32 temp;

    do {
        read = target->ReadMemory(etrace_addr + ETRACE_ONGOING, 4);
        temp = type->hex_to_uint32_le(read);
    } while(temp);
    do {
        read = target->ReadMemory(etrace_addr + ETRACE_FIFO, 4);
        temp = type->hex_to_uint32_le(read);
    } while(temp);
    do {
        read = target->ReadMemory(etrace_addr + ETRACE_TIMEOUT, 4);
        temp = type->hex_to_uint32_le(read);
    } while(temp != 0xFFFFFFFF);
    target->WriteMemory(etrace_addr + ETRACE_ENA,
                        type->uint32_to_hex_le(0),
                        4);
    do {
        read = target->ReadMemory(etrace_addr + ETRACE_IDLE, 4);
        temp = type->hex_to_uint32_le(read);
    } while(temp != 1);
}

void Etrace::Dump(Target* target, QString file_path)
{
    QByteArray read;
    quint32 end_offset;
    quint32 full_flag;
    quint64 address;
    quint64 size;

    read = target->ReadMemory(etrace_addr + ETRACE_ENDOFFSET, 4);
    end_offset = type->hex_to_uint32_le(read);
    read = target->ReadMemory(etrace_addr + ETRACE_FLG, 4);
    full_flag = type->hex_to_uint32_le(read);
    if (full_flag) {
        address = buffer_addr + end_offset;
        size = buffer_size;
    } else {
        address = buffer_addr;
        size = end_offset;
    }

    QFile etrace_f(file_path);
    etrace_f.open(QIODevice::WriteOnly);
    read = target->ReadMemory(address, size);
    etrace_f.write(read);
    etrace_f.close();
}

void Etrace::Clear(Target* target)
{
    Stop(target);

    target->WriteMemory(etrace_addr + ETRACE_ENDOFFSET,
                        type->uint32_to_hex_le(0),
                        4);
    target->WriteMemory(etrace_addr + ETRACE_FLG,
                        type->uint32_to_hex_le(0),
                        4);
}

void Etrace::Info(Target* target, Server* server)
{
    QByteArray read;
    quint32 end_offset;
    quint32 full_flag;

    read = target->ReadMemory(etrace_addr + ETRACE_ENDOFFSET, 4);
    end_offset = type->hex_to_uint32_le(read);
    read = target->ReadMemory(etrace_addr + ETRACE_FLG, 4);
    full_flag = type->hex_to_uint32_le(read);

    info_hex.clear();
    CommandPrint("Etrace Base: %#llx\n", etrace_addr);
    CommandPrint("Buffer Addr: %#llx\n", buffer_addr);
    CommandPrint("Buffer Size: %#llx\n", buffer_size);
    CommandPrint("Buffer Status: ");
    if (full_flag) {
        CommandPrint("used 100%% from %#llx [wraped]\n", buffer_addr + end_offset);
    } else {
        CommandPrint("used %d%% from %#llx to %#llx\n", (end_offset * 100) / buffer_size, buffer_addr, buffer_addr + end_offset);
    }
    server->Write(info_hex);
}
