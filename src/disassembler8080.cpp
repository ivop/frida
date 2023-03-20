// ---------------------------------------------------------------------------
//
// This file is part of:
//
// FRIDA - FRee Interactive DisAssembler
// Copyright (C) 2017,2023 Ivo van Poorten
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; ONLY version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ---------------------------------------------------------------------------

#include "disassembler.h"
#include "frida.h"
#include <QDebug>

enum addressing_mode {
    MODE_IMPL = 0,      // single byte instructions, except for RST
    MODE_D8,            // one data byte argument
    MODE_D16,           // two data bytes argument
    MODE_ADR,           // two address bytes argument
    MODE_JMP,           // jump and call instruction with address argument
    MODE_RST,           // single byte call instruction
    MODE_RET,           // all RET variants, useful for tracing
    MODE_LAST,
};

struct distabitem {
    const char * const inst;
    const char mode;
    const char * descr;
};

static const quint8 isizes[MODE_LAST] = {
    [MODE_IMPL] = 1,
    [MODE_D8]   = 2,
    [MODE_D16]  = 3,
    [MODE_ADR]  = 3,
    [MODE_JMP]  = 3,
    [MODE_RST]  = 1,
    [MODE_RET]  = 1,
};

static const QString operand_description[MODE_LAST] = {
    [MODE_IMPL] = "",
    [MODE_D8]   = "d8",
    [MODE_D16]  = "d16",
    [MODE_ADR]  = "adr",
    [MODE_JMP]  = "adr",
    [MODE_RST]  = "",
    [MODE_RET]  = "",
};

// Thanks to http://www.emulator101.com/reference/8080-by-opcode.html for
// this fine table :) Massaged it to fit my needs.

constexpr struct distabitem UNDEFINED = { "UNDEFINED", MODE_IMPL, "UNDEFINED" };

static struct distabitem distab8080[256] = {

    // "MNEMONIC", MODE, "DESCRIPTION [FLAGS AFFECTED]"

    // 00H - 0FH

{ "NOP",    MODE_IMPL,  "Nothing" },
{ "LXI B,", MODE_D16,   "B <- byte 3; C <- byte 2" },
{ "STAX B", MODE_IMPL,  "(BC) <- A" },
{ "INX B",  MODE_IMPL,  "BC <- BC+1" },
{ "INR B",  MODE_IMPL,  "B <- B+1 [Z,S,P,AC]" },
{ "DCR B",  MODE_IMPL,  "B <- B-1 [Z,S,P,AC]" },
{ "MVI B,", MODE_D8,    "B <- byte 2" },
{ "RLC",    MODE_IMPL,  "A = A << 1;bit 0 = prev bit 7;CY = prev bit 7 [CY]" },
UNDEFINED,
{ "DAD B",  MODE_IMPL,  "HL = HL + BC [CY]" },
{ "LDAX B", MODE_IMPL,  "A <- (BC)" },
{ "DCX B",  MODE_IMPL,  "BC = BC-1 [CY]" },
{ "INR C",  MODE_IMPL,  "C <- C+1 [Z,S,P,AC]" },
{ "DCR C",  MODE_IMPL,  "C <- C-1 [Z,S,P,AC]" },
{ "MVI C,", MODE_D8,    "C <- byte 2" },
{ "RRC",    MODE_IMPL,  "A = A >> 1;bit 7 = prev bit 0;CY = prev bit 0 [CY]" },

    // 10H - 1fH

UNDEFINED,
{ "LXI D,", MODE_D16,   "D <- byte 3; E <- byte 2" },
{ "STAX D", MODE_IMPL,  "(DE) <- A" },
{ "INX D",  MODE_IMPL,  "DE <- DE + 1" },
{ "INR D",  MODE_IMPL,  "D <- D+1 [Z,S,P,AC]" },
{ "DCR D",  MODE_IMPL,  "D <- D-1 [Z,S,P,AC]" },
{ "MVI D,", MODE_D8,    "D <- byte 2" },
{ "RAL",    MODE_IMPL,  "A = A << 1;bit 0 = prev CY;CY = prev bit 7 [CY]" },
UNDEFINED,
{ "DAD D",  MODE_IMPL,  "HL = HL + DE [CY]" },
{ "LDAX D", MODE_IMPL,  "A <- (DE)" },
{ "DCX D",  MODE_IMPL,  "DE = DE-1" },
{ "INR E",  MODE_IMPL,  "E <- E+1 [Z,S,P,AC]" },
{ "DCR E",  MODE_IMPL,  "E <- E-1 [Z,S,P,AC]" },
{ "MVI E,", MODE_D8,    "E <- byte 2" },
{ "RAR",    MODE_IMPL,  "A = A >> 1;bit 7 = prev bit 7;CY = prev bit 0 [CY]" },

