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
    MODE_IND_ZP,        // lda ($12)        65C02 only
    MODE_IND_ABS_X,     // jmp ($1234,x)    65C02 only
    MODE_ZP_REL,        // bbr0 $12,$fe     65C02 only (Rockwell/WDC)
    MODE_LAST
};

static const char * const fmts[MODE_LAST] = {
    [MODE_ACCU]      = "A",
    [MODE_IMPL]      = "",
    [MODE_IMM]       = "#%1",
    [MODE_REL]       = "%1",
    [MODE_ZP]        = "%1",
    [MODE_ZP_X]      = "%1,x",
    [MODE_ZP_Y]      = "%1,y",
    [MODE_IND_X]     = "(%1,x)",
    [MODE_IND_Y]     = "(%1),y",
    [MODE_ABS]       = "%1",
    [MODE_IND]       = "(%1)",
    [MODE_ABS_X]     = "%1,x",
    [MODE_ABS_Y]     = "%1,y",
    [MODE_IND_ZP]    = "(%1)",          // 65C02 only
    [MODE_IND_ABS_X] = "(%1,x)",        // 65C02 only
    [MODE_ZP_REL]    = "%1,%2",         // 65C02 only (Rockwell/WDC)
};

static const int isizes[MODE_LAST] = {
    [MODE_ACCU]      = 1,
    [MODE_IMPL]      = 1,
    [MODE_IMM]       = 2,
    [MODE_REL]       = 2,
    [MODE_ZP]        = 2,
    [MODE_ZP_X]      = 2,
    [MODE_ZP_Y]      = 2,
    [MODE_IND_X]     = 2,
    [MODE_IND_Y]     = 2,
    [MODE_ABS]       = 3,
    [MODE_IND]       = 3,
    [MODE_ABS_X]     = 3,
    [MODE_ABS_Y]     = 3,
    [MODE_IND_ZP]    = 2,               // 65C02 only
    [MODE_IND_ABS_X] = 3,               // 65C02 only
    [MODE_ZP_REL]    = 3,               // 65C02 only (Rockwell/WDC)
};

static const int can_be_label[MODE_LAST] = {
    [MODE_ACCU]      = 0,
    [MODE_IMPL]      = 0,
    [MODE_IMM]       = 0,
    [MODE_REL]       = 1,
    [MODE_ZP]        = 1,
    [MODE_ZP_X]      = 1,
    [MODE_ZP_Y]      = 1,
    [MODE_IND_X]     = 1,
    [MODE_IND_Y]     = 1,
    [MODE_ABS]       = 1,
    [MODE_IND]       = 1,
    [MODE_ABS_X]     = 1,
    [MODE_ABS_Y]     = 1,
    [MODE_IND_ZP]    = 1,               // 65C02 only
    [MODE_IND_ABS_X] = 1,               // 65C02 only
    [MODE_ZP_REL]    = 1,               // 65C02 only (Rockwell/WDC)
};

struct instr_descr {
    const char * const instr;
    const char * const descr;
    const char * const action;
    const char * const flags;
};

static struct instr_descr instruction_descriptions[] = {
{ "adc", "Add Value To Accumulator With Carry", "A + M + C → A, C", "NVZC" },
{ "and", "AND Value with Accumulator", "A ∧ M → A", "NZ" },
{ "asl", "Arithmetic Shift Left", "C ← /M7...M0/ ← 0", "NZC" },
{ "bcc", "Branch on Carry Clear", "Branch on C = 0", "" },
{ "bcs", "Branch on Carry Set", "Branch on C = 1", "" },
{ "beq", "Branch on Equal / Zero", "Branch on Z = 1", ""},
{ "bit", "Test Bits in Memory with Accumulator", "A ∧ M, M7 → N, M6 → V", "NVZ" },
{ "bmi", "Branch on Result Minus", "Branch on N = 1", "" },
{ "bne", "Branch on Not Equal / Not Zero", "Branch on Z = 0", "" },
{ "bpl", "Branch on Result Plus", "Branch on N = 0", "" },
{ "brk", "Break Command", "PC + 2↓, [FFFE] → PCL, [FFFF] → PCH", "I" },
{ "bvc", "Branch on Overflow Clear", "Branch on V = 0", "" },
{ "bvs", "Branch on Overflow Set", "Branch on V = 1", "" },
{ "clc", "Clear Carry Flag", "0 → C", "C" },
{ "cld", "Clear Decimal Mode", "0 → D", "D" },
{ "cli", "Clear Interrupt Disable", "0 → I", "I" },
{ "clv", "Clear Overflow Flag", "0 → V", "V" },
{ "cmp", "Compare Value And Accumulator", "A - M", "NZC" },
{ "cpx", "Compare Index Register X To Value", "X - M", "NZC" },
{ "cpy", "Compare Index Register Y To Value", "Y - M", "NZC" },
{ "dec", "Decrement Memory By One", "M - 1 → M", "NZ" },
{ "dex", "Decrement Index Register X By One", "X - 1 → X", "NZ" },
{ "dey", "Decrement Index Register Y By One", "Y - 1 → Y", "NZ" },
{ "eor", "Exclusive OR Value with Accumulator", "A ⊻ M → A", "NZ" },
{ "inc", "Increment Memory By One", "M + 1 → M", "NZ" },
{ "inx", "Increment Index Register X By One", "X + 1 → X", "NZ" },
{ "iny", "Increment Index Register Y By One", "Y + 1 → Y", "NZ" },
{ "jmp", "Jump To Location", "[PC + 1] → PCL, [PC + 2] → PCH", "" },
{ "jsr", "Jump To Subroutine", "PC + 2↓, [PC + 1] → PCL, [PC + 2] → PCH", "" },
{ "lda", "Load Accumulator With Value", "M → A", "NZ" },
{ "ldx", "Load Index Register X With Value", "M → X", "NZ" },
{ "ldy", "Load Index Register Y With Value", "M → Y", "NZ" },
{ "lsr", "Logical Shift Right", "0 → /M7...M0/ → C", "NZC" },
{ "nop", "No Operation", "", "" },
{ "ora", "OR Value With Accumulator", "A ∨ M → A", "NZ" },
{ "pha", "Push Accumulator On Stack", "A↓", "" },
{ "php", "Push Processor Status On Stack", "P↓", "" },
{ "pla", "Pull Accumulator From Stack", "A↑", "NZ" },
{ "plp", "Pull Processor Status From Stack", "P↑", "NVDIZC" },
{ "rol", "Rotate Left", "C ← /M7...M0/ ← C", "NZC" },
{ "ror", "Rotate Right", "C → /M7...M0/ → C", "NZC" },
{ "rti", "Return From Interrupt", "P↑ PC↑", "NVDIZC" },
{ "rts", "Return From Subroutine", "PC↑, PC + 1 → PC", "" },
{ "sbc", "Subtract Value From Accumulator With Borrow", "A - M - ~C → A", "NVZC" },
{ "sec", "Set Carry Flag", "1 → C", "C" },
{ "sed", "Set Decimal Mode", "1 → D", "D" },
{ "sei", "Set Interrupt Disable", "1 → I", "I" },
{ "sta", "Store Accumulator In Memory", "A → M", "" },
{ "stx", "Store Index Register X In Memory", "X → M", "" },
{ "sty", "Store Index Register Y In Memory", "Y → M", "" },
{ "tax", "Transfer Accumulator To Index X", "A → X", "NZ" },
{ "tay", "Transfer Accumulator To Index Y", "A → Y", "NZ" },
{ "tsx", "Transfer Stack Pointer To Index X", "S → X", "NZ" },
{ "txa", "Transfer Index X To Accumulator", "X → A", "NZ" },
{ "txs", "Transfer Index X To Stack Pointer", "X → S", "" },
{ "tya", "Transfer Index Y To Accumulator", "Y → A", "NZ" },
{ 0, 0, 0, 0 },
};

