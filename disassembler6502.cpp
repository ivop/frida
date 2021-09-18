#include <QDebug>
#include <QMessageBox>
#include "frida.h"
#include "disassembler.h"

enum addressing_mode {

    // no operand byte

    MODE_ACCU = 0,      // e.g. rol A
    MODE_IMPL,          // clc, ...

    // one operand byte

    MODE_IMM,           // lda #10
    MODE_REL,           // branches
    MODE_ZP,            // lda $12
    MODE_ZP_X,          // lda $12,x
    MODE_ZP_Y,          // lda $12,y
    MODE_IND_X,         // lda ($12,x)
    MODE_IND_Y,         // lda ($12),y

    // two operand bytes

    MODE_ABS,           // lda $1234
    MODE_IND,           // jmp ($1234)
    MODE_ABS_X,         // lda $1234,x
    MODE_ABS_Y,         // lda $1234,y
    MODE_LAST
};

static const char * const fmts[MODE_LAST] = {
    [MODE_ACCU]  = "A",
    [MODE_IMPL]  = "",
    [MODE_IMM]   = "#%1",
    [MODE_REL]   = "%1",
    [MODE_ZP]    = "%1",
    [MODE_ZP_X]  = "%1,x",
    [MODE_ZP_Y]  = "%1,y",
    [MODE_IND_X] = "(%1,x)",
    [MODE_IND_Y] = "(%1),y",
    [MODE_ABS]   = "%1",
    [MODE_IND]   = "(%1)",
    [MODE_ABS_X] = "%1,x",
    [MODE_ABS_Y] = "%1,y"
};

static const char isizes[MODE_LAST] = {
    [MODE_ACCU]  = 1,
    [MODE_IMPL]  = 1,
    [MODE_IMM]   = 2,
    [MODE_REL]   = 2,
    [MODE_ZP]    = 2,
    [MODE_ZP_X]  = 2,
    [MODE_ZP_Y]  = 2,
    [MODE_IND_X] = 2,
    [MODE_IND_Y] = 2,
    [MODE_ABS]   = 3,
    [MODE_IND]   = 3,
    [MODE_ABS_X] = 3,
    [MODE_ABS_Y] = 3
};

static const char can_be_label[MODE_LAST] = {
    [MODE_ACCU]  = 0,
    [MODE_IMPL]  = 0,
    [MODE_IMM]   = 0,
    [MODE_REL]   = 1,
    [MODE_ZP]    = 1,
    [MODE_ZP_X]  = 1,
    [MODE_ZP_Y]  = 1,
    [MODE_IND_X] = 1,
    [MODE_IND_Y] = 1,
    [MODE_ABS]   = 1,
    [MODE_IND]   = 1,
    [MODE_ABS_X] = 1,
    [MODE_ABS_Y] = 1
};

struct distabitem {
    const char * const inst;
    const char mode;
};

#define UNDEFINED { "UNDEFINED", MODE_IMPL }