    // 20H - 2fH

UNDEFINED,
{ "LXI H,", MODE_D16,   "H <- byte 3; L <- byte 2" },
{ "SHLD",   MODE_ADR,   "(adr) <-L;(adr+1) <- H" },
{ "INX H",  MODE_IMPL,  "HL <- HL + 1" },
{ "INR H",  MODE_IMPL,  "H <- H+1 [Z,S,P,AC]" },
{ "DCR H",  MODE_IMPL,  "H <- H-1 [Z,S,P,AC]" },
{ "MVI H,", MODE_D8,    "H <- byte 2" },
{ "DAA",    MODE_IMPL,  "Decimal Adjust Accumulator" },
UNDEFINED,
{ "DAD H",  MODE_IMPL,  "HL = HL + HI [CY]" },
{ "LHLD",   MODE_ADR,   "L <- (adr);H <- (adr+1)" },
{ "DCX H",  MODE_IMPL,  "HL = HL-1" },
{ "INR L",  MODE_IMPL,  "L <- L+1 [Z,S,P,AC]" },
{ "DCR L",  MODE_IMPL,  "L <- L-1 [Z,S,P,AC]" },
{ "MVI L,", MODE_D8,    "L <- byte 2" },
{ "CMA",    MODE_IMPL,  "A <- !A" },

    // 30H - 3fH

UNDEFINED,
{ "LXI SP,",MODE_D16,   "SP.hi <- byte 3;SP.lo <- byte 2" },
{ "STA",    MODE_ADR,   "(adr) <- A" },
{ "INX SP", MODE_IMPL,  "SP = SP + 1" },
{ "INR M",  MODE_IMPL,  "(HL) <- (HL)+1 [Z,S,P,AC]" },
{ "DCR M",  MODE_IMPL,  "(HL) <- (HL)-1 [Z,S,P,AC]" },
{ "MVI M,", MODE_D8,    "(HL) <- byte 2" },
{ "STC",    MODE_IMPL,  "CY	CY = 1" },
UNDEFINED,
{ "DAD SP", MODE_IMPL,  "HL = HL + SP [CY]" },
{ "LDA",    MODE_ADR,   "A <- (adr)" },
{ "DCX SP", MODE_IMPL,  "SP = SP-1" },
{ "INR A",  MODE_IMPL,  "A <- A+1 [Z,S,P,AC]" },
{ "DCR A",  MODE_IMPL,  "A <- A-1 [Z,S,P,AC]" },
{ "MVI A,", MODE_D8,    "A <- byte 2" },
{ "CMC",    MODE_IMPL,  "CY=!CY [CY]" },

    // 40H - 4FH

{ "MOV B,B",MODE_IMPL, "B <- B" },
{ "MOV B,C",MODE_IMPL, "B <- C" },
{ "MOV B,D",MODE_IMPL, "B <- D" },
{ "MOV B,E",MODE_IMPL, "B <- E" },
{ "MOV B,H",MODE_IMPL, "B <- H" },
{ "MOV B,L",MODE_IMPL, "B <- L" },
{ "MOV B,M",MODE_IMPL, "B <- (HL)" },
{ "MOV B,A",MODE_IMPL, "B <- A" },
{ "MOV C,B",MODE_IMPL, "C <- B" },
{ "MOV C,C",MODE_IMPL, "C <- C" },
{ "MOV C,D",MODE_IMPL, "C <- D" },
{ "MOV C,E",MODE_IMPL, "C <- E" },
{ "MOV C,H",MODE_IMPL, "C <- H" },
{ "MOV C,L",MODE_IMPL, "C <- L" },
{ "MOV C,M",MODE_IMPL, "C <- (HL)" },
{ "MOV C,A",MODE_IMPL, "C <- A" },