struct distabitem {
    const char * const inst;
    const char mode;
};

constexpr struct distabitem UNDEFINED = { "UNDEFINED", MODE_IMPL };

static struct distabitem distabNMOS6502[256] = {

    { "brk", MODE_IMPL },       // $00
    { "ora", MODE_IND_X },      // $01
    UNDEFINED,                  // $02
    UNDEFINED,                  // $03
    UNDEFINED,                  // $04
    { "ora", MODE_ZP },         // $05
    { "asl", MODE_ZP },         // $06
    UNDEFINED,                  // $07
    { "php", MODE_IMPL },       // $08
    { "ora", MODE_IMM },        // $09
    { "asl", MODE_ACCU },       // $0a
    UNDEFINED,                  // $0b
    UNDEFINED,                  // $0c
    { "ora", MODE_ABS },        // $0d
    { "asl", MODE_ABS },        // $0e
    UNDEFINED,                  // $0f

    { "bpl", MODE_REL },        // $10
    { "ora", MODE_IND_Y },      // $11
    UNDEFINED,                  // $12
    UNDEFINED,                  // $13
    UNDEFINED,                  // $14
    { "ora", MODE_ZP_X },       // $15
    { "asl", MODE_ZP_X },       // $16
    UNDEFINED,                  // $17
    { "clc", MODE_IMPL },       // $18
    { "ora", MODE_ABS_Y },      // $19
    UNDEFINED,                  // $1a
    UNDEFINED,                  // $1b
    UNDEFINED,                  // $1c
    { "ora", MODE_ABS_X },      // $1d
    { "asl", MODE_ABS_X },      // $1e
    UNDEFINED,                  // $1f

    { "jsr", MODE_ABS },        // $20
    { "and", MODE_IND_X },      // $21
    UNDEFINED,                  // $22
    UNDEFINED,                  // $23
    { "bit", MODE_ZP },         // $24
    { "and", MODE_ZP },         // $25
    { "rol", MODE_ZP },         // $26
    UNDEFINED,                  // $27
    { "plp", MODE_IMPL },       // $28
    { "and", MODE_IMM },        // $29
    { "rol", MODE_ACCU },       // $2a
    UNDEFINED,                  // $2b
    { "bit", MODE_ABS },        // $2c
    { "and", MODE_ABS },        // $2d
    { "rol", MODE_ABS },        // $2d
    UNDEFINED,                  // $2f

    { "bmi", MODE_REL },        // $30
    { "and", MODE_IND_Y },      // $31
    UNDEFINED,                  // $32
    UNDEFINED,                  // $33
    UNDEFINED,                  // $34
    { "and", MODE_ZP_X },       // $35
    { "rol", MODE_ZP_X },       // $36
    UNDEFINED,                  // $37
    { "sec", MODE_IMPL },       // $38
    { "and", MODE_ABS_Y },      // $39
    UNDEFINED,                  // $3a
    UNDEFINED,                  // $3b
    UNDEFINED,                  // $3c
    { "and", MODE_ABS_X },      // $3d
    { "rol", MODE_ABS_X},       // $3e
    UNDEFINED,                  // $3f

    { "rti", MODE_IMPL },       // $40
    { "eor", MODE_IND_X },      // $41
    UNDEFINED,                  // $42
    UNDEFINED,                  // $43
    UNDEFINED,                  // $44
    { "eor", MODE_ZP },         // $45
    { "lsr", MODE_ZP },         // $46
    UNDEFINED,                  // $47
    { "pha", MODE_IMPL },       // $48
    { "eor", MODE_IMM },        // $49
    { "lsr", MODE_ACCU },       // $4a
    UNDEFINED,                  // $4b
    { "jmp", MODE_ABS },        // $4c
    { "eor", MODE_ABS },        // $4d
    { "lsr", MODE_ABS },        // $4e
    UNDEFINED,                  // $4f

    { "bvc", MODE_REL },        // $50
    { "eor", MODE_IND_Y },      // $51
    UNDEFINED,                  // $52
    UNDEFINED,                  // $53
    UNDEFINED,                  // $54
    { "eor", MODE_ZP_X },       // $55
    { "lsr", MODE_ZP_X },       // $56
    UNDEFINED,                  // $57
    { "cli", MODE_IMPL },       // $58
    { "eor", MODE_ABS_Y },      // $59
    UNDEFINED,                  // $5a
    UNDEFINED,                  // $5b
    UNDEFINED,                  // $5c
    { "eor", MODE_ABS_X },      // $5d
    { "lsr", MODE_ABS_X },      // $5e
    UNDEFINED,                  // $5f

    { "rts", MODE_IMPL },       // $60
    { "adc", MODE_IND_X },      // $61
    UNDEFINED,                  // $62
    UNDEFINED,                  // $63
    UNDEFINED,                  // $64
    { "adc", MODE_ZP },         // $65
    { "ror", MODE_ZP },         // $66
    UNDEFINED,                  // $67
    { "pla", MODE_IMPL },       // $68
    { "adc", MODE_IMM },        // $69
    { "ror", MODE_ACCU },       // $6a
    UNDEFINED,                  // $6b
    { "jmp", MODE_IND },        // $6c
    { "adc", MODE_ABS },        // $6d
    { "ror", MODE_ABS },        // $6e
    UNDEFINED,                  // $6f

    { "bvs", MODE_REL },        // $70
    { "adc", MODE_IND_Y },      // $71
    UNDEFINED,                  // $72
    UNDEFINED,                  // $73
    UNDEFINED,                  // $74
    { "adc", MODE_ZP_X },       // $75
    { "ror", MODE_ZP_X },       // $76
    UNDEFINED,                  // $77
    { "sei", MODE_IMPL },       // $78
    { "adc", MODE_ABS_Y },      // $79
    UNDEFINED,                  // $7a
    UNDEFINED,                  // $7b
    UNDEFINED,                  // $7c
    { "adc", MODE_ABS_X },      // $7d
    { "ror", MODE_ABS_X },      // $7e
    UNDEFINED,                  // $7f