static struct distabitem distabNMOS6502[256] = {
    // $00-$0f
{ "brk", MODE_IMPL },   { "ora", MODE_IND_X },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "ora", MODE_ZP },
{ "asl", MODE_ZP },     UNDEFINED,
{ "php", MODE_IMPL },   { "ora", MODE_IMM },
{ "asl", MODE_ACCU },   UNDEFINED,
UNDEFINED,              { "ora", MODE_ABS },
{ "asl", MODE_ABS },    UNDEFINED,

    // $10-$1f
{ "bpl", MODE_REL },    { "ora", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "ora", MODE_ZP_X },
{ "asl", MODE_ZP_X },   UNDEFINED,
{ "clc", MODE_IMPL },   { "ora", MODE_ABS_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "ora", MODE_ABS_X },
{ "asl", MODE_ABS_X },  UNDEFINED,

    // $20-$2f
{ "jsr", MODE_ABS },    { "and", MODE_IND_X },
UNDEFINED,              UNDEFINED,
{ "bit", MODE_ZP },     { "and", MODE_ZP },
{ "rol", MODE_ZP },     UNDEFINED,
{ "plp", MODE_IMPL },   { "and", MODE_IMM },
{ "rol", MODE_ACCU },   UNDEFINED,
{ "bit", MODE_ABS },    { "and", MODE_ABS },
{ "rol", MODE_ABS },    UNDEFINED,

    // $30-$3f
{ "bmi", MODE_REL },    { "and", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "and", MODE_ZP_X },
{ "rol", MODE_ZP_X },   UNDEFINED,
{ "sec", MODE_IMPL },   { "and", MODE_ABS_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "and", MODE_ABS_X },
{ "rol", MODE_ABS_X},   UNDEFINED,

    // $40-$4f
{ "rti", MODE_IMPL },   { "eor", MODE_IND_X },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "eor", MODE_ZP },
{ "lsr", MODE_ZP },     UNDEFINED,
{ "pha", MODE_IMPL },   { "eor", MODE_IMM },
{ "lsr", MODE_ACCU },   UNDEFINED,
{ "jmp", MODE_ABS },    { "eor", MODE_ABS },
{ "lsr", MODE_ABS },    UNDEFINED,

    // $50-$5f
{ "bvc", MODE_REL },    { "eor", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "eor", MODE_ZP_X },
{ "lsr", MODE_ZP_X },   UNDEFINED,
{ "cli", MODE_IMPL },   { "eor", MODE_ABS_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "eor", MODE_ABS_X },
{ "lsr", MODE_ABS_X },  UNDEFINED,

    // $60-$6f
{ "rts", MODE_IMPL },   { "adc", MODE_IND_X },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "adc", MODE_ZP },
{ "ror", MODE_ZP },     UNDEFINED,
{ "pla", MODE_IMPL },   { "adc", MODE_IMM },
{ "ror", MODE_ACCU },   UNDEFINED,
{ "jmp", MODE_IND },    { "adc", MODE_ABS },
{ "ror", MODE_ABS },    UNDEFINED,

    // $70-$7f
{ "bvs", MODE_REL },    { "adc", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "adc", MODE_ZP_X },
{ "ror", MODE_ZP_X },   UNDEFINED,
{ "sei", MODE_IMPL },   { "adc", MODE_ABS_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "adc", MODE_ABS_X },
{ "ror", MODE_ABS_X },  UNDEFINED,

    // $80-$8f