    // 50H - 5FH

{ "MOV D,B",MODE_IMPL, "D <- B" },
{ "MOV D,C",MODE_IMPL, "D <- C" },
{ "MOV D,D",MODE_IMPL, "D <- D" },
{ "MOV D,E",MODE_IMPL, "D <- E" },
{ "MOV D,H",MODE_IMPL, "D <- H" },
{ "MOV D,L",MODE_IMPL, "D <- L" },
{ "MOV D,M",MODE_IMPL, "D <- (HL)" },
{ "MOV D,A",MODE_IMPL, "D <- A" },
{ "MOV E,B",MODE_IMPL, "E <- B" },
{ "MOV E,C",MODE_IMPL, "E <- C" },
{ "MOV E,D",MODE_IMPL, "E <- D" },
{ "MOV E,E",MODE_IMPL, "E <- E" },
{ "MOV E,H",MODE_IMPL, "E <- H" },
{ "MOV E,L",MODE_IMPL, "E <- L" },
{ "MOV E,M",MODE_IMPL, "E <- (HL)" },
{ "MOV E,A",MODE_IMPL, "E <- A" },

    // 60H - 6FH

{ "MOV H,B",MODE_IMPL, "H <- B" },
{ "MOV H,C",MODE_IMPL, "H <- C" },
{ "MOV H,D",MODE_IMPL, "H <- D" },
{ "MOV H,E",MODE_IMPL, "H <- E" },
{ "MOV H,H",MODE_IMPL, "H <- H" },
{ "MOV H,L",MODE_IMPL, "H <- L" },
{ "MOV H,M",MODE_IMPL, "H <- (HL)" },
{ "MOV H,A",MODE_IMPL, "H <- A" },
{ "MOV L,B",MODE_IMPL, "L <- B" },
{ "MOV L,C",MODE_IMPL, "L <- C" },
{ "MOV L,D",MODE_IMPL, "L <- D" },
{ "MOV L,E",MODE_IMPL, "L <- E" },
{ "MOV L,H",MODE_IMPL, "L <- H" },
{ "MOV L,L",MODE_IMPL, "L <- L" },
{ "MOV L,M",MODE_IMPL, "L <- (HL)" },
{ "MOV L,A",MODE_IMPL, "L <- A" },

    // 70H - 7FH

{ "MOV M,B",MODE_IMPL, "(HL) <- B" },
{ "MOV M,C",MODE_IMPL, "(HL) <- C" },
{ "MOV M,D",MODE_IMPL, "(HL) <- D" },
{ "MOV M,E",MODE_IMPL, "(HL) <- E" },
{ "MOV M,H",MODE_IMPL, "(HL) <- H" },
{ "MOV M,L",MODE_IMPL, "(HL) <- L" },
{ "HLT",    MODE_IMPL, "HaLT until next interrupt" },
{ "MOV M,A",MODE_IMPL, "(HL) <- A" },
{ "MOV A,B",MODE_IMPL, "A <- B" },
{ "MOV A,C",MODE_IMPL, "A <- C" },
{ "MOV A,D",MODE_IMPL, "A <- D" },
{ "MOV A,E",MODE_IMPL, "A <- E" },
{ "MOV A,H",MODE_IMPL, "A <- H" },
{ "MOV A,L",MODE_IMPL, "A <- L" },
{ "MOV A,M",MODE_IMPL, "A <- (HL)" },
{ "MOV A,A",MODE_IMPL, "A <- A" },

    // 80H - 8FH

{ "ADD B",  MODE_IMPL, "A <- A + B [Z,S,P,CY,AC]" },
{ "ADD C",  MODE_IMPL, "A <- A + C [Z,S,P,CY,AC]" },
{ "ADD D",  MODE_IMPL, "A <- A + D [Z,S,P,CY,AC]" },
{ "ADD E",  MODE_IMPL, "A <- A + E [Z,S,P,CY,AC]" },
{ "ADD H",  MODE_IMPL, "A <- A + H [Z,S,P,CY,AC]" },
{ "ADD L",  MODE_IMPL, "A <- A + L [Z,S,P,CY,AC]" },
{ "ADD M",  MODE_IMPL, "A <- A + (HL) [Z,S,P,CY,AC]" },
{ "ADD A",  MODE_IMPL, "A <- A + A [Z,S,P,CY,AC]" },
{ "ADC B",  MODE_IMPL, "A <- A + B + CY [Z,S,P,CY,AC]" },
{ "ADC C",  MODE_IMPL, "A <- A + C + CY [Z,S,P,CY,AC]" },
{ "ADC D",  MODE_IMPL, "A <- A + D + CY [Z,S,P,CY,AC]" },
{ "ADC E",  MODE_IMPL, "A <- A + E + CY [Z,S,P,CY,AC]" },
{ "ADC H",  MODE_IMPL, "A <- A + H + CY [Z,S,P,CY,AC]" },
{ "ADC L",  MODE_IMPL, "A <- A + L + CY [Z,S,P,CY,AC]" },
{ "ADC M",  MODE_IMPL, "A <- A + (HL) + CY [Z,S,P,CY,AC]" },
{ "ADC A",  MODE_IMPL, "A <- A + A + CY [Z,S,P,CY,AC]" },