    UNDEFINED,                  // $80
    { "sta", MODE_IND_X },      // $81
    UNDEFINED,                  // $82
    UNDEFINED,                  // $83
    { "sty", MODE_ZP },         // $84
    { "sta", MODE_ZP },         // $85
    { "stx", MODE_ZP },         // $86
    UNDEFINED,                  // $87
    { "dey", MODE_IMPL },       // $88
    UNDEFINED,                  // $89
    { "txa", MODE_IMPL },       // $8a
    UNDEFINED,                  // $8b
    { "sty", MODE_ABS },        // $8c
    { "sta", MODE_ABS },        // $8d
    { "stx", MODE_ABS },        // $8e
    UNDEFINED,                  // $8f

    { "bcc", MODE_REL },        // $90
    { "sta", MODE_IND_Y },      // $91
    UNDEFINED,                  // $92
    UNDEFINED,                  // $93
    { "sty", MODE_ZP_X },       // $94
    { "sta", MODE_ZP_X },       // $95
    { "stx", MODE_ZP_Y },       // $96
    UNDEFINED,                  // $97
    { "tya", MODE_IMPL },       // $98
    { "sta", MODE_ABS_Y },      // $99
    { "txs", MODE_IMPL },       // $9a
    UNDEFINED,                  // $9b
    UNDEFINED,                  // $9c
    { "sta", MODE_ABS_X },      // $9d
    UNDEFINED,                  // $9e
    UNDEFINED,                  // $9f

    { "ldy", MODE_IMM },        // $a0
    { "lda", MODE_IND_X },      // $a1
    { "ldx", MODE_IMM },        // $a2
    UNDEFINED,                  // $a3
    { "ldy", MODE_ZP },         // $a4
    { "lda", MODE_ZP },         // $a5
    { "ldx", MODE_ZP },         // $a6
    UNDEFINED,                  // $a7
    { "tay", MODE_IMPL },       // $a8
    { "lda", MODE_IMM },        // $a9
    { "tax", MODE_IMPL },       // $aa
    UNDEFINED,                  // $ab
    { "ldy", MODE_ABS },        // $ac
    { "lda", MODE_ABS },        // $ad
    { "ldx", MODE_ABS },        // $ae
    UNDEFINED,                  // $af

    { "bcs", MODE_REL },        // $b0
    { "lda", MODE_IND_Y },      // $b1
    UNDEFINED,                  // $b2
    UNDEFINED,                  // $b3
    { "ldy", MODE_ZP_X },       // $b4
    { "lda", MODE_ZP_X },       // $b5
    { "ldx", MODE_ZP_Y },       // $b6
    UNDEFINED,                  // $b7
    { "clv", MODE_IMPL },       // $b8
    { "lda", MODE_ABS_Y },      // $b9
    { "tsx", MODE_IMPL },       // $ba
    UNDEFINED,                  // $bb
    { "ldy", MODE_ABS_X },      // $bc
    { "lda", MODE_ABS_X },      // $bd
    { "ldx", MODE_ABS_Y },      // $be
    UNDEFINED,                  // $bf

    { "cpy", MODE_IMM },        // $c0
    { "cmp", MODE_IND_X },      // $c1
    UNDEFINED,                  // $c2
    UNDEFINED,                  // $c3
    { "cpy", MODE_ZP },         // $c4
    { "cmp", MODE_ZP },         // $c5
    { "dec", MODE_ZP },         // $c6
    UNDEFINED,                  // $c7
    { "iny", MODE_IMPL },       // $c8
    { "cmp", MODE_IMM },        // $c9
    { "dex", MODE_IMPL },       // $ca
    UNDEFINED,                  // $cb
    { "cpy", MODE_ABS },        // $cc
    { "cmp", MODE_ABS },        // $cd
    { "dec", MODE_ABS },        // $ce
    UNDEFINED,                  // $cf

    { "bne", MODE_REL },        // $d0
    { "cmp", MODE_IND_Y },      // $d1
    UNDEFINED,                  // $d2
    UNDEFINED,                  // $d3
    UNDEFINED,                  // $d4
    { "cmp", MODE_ZP_X },       // $d5
    { "dec", MODE_ZP_X },       // $d6
    UNDEFINED,                  // $d7
    { "cld", MODE_IMPL },       // $d8
    { "cmp", MODE_ABS_Y },      // $d9
    UNDEFINED,                  // $da
    UNDEFINED,                  // $db
    UNDEFINED,                  // $dc
    { "cmp", MODE_ABS_X },      // $dd
    { "dec", MODE_ABS_X },      // $de
    UNDEFINED,                  // $df

    { "cpx", MODE_IMM },        // $e0
    { "sbc", MODE_IND_X },      // $e1
    UNDEFINED,                  // $e2
    UNDEFINED,                  // $e3
    { "cpx", MODE_ZP },         // $e4
    { "sbc", MODE_ZP },         // $e5
    { "inc", MODE_ZP },         // $e6
    UNDEFINED,                  // $e7
    { "inx", MODE_IMPL },       // $e8
    { "sbc", MODE_IMM },        // $e9
    { "nop", MODE_IMPL },       // $ea
    UNDEFINED,                  // $eb
    { "cpx", MODE_ABS },        // $ec
    { "sbc", MODE_ABS },        // $ed
    { "inc", MODE_ABS },        // $ee
    UNDEFINED,                  // $ef

    { "beq", MODE_REL },        // $f0
    { "sbc", MODE_IND_Y },      // $f1
    UNDEFINED,                  // $f2
    UNDEFINED,                  // $f3
    UNDEFINED,                  // $f4
    { "sbc", MODE_ZP_X },       // $f5
    { "inc", MODE_ZP_X },       // $f6
    UNDEFINED,                  // $f7
    { "sed", MODE_IMPL },       // $f8
    { "sbc", MODE_ABS_Y },      // $f9
    UNDEFINED,                  // $fa
    UNDEFINED,                  // $fb
    UNDEFINED,                  // $fc
    { "sbc", MODE_ABS_X },      // $fd
    { "inc", MODE_ABS_X },      // $fe
    UNDEFINED                   // $ff

};

// ---------------------------------------------------------------------------

// mads assembler names for undefined mnemonics, unless no such mnemonic
// exists (i.e. ANC2, SBC2, ...)

static struct distabitem distabNMOS6502UNDEF[256] = {

                                // std  // undef

    { "brk", MODE_IMPL },       // $00
    { "ora", MODE_IND_X },      // $01
    { "cim", MODE_IMPL },               // $02
    { "aso", MODE_IND_X },              // $03
    { "nop", MODE_ZP },                 // $04
    { "ora", MODE_ZP },         // $05
    { "asl", MODE_ZP },         // $06
    { "aso", MODE_ZP },                 // $07
    { "php", MODE_IMPL },       // $08
    { "ora", MODE_IMM },        // $09
    { "asl", MODE_ACCU },       // $0a
    { "anc", MODE_IMM },                // $0b
    { "top", MODE_ABS },                // $0c
    { "ora", MODE_ABS },        // $0d
    { "asl", MODE_ABS },        // $0e
    { "aso", MODE_ABS },                // $0f