UNDEFINED,              { "sta", MODE_IND_X },
UNDEFINED,              UNDEFINED,
{ "sty", MODE_ZP },     { "sta", MODE_ZP },
{ "stx", MODE_ZP },     UNDEFINED,
{ "dey", MODE_IMPL },   UNDEFINED,
{ "txa", MODE_IMPL },   UNDEFINED,
{ "sty", MODE_ABS },    { "sta", MODE_ABS },
{ "stx", MODE_ABS },    UNDEFINED,

    // $90-$9f
{ "bcc", MODE_REL },    { "sta", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
{ "sty", MODE_ZP_X },   { "sta", MODE_ZP_X },
{ "stx", MODE_ZP_Y },   UNDEFINED,
{ "tya", MODE_IMPL },   { "sta", MODE_ABS_Y },
{ "txs", MODE_IMPL },   UNDEFINED,
UNDEFINED,              { "sta", MODE_ABS_X },
UNDEFINED,              UNDEFINED,

    // $a0-$af
{ "ldy", MODE_IMM },    { "lda", MODE_IND_X },
{ "ldx", MODE_IMM },    UNDEFINED,
{ "ldy", MODE_ZP },     { "lda", MODE_ZP },
{ "ldx", MODE_ZP },     UNDEFINED,
{ "tay", MODE_IMPL },   { "lda", MODE_IMM },
{ "tax", MODE_IMPL },   UNDEFINED,
{ "ldy", MODE_ABS },    { "lda", MODE_ABS },
{ "ldx", MODE_ABS },    UNDEFINED,

    // $b0-$bf
{ "bcs", MODE_REL },    { "lda", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
{ "ldy", MODE_ZP_X },   { "lda", MODE_ZP_X },
{ "ldx", MODE_ZP_Y },   UNDEFINED,
{ "clv", MODE_IMPL },   { "lda", MODE_ABS_Y },
{ "tsx", MODE_IMPL },   UNDEFINED,
{ "ldy", MODE_ABS_X },  { "lda", MODE_ABS_X },
{ "ldx", MODE_ABS_Y },  UNDEFINED,

    // $c0-$cf
{ "cpy", MODE_IMM },    { "cmp", MODE_IND_X },
UNDEFINED,              UNDEFINED,
{ "cpy", MODE_ZP },     { "cmp", MODE_ZP },
{ "dec", MODE_ZP },     UNDEFINED,
{ "iny", MODE_IMPL },   { "cmp", MODE_IMM },
{ "dex", MODE_IMPL },   UNDEFINED,
{ "cpy", MODE_ABS },    { "cmp", MODE_ABS },
{ "dec", MODE_ABS },    UNDEFINED,

    // $d0-$df
{ "bne", MODE_REL },    { "cmp", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "cmp", MODE_ZP_X },
{ "dec", MODE_ZP_X },   UNDEFINED,
{ "cld", MODE_IMPL },   { "cmp", MODE_ABS_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "cmp", MODE_ABS_X },
{ "dec", MODE_ABS_X },  UNDEFINED,

    // $e0-$ef
{ "cpx", MODE_IMM },    { "sbc", MODE_IND_X },
UNDEFINED,              UNDEFINED,
{ "cpx", MODE_ZP },     { "sbc", MODE_ZP },
{ "inc", MODE_ZP },     UNDEFINED,
{ "inx", MODE_IMPL },   { "sbc", MODE_IMM },
{ "nop", MODE_IMPL },   UNDEFINED,
{ "cpx", MODE_ABS },    { "sbc", MODE_ABS },
{ "inc", MODE_ABS },    UNDEFINED,

    // $f0-$ff
{ "beq", MODE_REL },    { "sbc", MODE_IND_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "sbc", MODE_ZP_X },
{ "inc", MODE_ZP_X },   UNDEFINED,
{ "sed", MODE_IMPL },   { "sbc", MODE_ABS_Y },
UNDEFINED,              UNDEFINED,
UNDEFINED,              { "sbc", MODE_ABS_X },
{ "inc", MODE_ABS_X },  UNDEFINED

};

static struct distabitem *distab;

void Disassembler6502::initTables(void) {
    distab = distabNMOS6502;    // XXX add support for undefined instructions
                                // and CMOS 65C02 instruction set
    hexPrefix = "$";
    hexSuffix = "";
}

int Disassembler6502::getInstructionSizeAt(quint64 address) {
    quint8 *data = segments[currentSegment].data;
    return isizes[(enum addressing_mode)distab[data[address]].mode];
}

void Disassembler6502::createOperandLabel(quint64 i) {
    quint8 *data = segments[currentSegment].data;
    quint64 addr = 0, start = segments[currentSegment].start;
    int n = isizes[(enum addressing_mode)distab[data[i]].mode];
    QString hex;

    if (can_be_label[(enum addressing_mode)distab[data[i]].mode]) {
        if (n == 2)
            addr = data[i+1];
        else if (n == 3)
            addr = data[i+1] | data[i+2]<<8;
        if (distab[data[i]].mode == MODE_REL)
            addr = 2 + start + i + addr - (addr>0x7f ? 0x100 : 0);

        if (  segments[currentSegment].localLabels.contains(addr)
           || userLabels.contains(addr))
            return;

        hex = QString("L%1").arg(addr,4,16,(QChar)'0');
        autoLabels.insert(addr, hex);
    }
}

void Disassembler6502::disassembleInstructionAt(quint64 i,
                                             struct disassembly &dis, int &n) {
    QMap<quint64,QString> *localLabels = &segments[currentSegment].localLabels;
    quint8 *flags = segments[currentSegment].flags;
    quint8 *data = segments[currentSegment].data;
    quint64 start = segments[currentSegment].start;
    quint16 opcode = data[i], operand = 0;
    QString temps, hex;
    struct distabitem item = distab[opcode];
    enum addressing_mode m = (enum addressing_mode) item.mode;

    n = isizes[m];
    if (n > 1) {
        operand = data[i+1];
        if (n == 3) {
            operand |= (quint16) data[i+2] << 8;
        }
        if (m == MODE_REL) {
            operand = 2 + start + i + operand - (operand>0x7f ? 0x100 : 0);
        }
        if (can_be_label[m] && (   autoLabels.contains(operand)
                                || userLabels.contains(operand)
                                || localLabels->contains(operand) )) {
            if (localLabels->contains(operand))
                hex = localLabels->value(operand);
            else if (userLabels.contains(operand))
                hex = userLabels.value(operand);
            else
                hex = autoLabels.value(operand);
        } else if (m == MODE_IMM && flags[i+1] & (FLAG_LOW_BYTE|FLAG_HIGH_BYTE)) {
            QMap<quint64,quint16> *map;
            char pref;
            if (flags[i+1] & FLAG_LOW_BYTE) {
                map = &segments[currentSegment].lowbytes;
                pref = '<';
            } else {
                map = &segments[currentSegment].highbytes;
                pref = '>';
            }
            quint16 addr = map->value(start+i+1);
            if (localLabels->contains(addr))
                hex = pref + QString("%1").arg(localLabels->value(addr));
            else if (userLabels.contains(addr))
                hex = pref + QString("%1").arg(userLabels.value(addr));
            else if (autoLabels.contains(addr))
                hex = pref + QString("%1").arg(autoLabels.value(addr));
            else
                hex = pref + QString("$%1").arg(addr, 4, 16, (QChar)'0');
        } else {
            hex   = QString("$%1").arg(operand, 2, 16, (QChar)'0');
        }
        temps = QString(fmts[m]).arg(hex);
    }
    dis = { start + i, distab[opcode].inst, temps, n,false, (quint64) opcode,
                                                (quint64) operand, 0 };
    if (m == MODE_REL || opcode == 0x4c || opcode == 0x6c || opcode == 0x20 ||
            opcode == 0x40 || opcode ==0x60) {
        dis.changes_pc = true;
    }
}

void Disassembler6502::trace(quint64 address) {
    quint8 *data = segments[currentSegment].data;
    quint8 *datatypes = segments[currentSegment].datatypes;
    quint64 start = segments[currentSegment].start;
    quint64 end = segments[currentSegment].end;
    quint64 size = end - start + 1;
    quint8 *mark = new quint8[size]();  // () --> all zeroed
    QList<quint64> tracelist;
    tracelist.append(address);

    if (address < start || address > end)
        return;

    initTables();

    while (!tracelist.isEmpty()) {
        address = tracelist.first();
        tracelist.removeFirst();

        while (1) {
            int i = address - start;
            quint16 opcode = data[i], operand = 0;
            struct distabitem item = distab[opcode];
            enum addressing_mode m = (enum addressing_mode) item.mode;
            int n = isizes[m];

            if (address+n>=end)
                break;
            if (  datatypes[i] != DT_UNDEFINED_BYTES
               && datatypes[i] != DT_CODE)
                break;

            if (n > 1) {
                operand = data[i+1];
                if (n == 3) {
                    operand |= (quint16) data[i+2] << 8;
                }
                if (m == MODE_REL) {
                    operand = 2 + start + i + operand - (operand>0x7f ? 0x100 : 0);
                }
            }

            for (int j=0; j<n; j++) {
                datatypes[i+j] = DT_CODE;
                mark[i+j] = 1;
            }

            // branches or jsr or jmp, and operand is inside this segment
            if ((m == MODE_REL || opcode == 0x20 || opcode == 0x4c)
                    && operand >= start && operand <= end) {
                if (!mark[operand-start])
                    tracelist.append(operand);
            }

            // rts, rti, jmp
            if (opcode == 0x60 || opcode == 0x40 || opcode == 0x4c)
                break;

            if (distab[opcode].inst == QString("UNDEFINED")) {
                datatypes[i] = DT_UNDEFINED_CODE;
                break;
            }

            address += n;
        }

    }
}