    // 90H - 9FH

{ "SUB B",  MODE_IMPL, "A <- A - B [Z,S,P,CY,AC]" },
{ "SUB C",  MODE_IMPL, "A <- A - C [Z,S,P,CY,AC]" },
{ "SUB D",  MODE_IMPL, "A <- A + D [Z,S,P,CY,AC]" },
{ "SUB E",  MODE_IMPL, "A <- A - E [Z,S,P,CY,AC]" },
{ "SUB H",  MODE_IMPL, "A <- A + H [Z,S,P,CY,AC]" },
{ "SUB L",  MODE_IMPL, "A <- A - L [Z,S,P,CY,AC]" },
{ "SUB M",  MODE_IMPL, "A <- A + (HL) [Z,S,P,CY,AC]" },
{ "SUB A",  MODE_IMPL, "A <- A - A [Z,S,P,CY,AC]" },
{ "SBB B",  MODE_IMPL, "A <- A - B - CY [Z,S,P,CY,AC]" },
{ "SBB C",  MODE_IMPL, "A <- A - C - CY [Z,S,P,CY,AC]" },
{ "SBB D",  MODE_IMPL, "A <- A - D - CY [Z,S,P,CY,AC]" },
{ "SBB E",  MODE_IMPL, "A <- A - E - CY [Z,S,P,CY,AC]" },
{ "SBB H",  MODE_IMPL, "A <- A - H - CY [Z,S,P,CY,AC]" },
{ "SBB L",  MODE_IMPL, "A <- A - L - CY [Z,S,P,CY,AC]" },
{ "SBB M",  MODE_IMPL, "A <- A - (HL) - CY [Z,S,P,CY,AC]" },
{ "SBB A",  MODE_IMPL, "A <- A - A - CY [Z,S,P,CY,AC]" },

    // A0H - AFH

{ "ANA B",  MODE_IMPL, "A <- A & B [Z,S,P,CY,AC]" },
{ "ANA C",  MODE_IMPL, "A <- A & C [Z,S,P,CY,AC]" },
{ "ANA D",  MODE_IMPL, "A <- A & D [Z,S,P,CY,AC]" },
{ "ANA E",  MODE_IMPL, "A <- A & E [Z,S,P,CY,AC]" },
{ "ANA H",  MODE_IMPL, "A <- A & H [Z,S,P,CY,AC]" },
{ "ANA L",  MODE_IMPL, "A <- A & L [Z,S,P,CY,AC]" },
{ "ANA M",  MODE_IMPL, "A <- A & (HL) [Z,S,P,CY,AC]" },
{ "ANA A",  MODE_IMPL, "A <- A & A [Z,S,P,CY,AC]" },
{ "XRA B",  MODE_IMPL, "A <- A ^ B [Z,S,P,CY,AC]" },
{ "XRA C",  MODE_IMPL, "A <- A ^ C [Z,S,P,CY,AC]" },
{ "XRA D",  MODE_IMPL, "A <- A ^ D [Z,S,P,CY,AC]" },
{ "XRA E",  MODE_IMPL, "A <- A ^ E [Z,S,P,CY,AC]" },
{ "XRA H",  MODE_IMPL, "A <- A ^ H [Z,S,P,CY,AC]" },
{ "XRA L",  MODE_IMPL, "A <- A ^ L [Z,S,P,CY,AC]" },
{ "XRA M",  MODE_IMPL, "A <- A ^ (HL) [Z,S,P,CY,AC]" },
{ "XRA A",  MODE_IMPL, "A <- A ^ A [Z,S,P,CY,AC]" },