    { "bpl", MODE_REL },        // $10
    { "ora", MODE_IND_Y },      // $11
    { "cim", MODE_IMM },                // $12
    { "aso", MODE_IND_Y },              // $13
    { "nop", MODE_ZP_X },               // $14
    { "ora", MODE_ZP_X },       // $15
    { "asl", MODE_ZP_X },       // $16
    { "aso", MODE_ZP_X },               // $17
    { "clc", MODE_IMPL },       // $18
    { "ora", MODE_ABS_Y },      // $19
    { "npo", MODE_IMPL },               // $1a
    { "aso", MODE_ABS_Y },              // $1b
    { "top", MODE_ABS_X },              // $1c
    { "ora", MODE_ABS_X },      // $1d
    { "asl", MODE_ABS_X },      // $1e
    { "aso", MODE_ABS_X },              // $1f

    { "jsr", MODE_ABS },        // $20
    { "and", MODE_IND_X },      // $21
    { "cim", MODE_ZP },                 // $22
    { "rln", MODE_IND_X },              // $23
    { "bit", MODE_ZP },         // $24
    { "and", MODE_ZP },         // $25
    { "rol", MODE_ZP },         // $26
    { "rln", MODE_ZP },                 // $27
    { "plp", MODE_IMPL },       // $28
    { "and", MODE_IMM },        // $29
    { "rol", MODE_ACCU },       // $2a
    { "anc2", MODE_IMM },               // $2b
    { "bit", MODE_ABS },        // $2c
    { "and", MODE_ABS },        // $2d
    { "rol", MODE_ABS },        // $2d
    { "rln", MODE_ABS },                // $2f

    { "bmi", MODE_REL },        // $30
    { "and", MODE_IND_Y },      // $31
    { "cim", MODE_ZP_X },               // $32
    { "rln", MODE_IND_Y },              // $33
    { "nop", MODE_ZP_X },               // $34
    { "and", MODE_ZP_X },       // $35
    { "rol", MODE_ZP_X },       // $36
    { "rln", MODE_ZP_X },               // $37
    { "sec", MODE_IMPL },       // $38
    { "and", MODE_ABS_Y },      // $39
    { "nop", MODE_IMPL },               // $3a
    { "rln", MODE_ABS_Y },              // $3b
    { "nop", MODE_ABS_X },              // $3c
    { "and", MODE_ABS_X },      // $3d
    { "rol", MODE_ABS_X},       // $3e
    { "rln", MODE_ABS_X },              // $3f

    { "rti", MODE_IMPL },       // $40
    { "eor", MODE_IND_X },      // $41
    { "cim", MODE_ZP_Y },               // $42
    { "lse", MODE_IND_X },              // $43
    { "dop", MODE_ZP },                 // $44
    { "eor", MODE_ZP },         // $45
    { "lsr", MODE_ZP },         // $46
    { "lse", MODE_ZP },                 // $47
    { "pha", MODE_IMPL },       // $48
    { "eor", MODE_IMM },        // $49
    { "lsr", MODE_ACCU },       // $4a
    { "alr", MODE_IMM },                // $4b
    { "jmp", MODE_ABS },        // $4c
    { "eor", MODE_ABS },        // $4d
    { "lsr", MODE_ABS },        // $4e
    { "lse", MODE_ABS },                // $4f

    { "bvc", MODE_REL },        // $50
    { "eor", MODE_IND_Y },      // $51
    { "cim", MODE_IND_X },              // $52
    { "lse", MODE_IND_Y },              // $53
    { "dop", MODE_ZP_X },               // $54
    { "eor", MODE_ZP_X },       // $55
    { "lsr", MODE_ZP_X },       // $56
    { "lse", MODE_ZP_X },               // $57
    { "cli", MODE_IMPL },       // $58
    { "eor", MODE_ABS_Y },      // $59
    { "nop", MODE_IMPL },               // $5a
    { "lse", MODE_ABS_Y },              // $5b
    { "nop", MODE_ABS_X },              // $5c
    { "eor", MODE_ABS_X },      // $5d
    { "lsr", MODE_ABS_X },      // $5e
    { "lse", MODE_ABS_X },              // $5f

    { "rts", MODE_IMPL },       // $60
    { "adc", MODE_IND_X },      // $61
    { "cim", MODE_IND_Y },              // $62
    { "rrd", MODE_IND_X },              // $63
    { "nop", MODE_ZP },                 // $64
    { "adc", MODE_ZP },         // $65
    { "ror", MODE_ZP },         // $66
    { "rrd", MODE_ZP },                 // $67
    { "pla", MODE_IMPL },       // $68
    { "adc", MODE_IMM },        // $69
    { "ror", MODE_ACCU },       // $6a
    { "arr", MODE_IMM },                // $6b
    { "jmp", MODE_IND },        // $6c
    { "adc", MODE_ABS },        // $6d
    { "ror", MODE_ABS },        // $6e
    { "rrd", MODE_ABS },                // $6f

    { "bvs", MODE_REL },        // $70
    { "adc", MODE_IND_Y },      // $71
    { "cim", MODE_ABS },                // $72
    { "rrd", MODE_IND_Y },              // $73
    { "nop", MODE_ZP_X },               // $74
    { "adc", MODE_ZP_X },       // $75
    { "ror", MODE_ZP_X },       // $76
    { "rrd", MODE_ZP_X },               // $77
    { "sei", MODE_IMPL },       // $78
    { "adc", MODE_ABS_Y },      // $79
    { "nop", MODE_IMPL },               // $7a
    { "rrd", MODE_ABS_Y },              // $7b
    { "nop", MODE_ABS_X },              // $7c
    { "adc", MODE_ABS_X },      // $7d
    { "ror", MODE_ABS_X },      // $7e
    { "rrd", MODE_ABS_X },              // $7f

    { "dop", MODE_IMM },                // $80
    { "sta", MODE_IND_X },      // $81
    { "nop", MODE_IMM },                // $82
    { "sax", MODE_IND_X },              // $83
    { "sty", MODE_ZP },         // $84
    { "sta", MODE_ZP },         // $85
    { "stx", MODE_ZP },         // $86
    { "sax", MODE_ZP },                 // $87
    { "dey", MODE_IMPL },       // $88
    { "nop", MODE_IMM },                // $89
    { "txa", MODE_IMPL },       // $8a
    { "ane", MODE_IMM },                // $8b
    { "sty", MODE_ABS },        // $8c
    { "sta", MODE_ABS },        // $8d
    { "stx", MODE_ABS },        // $8e
    { "sax", MODE_ABS },                // $8f

    { "bcc", MODE_REL },        // $90
    { "sta", MODE_IND_Y },      // $91
    { "cim", MODE_ABS_X },              // $92
    { "sha", MODE_IND_Y },              // $93
    { "sty", MODE_ZP_X },       // $94
    { "sta", MODE_ZP_X },       // $95
    { "stx", MODE_ZP_Y },       // $96
    { "sax", MODE_ZP_Y },               // $97
    { "tya", MODE_IMPL },       // $98
    { "sta", MODE_ABS_Y },      // $99
    { "txs", MODE_IMPL },       // $9a
    { "shs", MODE_ABS_Y },              // $9b
    { "shy", MODE_ABS_X },              // $9c
    { "sta", MODE_ABS_X },      // $9d
    { "shx", MODE_ABS_Y },              // $9e
    { "sha", MODE_ABS_Y },              // $9f

    { "ldy", MODE_IMM },        // $a0
    { "lda", MODE_IND_X },      // $a1
    { "ldx", MODE_IMM },        // $a2
    { "lax", MODE_IND_X },              // $a3
    { "ldy", MODE_ZP },         // $a4
    { "lda", MODE_ZP },         // $a5
    { "ldx", MODE_ZP },         // $a6
    { "lax", MODE_ZP },                 // $a7
    { "tay", MODE_IMPL },       // $a8
    { "lda", MODE_IMM },        // $a9
    { "tax", MODE_IMPL },       // $aa
    { "anx", MODE_IMM },                // $ab
    { "ldy", MODE_ABS },        // $ac
    { "lda", MODE_ABS },        // $ad
    { "ldx", MODE_ABS },        // $ae
    { "lax", MODE_ABS },                // $af

    { "bcs", MODE_REL },        // $b0
    { "lda", MODE_IND_Y },      // $b1
    { "cim", MODE_ABS_Y },              // $b2
    { "lax", MODE_IND_Y },              // $b3
    { "ldy", MODE_ZP_X },       // $b4
    { "lda", MODE_ZP_X },       // $b5
    { "ldx", MODE_ZP_Y },       // $b6
    { "lax", MODE_ZP_Y },               // $b7
    { "clv", MODE_IMPL },       // $b8
    { "lda", MODE_ABS_Y },      // $b9
    { "tsx", MODE_IMPL },       // $ba
    { "las", MODE_ABS_Y },              // $bb
    { "ldy", MODE_ABS_X },      // $bc
    { "lda", MODE_ABS_X },      // $bd
    { "ldx", MODE_ABS_Y },      // $be
    { "lax", MODE_ABS_Y },              // $bf

    { "cpy", MODE_IMM },        // $c0
    { "cmp", MODE_IND_X },      // $c1
    { "nop", MODE_IMM },                // $c2
    { "dcp", MODE_IND_X },              // $c3
    { "cpy", MODE_ZP },         // $c4
    { "cmp", MODE_ZP },         // $c5
    { "dec", MODE_ZP },         // $c6
    { "dcp", MODE_ZP },                 // $c7
    { "iny", MODE_IMPL },       // $c8
    { "cmp", MODE_IMM },        // $c9
    { "dex", MODE_IMPL },       // $ca
    { "sbx", MODE_IMM },                // $cb
    { "cpy", MODE_ABS },        // $cc
    { "cmp", MODE_ABS },        // $cd
    { "dec", MODE_ABS },        // $ce
    { "dcp", MODE_ABS },                // $cf

    { "bne", MODE_REL },        // $d0
    { "cmp", MODE_IND_Y },      // $d1
    { "cim", MODE_IND },                // $d2
    { "dcp", MODE_IND_Y },              // $d3
    { "nop", MODE_ZP_X },               // $d4
    { "cmp", MODE_ZP_X },       // $d5
    { "dec", MODE_ZP_X },       // $d6
    { "dcp", MODE_ZP_X },               // $d7
    { "cld", MODE_IMPL },       // $d8
    { "cmp", MODE_ABS_Y },      // $d9
    { "nop", MODE_IMPL },               // $da
    { "dcp", MODE_ABS_Y },              // $db
    { "nop", MODE_ABS_X },              // $dc
    { "cmp", MODE_ABS_X },      // $dd
    { "dec", MODE_ABS_X },      // $de
    { "dcp", MODE_ABS_X },              // $df

    { "cpx", MODE_IMM },        // $e0
    { "sbc", MODE_IND_X },      // $e1
    { "nop", MODE_IMM },                // $e2
    { "isb", MODE_IND_X },              // $e3
    { "cpx", MODE_ZP },         // $e4
    { "sbc", MODE_ZP },         // $e5
    { "inc", MODE_ZP },         // $e6
    { "isb", MODE_ZP },                 // $e7
    { "inx", MODE_IMPL },       // $e8
    { "sbc", MODE_IMM },        // $e9
    { "nop", MODE_IMPL },       // $ea
    { "sbc2", MODE_IMM },               // $eb
    { "cpx", MODE_ABS },        // $ec
    { "sbc", MODE_ABS },        // $ed
    { "inc", MODE_ABS },        // $ee
    { "isb", MODE_ABS },                // $ef

    { "beq", MODE_REL },        // $f0
    { "sbc", MODE_IND_Y },      // $f1
    { "cim", MODE_REL},                 // $f2
    { "isb", MODE_IND_Y },              // $f3
    { "nop", MODE_ZP_X },               // $f4
    { "sbc", MODE_ZP_X },       // $f5
    { "inc", MODE_ZP_X },       // $f6
    { "isb", MODE_ZP_X },               // $f7
    { "sed", MODE_IMPL },       // $f8
    { "sbc", MODE_ABS_Y },      // $f9
    { "nop", MODE_IMPL },               // $fa
    { "isb", MODE_ABS_Y },              // $fb
    { "nop", MODE_ABS_X },              // $fc
    { "sbc", MODE_ABS_X },      // $fd
    { "inc", MODE_ABS_X },      // $fe
    { "isb", MODE_ABS_X }               // $ff

};

// ---------------------------------------------------------------------------

static struct distabitem distabCMOS65C02[256] = {

                                // std  // cmos // rockwell/wdc

    { "brk", MODE_IMPL },       // $00
    { "ora", MODE_IND_X },      // $01
    UNDEFINED,                  // $02
    UNDEFINED,                  // $03
    { "tsb", MODE_ZP },                 // $04
    { "ora", MODE_ZP },         // $05
    { "asl", MODE_ZP },         // $06
    { "rmb0", MODE_ZP },                    // $07
    { "php", MODE_IMPL },       // $08
    { "ora", MODE_IMM },        // $09
    { "asl", MODE_ACCU },       // $0a
    UNDEFINED,                  // $0b
    { "tsb", MODE_ABS },                // $0c
    { "ora", MODE_ABS },        // $0d
    { "asl", MODE_ABS },        // $0e
    { "bbr0", MODE_ZP_REL },                // $0f