    // B0H - BFH

{ "ORA B",  MODE_IMPL, "A <- A | B [Z,S,P,CY,AC]" },
{ "ORA C",  MODE_IMPL, "A <- A | C [Z,S,P,CY,AC]" },
{ "ORA D",  MODE_IMPL, "A <- A | D [Z,S,P,CY,AC]" },
{ "ORA E",  MODE_IMPL, "A <- A | E [Z,S,P,CY,AC]" },
{ "ORA H",  MODE_IMPL, "A <- A | H [Z,S,P,CY,AC]" },
{ "ORA L",  MODE_IMPL, "A <- A | L [Z,S,P,CY,AC]" },
{ "ORA M",  MODE_IMPL, "A <- A | (HL) [Z,S,P,CY,AC]" },
{ "ORA A",  MODE_IMPL, "A <- A | A [Z,S,P,CY,AC]" },
{ "CMP B",  MODE_IMPL, "A - B [Z,S,P,CY,AC]" },
{ "CMP C",  MODE_IMPL, "A - C [Z,S,P,CY,AC]" },
{ "CMP D",  MODE_IMPL, "A - D [Z,S,P,CY,AC]" },
{ "CMP E",  MODE_IMPL, "A - E [Z,S,P,CY,AC]" },
{ "CMP H",  MODE_IMPL, "A - H [Z,S,P,CY,AC]" },
{ "CMP L",  MODE_IMPL, "A - L [Z,S,P,CY,AC]" },
{ "CMP M",  MODE_IMPL, "A - (HL) [Z,S,P,CY,AC]" },
{ "CMP A",  MODE_IMPL, "A - A [Z,S,P,CY,AC]" },

    // C0H - CFH

{ "RNZ",    MODE_RET,   "if NZ, RET" },
{ "POP B",  MODE_IMPL,  "C <- (SP);B <- (SP+1);SP <- SP+2" },
{ "JNZ",    MODE_JMP,   "if NZ, PC <- adr" },
{ "JMP",    MODE_JMP,   "PC <- adr" },
{ "CNZ",    MODE_JMP,   "if NZ, CALL adr" },
{ "PUSH B", MODE_IMPL,  "(SP-2) <- C;(SP-1) <- B;SP <- SP-2" },
{ "ADI",    MODE_D8,    "A <- A + byte [Z,S,P,CY,AC]" },
{ "RST 0",  MODE_RST,   "CALL 00H" },
{ "RZ",     MODE_RET,   "if Z, RET" },
{ "RET",    MODE_RET,   "PC.lo <- (SP);PC.hi <- (SP+1);SP <- SP+2" },
{ "JZ",     MODE_JMP,   "if Z, PC <- adr" },
UNDEFINED,
{ "CZ",     MODE_JMP,   "if Z, CALL adr" },
{ "CALL",   MODE_JMP,   "(SP-1) <- PC.hi;(SP-2) <- PC.lo;SP <- SP-2;PC <- adr" },
{ "ACI",    MODE_D8,    "A <- A + data + CY [Z,S,P,CY,AC]" },
{ "RST 1",  MODE_RST,   "CALL 08H" },

    // D0H - DFH

{ "RNC",    MODE_RET,   "if NCY, RET" },
{ "POP D",  MODE_IMPL,  "E <- (SP);D <- (SP+1);SP <- SP+2" },
{ "JNC",    MODE_JMP,   "if NCY, PC<-adr" },
{ "OUT",    MODE_D8,    "OUTput A to device num" },
{ "CNC",    MODE_JMP,   "if NCY, CALL adr" },
{ "PUSH D", MODE_IMPL,  "(SP-2) <- E;(SP-1) <- D;SP <- SP-2" },
{ "SUI",    MODE_D8,    "A <- A - data [Z,S,P,CY,AC]" },
{ "RST 2",  MODE_RST,   "CALL 10H" },
{ "RC",     MODE_RET,   "if CY, RET" },
UNDEFINED,
{ "JC",     MODE_JMP,   "if CY, PC <- adr" },
{ "IN",     MODE_D8,    "INput from device num to A" },
{ "CC",     MODE_JMP,   "if CY, CALL adr" },
UNDEFINED,
{ "SBI",    MODE_D8,    "A <- A - data - CY [Z,S,P,CY,AC]" },
{ "RST 3",  MODE_RST,   "CALL 18H" },

    // E0H - EFH

{ "RPO",    MODE_RET,   "if POdd, RET" },
{ "POP H",  MODE_IMPL,  "L <- (SP);H <- (SP+1);SP <- SP+2" },
{ "JPO",    MODE_JMP,   "if POdd, PC <- adr" },
{ "XTHL",   MODE_IMPL,  "L <-> (SP);H <-> (SP+1)" },
{ "CPO",    MODE_JMP,   "if POdd, CALL adr" },
{ "PUSH H", MODE_IMPL,  "(SP-2) <- L;(SP-1) <- H;SP <- SP - 2" },
{ "ANI",    MODE_D8,    "A <- A & data [Z,S,P,CY,AC]" },
{ "RST 4",  MODE_RST,   "CALL 20H" },
{ "RPE",    MODE_RET,   "if PEven, RET" },
{ "PCHL",   MODE_IMPL,  "PC.hi <- H;PC.lo <- L" },     // unknown "jump"
{ "JPE",    MODE_JMP,   "if PEven, PC <- adr" },
{ "XCHG",   MODE_IMPL,  "H <-> D;L <-> E" },
{ "CPE",    MODE_JMP,   "if PEven, CALL adr" },
UNDEFINED,
{ "XRI",    MODE_D8,    "A <- A ^ data [Z,S,P,CY,AC]" },
{ "RST 5",  MODE_RST,   "CALL 28H" },

    // F0H - FFH

{ "RP",     MODE_RET,   "if Plus, RET" },
{ "POP PSW",MODE_IMPL,  "flags <- (SP);A <- (SP+1);SP <- SP+2 [Z,S,P,CY,AC]" },
{ "JP",     MODE_JMP,   "if Plus, PC <- adr" },
{ "DI",     MODE_IMPL,  "Disable Interrupts" },
{ "CP",     MODE_JMP,   "if Plus, PC <- adr" },
{ "PUSH PSW", MODE_IMPL,"(SP-2)<-flags;(SP-1)<-A;SP <- SP-2" },
{ "ORI",    MODE_D8,    "A <- A | data [Z, S, P, CY, AC]" },
{ "RST 6",  MODE_RST,   "CALL 30H" },
{ "RM",     MODE_RET,   "if Minus, RET" },
{ "SPHL",   MODE_IMPL,  "SP <- HL" },
{ "JM",     MODE_JMP,   "if Minus, PC <- adr" },
{ "EI",     MODE_IMPL,  "Enable Interrupts" },
{ "CM",     MODE_JMP,   "if Minus, CALL adr" },
UNDEFINED,
{ "CPI",    MODE_D8,    "A - data [Z,S,P,CY,AC]" },
{ "RST 7",  MODE_RST,   "CALL 38H" },

};

static struct distabitem *distab;

void Disassembler8080::initTables(void) {
    distab = distab8080;
    hexPrefix = QLatin1String("");
    hexSuffix = QStringLiteral("H");
    toUpper = true;
}

int Disassembler8080::getInstructionSizeAt(quint64 address) {
    quint8 *data = segments[currentSegment].data;
    return isizes[(enum addressing_mode)distab[data[address]].mode];
}

void Disassembler8080::createOperandLabels(quint64 address) {
    QMap<quint64,QString> *localLabels = &segments[currentSegment].localLabels;
    quint8 *data = segments[currentSegment].data;
    quint16 opcode;
    quint16 operand = 0;
    QString temps;
    quint64 i = address;

    opcode = data[i];

    struct distabitem item = distab[opcode];
    auto m = (enum addressing_mode) item.mode;

    if (m == MODE_ADR || m == MODE_JMP) {
        operand = data[i+1] + (data[i+2]<<8);

        if (  localLabels->contains(operand) || userLabels.contains(operand))
            return;

        temps = QStringLiteral("L%1").arg(operand,4,16,(QChar)'0');
        if (toUpper)
            temps = temps.toUpper();
        autoLabels.insert(operand, temps);

    }
}