    { "bpl", MODE_REL },        // $10
    { "ora", MODE_IND_Y },      // $11
    { "ora", MODE_IND_ZP },             // $12
    UNDEFINED,                  // $13
    { "trb", MODE_ZP },                 // $14
    { "ora", MODE_ZP_X },       // $15
    { "asl", MODE_ZP_X },       // $16
    { "rmb1", MODE_ZP },                    // $17
    { "clc", MODE_IMPL },       // $18
    { "ora", MODE_ABS_Y },      // $19
    { "inc", MODE_ACCU },               // $1a
    UNDEFINED,                  // $1b
    { "trb", MODE_ABS },                // $1c
    { "ora", MODE_ABS_X },      // $1d
    { "asl", MODE_ABS_X },      // $1e
    { "bbr1", MODE_ZP_REL },                // $1f

    { "jsr", MODE_ABS },        // $20
    { "and", MODE_IND_X },      // $21
    UNDEFINED,                  // $22
    UNDEFINED,                  // $23
    { "bit", MODE_ZP },         // $24
    { "and", MODE_ZP },         // $25
    { "rol", MODE_ZP },         // $26
    { "rmb2", MODE_ZP },                    // $27
    { "plp", MODE_IMPL },       // $28
    { "and", MODE_IMM },        // $29
    { "rol", MODE_ACCU },       // $2a
    UNDEFINED,                  // $2b
    { "bit", MODE_ABS },        // $2c
    { "and", MODE_ABS },        // $2d
    { "rol", MODE_ABS },        // $2d
    { "bbr2", MODE_ZP_REL },                // $2f

    { "bmi", MODE_REL },        // $30
    { "and", MODE_IND_Y },      // $31
    { "and", MODE_IND_ZP },             // $32
    UNDEFINED,                  // $33
    { "bit", MODE_ZP_X },               // $34
    { "and", MODE_ZP_X },       // $35
    { "rol", MODE_ZP_X },       // $36
    { "rmb3", MODE_ZP },                    // $37
    { "sec", MODE_IMPL },       // $38
    { "and", MODE_ABS_Y },      // $39
    { "dec", MODE_ACCU },               // $3a
    UNDEFINED,                  // $3b
    { "bit", MODE_ABS_X },              // $3c
    { "and", MODE_ABS_X },      // $3d
    { "rol", MODE_ABS_X},       // $3e
    { "bbr3", MODE_ZP_REL },                // $3f

    { "rti", MODE_IMPL },       // $40
    { "eor", MODE_IND_X },      // $41
    UNDEFINED,                  // $42
    UNDEFINED,                  // $43
    UNDEFINED,                  // $44
    { "eor", MODE_ZP },         // $45
    { "lsr", MODE_ZP },         // $46
    { "rmb4", MODE_ZP },                    // $47
    { "pha", MODE_IMPL },       // $48
    { "eor", MODE_IMM },        // $49
    { "lsr", MODE_ACCU },       // $4a
    UNDEFINED,                  // $4b
    { "jmp", MODE_ABS },        // $4c
    { "eor", MODE_ABS },        // $4d
    { "lsr", MODE_ABS },        // $4e
    { "bbr4", MODE_ZP_REL },                // $4f

    { "bvc", MODE_REL },        // $50
    { "eor", MODE_IND_Y },      // $51
    { "eor", MODE_IND_ZP },             // $52
    UNDEFINED,                  // $53
    UNDEFINED,                  // $54
    { "eor", MODE_ZP_X },       // $55
    { "lsr", MODE_ZP_X },       // $56
    { "rmb5", MODE_ZP },                    // $57
    { "cli", MODE_IMPL },       // $58
    { "eor", MODE_ABS_Y },      // $59
    { "phy", MODE_IMPL },               // $5a
    UNDEFINED,                  // $5b
    UNDEFINED,                  // $5c
    { "eor", MODE_ABS_X },      // $5d
    { "lsr", MODE_ABS_X },      // $5e
    { "bbr5", MODE_ZP_REL },                // $5f

    { "rts", MODE_IMPL },       // $60
    { "adc", MODE_IND_X },      // $61
    UNDEFINED,                  // $62
    UNDEFINED,                  // $63
    { "stz", MODE_ZP },                 // $64
    { "adc", MODE_ZP },         // $65
    { "ror", MODE_ZP },         // $66
    { "rmb6", MODE_ZP },                    // $67
    { "pla", MODE_IMPL },       // $68
    { "adc", MODE_IMM },        // $69
    { "ror", MODE_ACCU },       // $6a
    UNDEFINED,                  // $6b
    { "jmp", MODE_IND },        // $6c
    { "adc", MODE_ABS },        // $6d
    { "ror", MODE_ABS },        // $6e
    { "bbr6", MODE_ZP_REL },                // $6f

    { "bvs", MODE_REL },        // $70
    { "adc", MODE_IND_Y },      // $71
    { "adc", MODE_IND_ZP },             // $72
    UNDEFINED,                  // $73
    { "stz", MODE_ZP_X },               // $74
    { "adc", MODE_ZP_X },       // $75
    { "ror", MODE_ZP_X },       // $76
    { "rmb7", MODE_ZP },                    // $77
    { "sei", MODE_IMPL },       // $78
    { "adc", MODE_ABS_Y },      // $79
    { "ply", MODE_IMPL },               // $7a
    UNDEFINED,                  // $7b
    { "jmp", MODE_IND_ABS_X },          // $7c
    { "adc", MODE_ABS_X },      // $7d
    { "ror", MODE_ABS_X },      // $7e
    { "bbr7", MODE_ZP_REL },                // $7f

    { "bra", MODE_REL },                // $80
    { "sta", MODE_IND_X },      // $81
    UNDEFINED,                  // $82
    UNDEFINED,                  // $83
    { "sty", MODE_ZP },         // $84
    { "sta", MODE_ZP },         // $85
    { "stx", MODE_ZP },         // $86
    { "smb0", MODE_ZP },                    // $87
    { "dey", MODE_IMPL },       // $88
    { "bit", MODE_IMM },                // $89
    { "txa", MODE_IMPL },       // $8a
    UNDEFINED,                  // $8b
    { "sty", MODE_ABS },        // $8c
    { "sta", MODE_ABS },        // $8d
    { "stx", MODE_ABS },        // $8e
    { "bbs0", MODE_ZP_REL },                // $8f

    { "bcc", MODE_REL },        // $90
    { "sta", MODE_IND_Y },      // $91
    { "sta", MODE_IND_ZP },             // $92
    UNDEFINED,                  // $93
    { "sty", MODE_ZP_X },       // $94
    { "sta", MODE_ZP_X },       // $95
    { "stx", MODE_ZP_Y },       // $96
    { "smb1", MODE_ZP },                    // $97
    { "tya", MODE_IMPL },       // $98
    { "sta", MODE_ABS_Y },      // $99
    { "txs", MODE_IMPL },       // $9a
    UNDEFINED,                  // $9b
    { "stz", MODE_ABS },                // $9c
    { "sta", MODE_ABS_X },      // $9d
    { "stz", MODE_ABS_X },              // $9e
    { "bbs1", MODE_ZP_REL },                // $9f