void Disassembler8080::disassembleInstructionAt(quint64 address,
                                            struct disassembly &dis, int &n) {
    QMap<quint64,QString> *localLabels = &segments[currentSegment].localLabels;
//    quint8 *flags = segments[currentSegment].flags;
    quint8 *data = segments[currentSegment].data;
    quint64 start = segments[currentSegment].start;
    quint16 opcode;
    quint16 operand = 0;
    QString temps;
    quint64 i = address;

    opcode = data[i];

    struct distabitem item = distab[opcode];
    auto m = (enum addressing_mode) item.mode;

    n = isizes[m];

    if (n == 2) {   // always MODE_D8
        operand = data[i+1];

        temps = QString(hexPrefix + "%1" + hexSuffix).arg(operand, 2, 16, QChar('0'));
        if (toUpper)
            temps = temps.toUpper();
    }

    if (n == 3) {   // always m == MODE_D16 || m == MODE_ADR || m == MODE_JMP
        operand = data[i+1] + (data[i+2] << 8);

        if (localLabels->contains(operand))
            temps = localLabels->value(operand);
        else if (userLabels.contains(operand))
            temps = userLabels.value(operand);
        else
            temps = autoLabels.value(operand);

        if (temps.isEmpty()) {
            temps = QString(hexPrefix + "%1" + hexSuffix).arg(operand, 4, 16, QChar('0'));
            if (toUpper)
                temps = temps.toUpper();
        }
    }

    dis = { start + i, distab[opcode].inst, temps, n,false, (quint64) opcode,
                                                (quint64) operand, 0 };
    if (m == MODE_JMP)
        dis.changes_pc = true;
}

void Disassembler8080::trace(quint64 address) {
    quint8 *data = segments[currentSegment].data;
    quint8 *datatypes = segments[currentSegment].datatypes;
    quint64 start = segments[currentSegment].start;
    quint64 end = segments[currentSegment].end;
    quint64 size = end - start + 1;
    auto *mark = new quint8[size]();  // () --> all zeroed
    QList<quint64> tracelist;
    tracelist.append(address);

    if (address < start || address > end) {
        delete[] mark;
        return;
    }

    initTables();

    while (!tracelist.isEmpty()) {
        address = tracelist.first();
        tracelist.removeFirst();

        while (true) {
            quint64 i = address - start;
            quint16 opcode = data[i];
            quint16 operand = 0;
            struct distabitem item = distab[opcode];
            auto m = (enum addressing_mode) item.mode;
            int n = isizes[m];

            if (address+n-1>end)
                break;
            if (  datatypes[i] != DT_UNDEFINED_BYTES
               && datatypes[i] != DT_CODE)
                break;

            for (int j=0; j<n; j++) {
                datatypes[i+j] = DT_CODE;
                mark[i+j] = 1;
            }

            if (n == 3 && m == MODE_JMP) {
                operand = data[i+1] + (data[i+2] << 8);

                if (operand >= start && operand <= end) {
                    if (!mark[operand-start])
                        tracelist.append(operand);
                }

                if (opcode == 0xc3 || opcode == 0xc9)   // JMP and RET
                    break;
            }

            if (opcode == 0xe9) // PCHL is also a jump
                break;

            if (distab[opcode].inst == QStringLiteral("UNDEFINED")) {
                datatypes[i] = DT_UNDEFINED_CODE;
                break;
            }

            address += n;

        } // while(1)
    } // tracelist
    delete[] mark;
}

QString Disassembler8080::getDescriptionAt(quint64 address) {
    quint64 start = segments[currentSegment].start;
    quint8 *data = segments[currentSegment].data;
    quint8 *datatypes = segments[currentSegment].datatypes;

    quint64 i = address - start;

    if (datatypes[i] != DT_CODE)
        return QLatin1String("");

    quint8 opcode = data[i];
    quint8 m = distab[opcode].mode;

    QString descr = distab[opcode].descr;
    descr = descr.replace(QLatin1String(";"), QLatin1String("<br>"));
    descr = descr.replace(QLatin1String("["), QLatin1String("<br><br><i>flags: "));
    descr = descr.replace(QLatin1String("]"), QLatin1String("</i><br>"));
    descr = descr.replace(QLatin1String("<->"), QLatin1String("&lt;-&gt;"));
    descr = descr.replace(QLatin1String("<-"), QLatin1String("&lt;-"));           // &larr; looks ugly
    descr = descr.replace(QLatin1String("->"), QLatin1String("-&gt;"));           // &rarr; looks ugly
    descr = descr.replace(QLatin1String("d8"), QLatin1String("<i>d8</i>"));
    descr = descr.replace(QLatin1String("d16"), QLatin1String("<i>d16</i>"));
    descr = descr.replace(QLatin1String("adr"), QLatin1String("<i>adr</i>"));

    descr = "<h3>" + QString(distab[opcode].inst) + " <i>" +
            operand_description[m] + "</i></h3>" + descr;

    return descr;
}