    { "ldy", MODE_IMM },        // $a0
    { "lda", MODE_IND_X },      // $a1
    { "ldx", MODE_IMM },        // $a2
    UNDEFINED,                  // $a3
    { "ldy", MODE_ZP },         // $a4
    { "lda", MODE_ZP },         // $a5
    { "ldx", MODE_ZP },         // $a6
    { "smb2", MODE_ZP },                    // $a7
    { "tay", MODE_IMPL },       // $a8
    { "lda", MODE_IMM },        // $a9
    { "tax", MODE_IMPL },       // $aa
    UNDEFINED,                  // $ab
    { "ldy", MODE_ABS },        // $ac
    { "lda", MODE_ABS },        // $ad
    { "ldx", MODE_ABS },        // $ae
    { "bbs2", MODE_ZP_REL },                // $af

    { "bcs", MODE_REL },        // $b0
    { "lda", MODE_IND_Y },      // $b1
    { "lda", MODE_IND_ZP },             // $b2
    UNDEFINED,                  // $b3
    { "ldy", MODE_ZP_X },       // $b4
    { "lda", MODE_ZP_X },       // $b5
    { "ldx", MODE_ZP_Y },       // $b6
    { "smb3", MODE_ZP },                    // $b7
    { "clv", MODE_IMPL },       // $b8
    { "lda", MODE_ABS_Y },      // $b9
    { "tsx", MODE_IMPL },       // $ba
    UNDEFINED,                  // $bb
    { "ldy", MODE_ABS_X },      // $bc
    { "lda", MODE_ABS_X },      // $bd
    { "ldx", MODE_ABS_Y },      // $be
    { "bbs3", MODE_ZP_REL },                // $bf

    { "cpy", MODE_IMM },        // $c0
    { "cmp", MODE_IND_X },      // $c1
    UNDEFINED,                  // $c2
    UNDEFINED,                  // $c3
    { "cpy", MODE_ZP },         // $c4
    { "cmp", MODE_ZP },         // $c5
    { "dec", MODE_ZP },         // $c6
    { "smb4", MODE_ZP },                    // $c7
    { "iny", MODE_IMPL },       // $c8
    { "cmp", MODE_IMM },        // $c9
    { "dex", MODE_IMPL },       // $ca
    { "wai", MODE_IMPL },                   // $cb
    { "cpy", MODE_ABS },        // $cc
    { "cmp", MODE_ABS },        // $cd
    { "dec", MODE_ABS },        // $ce
    { "bbs4", MODE_ZP_REL },                // $cf

    { "bne", MODE_REL },        // $d0
    { "cmp", MODE_IND_Y },      // $d1
    { "cmp", MODE_IND_ZP },             // $d2
    UNDEFINED,                  // $d3
    UNDEFINED,                  // $d4
    { "cmp", MODE_ZP_X },       // $d5
    { "dec", MODE_ZP_X },       // $d6
    { "smb5", MODE_ZP },                    // $d7
    { "cld", MODE_IMPL },       // $d8
    { "cmp", MODE_ABS_Y },      // $d9
    { "phx", MODE_IMPL },               // $da
    { "stp", MODE_IMPL },                   // $db
    UNDEFINED,                  // $dc
    { "cmp", MODE_ABS_X },      // $dd
    { "dec", MODE_ABS_X },      // $de
    { "bbs5", MODE_ZP_REL },                // $df

    { "cpx", MODE_IMM },        // $e0
    { "sbc", MODE_IND_X },      // $e1
    UNDEFINED,                  // $e2
    UNDEFINED,                  // $e3
    { "cpx", MODE_ZP },         // $e4
    { "sbc", MODE_ZP },         // $e5
    { "inc", MODE_ZP },         // $e6
    { "smb6", MODE_ZP },                    // $e7
    { "inx", MODE_IMPL },       // $e8
    { "sbc", MODE_IMM },        // $e9
    { "nop", MODE_IMPL },       // $ea
    UNDEFINED,                  // $eb
    { "cpx", MODE_ABS },        // $ec
    { "sbc", MODE_ABS },        // $ed
    { "inc", MODE_ABS },        // $ee
    { "bbs6", MODE_ZP_REL },                // $ef

    { "beq", MODE_REL },        // $f0
    { "sbc", MODE_IND_Y },      // $f1
    { "sbc", MODE_IND_ZP },             // $f2
    UNDEFINED,                  // $f3
    UNDEFINED,                  // $f4
    { "sbc", MODE_ZP_X },       // $f5
    { "inc", MODE_ZP_X },       // $f6
    { "smb7", MODE_ZP },                    // $f7
    { "sed", MODE_IMPL },       // $f8
    { "sbc", MODE_ABS_Y },      // $f9
    { "plx", MODE_IMPL },               // $fa
    UNDEFINED,                  // $fb
    UNDEFINED,                  // $fc
    { "sbc", MODE_ABS_X },      // $fd
    { "inc", MODE_ABS_X },      // $fe
    { "bbs7", MODE_ZP_REL },                // $ff

};

// ---------------------------------------------------------------------------

static struct distabitem *distab;

void Disassembler6502::initTables(void) {
    switch(this->cputype) {
        case CT_NMOS6502:       distab = distabNMOS6502;        break;
        case CT_NMOS6502UNDEF:  distab = distabNMOS6502UNDEF;   break;
        case CT_CMOS65C02:      distab = distabCMOS65C02;   break;
        default: break;
    }

    hexPrefix = QStringLiteral("$");
    hexSuffix = QLatin1String("");
    toUpper   = false;
}

int Disassembler6502::getInstructionSizeAt(quint64 address) {
    quint8 *data = segments[currentSegment].data;
    return isizes[(enum addressing_mode)distab[data[address]].mode];
}

void Disassembler6502::createOperandLabels(quint64 address) {
    struct segment *s = &segments[currentSegment];
    quint8 *data = s->data;
    quint64 addr = 0;
    quint64 addr2 = 0;
    quint64 start = s->start;
    quint64 i = address;
    int n = isizes[(enum addressing_mode)distab[data[i]].mode];
    QString hex;

    if (distab[data[i]].mode == MODE_ZP_REL) {
        addr  = data[i+1];
        addr2 = data[i+2];
        addr2 = 3 + start + i + addr2 - (addr2>0x7f ? 0x100 : 0);

        if ( ! (s->localLabels.contains(addr)
                || globalLabels.contains(addr))) {

            hex = QStringLiteral("L%1").arg(addr,4,16,(QChar)'0');
            if (toUpper)
                hex = hex.toUpper();
            globalLabels.insert(addr, hex);
        }

        if ( ! (s->localLabels.contains(addr2)
               || globalLabels.contains(addr2))) {

            hex = QStringLiteral("L%1").arg(addr2,4,16,(QChar)'0');
            if (toUpper)
                hex = hex.toUpper();
            globalLabels.insert(addr2, hex);
        }

    } else if (can_be_label[(enum addressing_mode)distab[data[i]].mode]) {
        if (n == 2)
            addr = data[i+1];
        else if (n == 3)
            addr = data[i+1] | data[i+2]<<8;
        if (distab[data[i]].mode == MODE_REL)
            addr = 2 + start + i + addr - (addr>0x7f ? 0x100 : 0);

        if (s->localLabels.contains(addr)
           || globalLabels.contains(addr))
            return;

        hex = QStringLiteral("L%1").arg(addr,4,16,(QChar)'0');
        if (toUpper)
            hex = hex.toUpper();
        globalLabels.insert(addr, hex);
    }
}

void Disassembler6502::disassembleInstructionAt(quint64 address,
                                             struct disassembly &dis, int &n) {
    struct segment *s = &segments[currentSegment];
    QMap<quint64,QString> *localLabels = &s->localLabels;
    quint8 *flags = s->flags;
    quint8 *data = s->data;
    quint64 start = s->start;
    quint16 opcode;
    quint16 operand = 0;
    quint16 operand2 = 0;
    QString temps;
    QString hex;
    QString hex2;
    quint64 i = address;

    opcode = data[i];

    struct distabitem item = distab[opcode];
    auto m = (enum addressing_mode) item.mode;

    n = isizes[m];

    if (n > 1) {
        operand = data[i+1];

        if (n == 3) {
            operand |= (quint16) data[i+2] << 8;
        }

        if (m == MODE_REL) {
            operand = 2 + start + i + operand - (operand>0x7f ? 0x100 : 0);
        }

        if (m == MODE_ZP_REL) {

            operand  = data[i+1];
            operand2 = data[i+2];
            operand2 = 3 + start + i + operand2 - (operand2>0x7f ? 0x100 : 0);

            if (localLabels->contains(operand))
                hex = localLabels->value(operand);
            else if (globalLabels.contains(operand))
                hex = globalLabels.value(operand);
            else
                hex = QStringLiteral("$%1").arg(operand, 2, 16, (QChar)'0');

            if (localLabels->contains(operand2))
                hex2 = localLabels->value(operand2);
            else if (globalLabels.contains(operand2))
                hex2 = globalLabels.value(operand2);
            else
                hex2 = QStringLiteral("$%1").arg(operand2, 2, 16, (QChar)'0');


        } else if (can_be_label[m] && (globalLabels.contains(operand)
                                    || localLabels->contains(operand) )) {

            if (localLabels->contains(operand))
                hex = localLabels->value(operand);
            else
                hex = globalLabels.value(operand);

        } else if (m == MODE_IMM && flags[i+1] & (FLAG_LOW_BYTE|FLAG_HIGH_BYTE)) {

            QMap<quint64,quint16> *map;
            char pref;
            if (flags[i+1] & FLAG_LOW_BYTE) {
                map = &s->lowbytes;
                pref = '<';
            } else {
                map = &s->highbytes;
                pref = '>';
            }
            quint16 addr = map->value(i+1);
            if (localLabels->contains(addr))
                hex = pref + QStringLiteral("%1").arg(localLabels->value(addr));
            else if (globalLabels.contains(addr))
                hex = pref + QStringLiteral("%1").arg(globalLabels.value(addr));
            else
                hex = pref + QStringLiteral("$%1").arg(addr, 4, 16, (QChar)'0');

        } else {
                hex   = QStringLiteral("$%1").arg(operand, 2, 16, (QChar)'0');
        }

        // build final string

        if (m != MODE_ZP_REL) {
            temps = QString(fmts[m]).arg(hex);
        } else {
            temps = QString(fmts[m]).arg(hex, hex2);
        }
    }
    dis = { start + i, distab[opcode].inst, temps, n,false, (quint64) opcode,
                                                (quint64) operand, 0 };
    if (m == MODE_REL || opcode == 0x4c || opcode == 0x6c || opcode == 0x20 ||
            opcode == 0x40 || opcode ==0x60) {
        dis.changes_pc = true;
    }
}

void Disassembler6502::trace(quint64 address) {
    struct segment *s = &segments[currentSegment];
    quint8 *data = s->data;
    quint8 *datatypes = s->datatypes;
    quint64 start = s->start;
    quint64 end = s->end;
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
            quint16 operand2 = 0;
            struct distabitem item = distab[opcode];
            auto m = (enum addressing_mode) item.mode;
            int n = isizes[m];

            if (address+n-1>end)
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
                if (m == MODE_ZP_REL) {
                    operand  = data[i+1];
                    operand2 = data[i+2];
                    operand2 = 3 + start + i + operand2 - (operand2>0x7f ? 0x100 : 0);
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

            if (m == MODE_ZP_REL && operand2 >= start && operand2 <=end) {
                if (!mark[operand2-start])
                    tracelist.append(operand2);
            }

            // rts, rti, jmp
            if (opcode == 0x60 || opcode == 0x40 || opcode == 0x4c)
                break;

            if (distab[opcode].inst == QStringLiteral("UNDEFINED")) {
                datatypes[i] = DT_UNDEFINED_CODE;
                break;
            }

            address += n;
        }

    }
    delete[] mark;
}

QString Disassembler6502::getDescriptionAt(quint64 address) {
    struct segment *s = &segments[currentSegment];
    quint64 start = s->start;
    quint8 *data = s->data;
    quint8 *datatypes = s->datatypes;
    QString instr;
    QString descr;
    QString action;
    QString flags;
    QString fulldescr;
    quint64 i;
    int x;

    i = address - start;

    if (datatypes[i] != DT_CODE)
        return QLatin1String("");

    quint8 opcode = data[i];
//    quint8 m = distab[opcode].mode;

    instr = distab[opcode].inst;

    // find instruction

    for (x=0; instruction_descriptions[x].descr; x++) {
        if (instruction_descriptions[x].instr == instr) {
            descr  = instruction_descriptions[x].descr;
            action = instruction_descriptions[x].action;
            flags  = instruction_descriptions[x].flags;
            qDebug() << descr;
            break;
        }
    }

    if (descr.isEmpty())
        return QLatin1String("");

    action = action.replace(QLatin1String(", "), QLatin1String("<br>"));
    action = action.replace(QLatin1String("M7"), QLatin1String("M<sub>7</sub>"));
    action = action.replace(QLatin1String("M6"), QLatin1String("M<sub>6</sub>"));
    action = action.replace(QLatin1String("M0"), QLatin1String("M<sub>0</sub>"));

    instr = instr.toUpper();

    fulldescr = "<h2 style=\"color: red\">" + instr + "</h2><h4>" + descr + "</h4>" +
                action + "<br><br><i style=\"color: blue\">flags: " + flags + "</i>";

    return fulldescr;
}
