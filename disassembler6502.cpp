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
    MODE_IND_ZP,        // lda ($12)        65C02 only
    MODE_IND_ABS_X,     // jmp ($1234,x)    65C02 only
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
    [MODE_IND_ABS_X] = "(%1,x)"         // 65C02 only
};

static const char isizes[MODE_LAST] = {
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
    [MODE_IND_ABS_X] = 3                // 65C02 only
};

static const char can_be_label[MODE_LAST] = {
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
    [MODE_IND_ABS_X] = 1                // 65C02 only
};

struct distabitem {
    const char * const inst;
    const char mode;
};

#define UNDEFINED { "UNDEFINED", MODE_IMPL }

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

    { "brk", MODE_IMPL },       // $00
    { "ora", MODE_IND_X },      // $01
    UNDEFINED,                  // $02
    UNDEFINED,                  // $03
    { "tsb", MODE_ZP },                 // $04
    { "ora", MODE_ZP },         // $05
    { "asl", MODE_ZP },         // $06
    UNDEFINED,                  // $07
    { "php", MODE_IMPL },       // $08
    { "ora", MODE_IMM },        // $09
    { "asl", MODE_ACCU },       // $0a
    UNDEFINED,                  // $0b
    { "tsb", MODE_ABS },                // $0c
    { "ora", MODE_ABS },        // $0d
    { "asl", MODE_ABS },        // $0e
    UNDEFINED,                  // $0f

    { "bpl", MODE_REL },        // $10
    { "ora", MODE_IND_Y },      // $11
    { "ora", MODE_IND_ZP },             // $12
    UNDEFINED,                  // $13
    { "trb", MODE_ZP },                 // $14
    { "ora", MODE_ZP_X },       // $15
    { "asl", MODE_ZP_X },       // $16
    UNDEFINED,                  // $17
    { "clc", MODE_IMPL },       // $18
    { "ora", MODE_ABS_Y },      // $19
    { "inc", MODE_ACCU },               // $1a
    UNDEFINED,                  // $1b
    { "trb", MODE_ABS },                // $1c
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
    { "and", MODE_IND_ZP },             // $32
    UNDEFINED,                  // $33
    { "bit", MODE_ZP_X },               // $34
    { "and", MODE_ZP_X },       // $35
    { "rol", MODE_ZP_X },       // $36
    UNDEFINED,                  // $37
    { "sec", MODE_IMPL },       // $38
    { "and", MODE_ABS_Y },      // $39
    { "dec", MODE_ACCU },               // $3a
    UNDEFINED,                  // $3b
    { "bit", MODE_ABS_X },              // $3c
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
    { "eor", MODE_IND_ZP },             // $52
    UNDEFINED,                  // $53
    UNDEFINED,                  // $54
    { "eor", MODE_ZP_X },       // $55
    { "lsr", MODE_ZP_X },       // $56
    UNDEFINED,                  // $57
    { "cli", MODE_IMPL },       // $58
    { "eor", MODE_ABS_Y },      // $59
    { "phy", MODE_IMPL },               // $5a
    UNDEFINED,                  // $5b
    UNDEFINED,                  // $5c
    { "eor", MODE_ABS_X },      // $5d
    { "lsr", MODE_ABS_X },      // $5e
    UNDEFINED,                  // $5f

    { "rts", MODE_IMPL },       // $60
    { "adc", MODE_IND_X },      // $61
    UNDEFINED,                  // $62
    UNDEFINED,                  // $63
    { "stz", MODE_ZP },                 // $64
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
    { "adc", MODE_IND_ZP },             // $72
    UNDEFINED,                  // $73
    { "stz", MODE_ZP_X },               // $74
    { "adc", MODE_ZP_X },       // $75
    { "ror", MODE_ZP_X },       // $76
    UNDEFINED,                  // $77
    { "sei", MODE_IMPL },       // $78
    { "adc", MODE_ABS_Y },      // $79
    { "ply", MODE_IMPL },               // $7a
    UNDEFINED,                  // $7b
    { "jmp", MODE_IND_ABS_X },          // $7c
    { "adc", MODE_ABS_X },      // $7d
    { "ror", MODE_ABS_X },      // $7e
    UNDEFINED,                  // $7f

    { "bra", MODE_REL },                // $80
    { "sta", MODE_IND_X },      // $81
    UNDEFINED,                  // $82
    UNDEFINED,                  // $83
    { "sty", MODE_ZP },         // $84
    { "sta", MODE_ZP },         // $85
    { "stx", MODE_ZP },         // $86
    UNDEFINED,                  // $87
    { "dey", MODE_IMPL },       // $88
    { "bit", MODE_IMM },                // $89
    { "txa", MODE_IMPL },       // $8a
    UNDEFINED,                  // $8b
    { "sty", MODE_ABS },        // $8c
    { "sta", MODE_ABS },        // $8d
    { "stx", MODE_ABS },        // $8e
    UNDEFINED,                  // $8f

    { "bcc", MODE_REL },        // $90
    { "sta", MODE_IND_Y },      // $91
    { "sta", MODE_IND_ZP },             // $92
    UNDEFINED,                  // $93
    { "sty", MODE_ZP_X },       // $94
    { "sta", MODE_ZP_X },       // $95
    { "stx", MODE_ZP_Y },       // $96
    UNDEFINED,                  // $97
    { "tya", MODE_IMPL },       // $98
    { "sta", MODE_ABS_Y },      // $99
    { "txs", MODE_IMPL },       // $9a
    UNDEFINED,                  // $9b
    { "stz", MODE_ABS },                // $9c
    { "sta", MODE_ABS_X },      // $9d
    { "stz", MODE_ABS_X },              // $9e
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
    { "lda", MODE_IND_ZP },             // $b2
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
    { "cmp", MODE_IND_ZP },             // $d2
    UNDEFINED,                  // $d3
    UNDEFINED,                  // $d4
    { "cmp", MODE_ZP_X },       // $d5
    { "dec", MODE_ZP_X },       // $d6
    UNDEFINED,                  // $d7
    { "cld", MODE_IMPL },       // $d8
    { "cmp", MODE_ABS_Y },      // $d9
    { "phx", MODE_IMPL },               // $da
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
    { "sbc", MODE_IND_ZP },             // $f2
    UNDEFINED,                  // $f3
    UNDEFINED,                  // $f4
    { "sbc", MODE_ZP_X },       // $f5
    { "inc", MODE_ZP_X },       // $f6
    UNDEFINED,                  // $f7
    { "sed", MODE_IMPL },       // $f8
    { "sbc", MODE_ABS_Y },      // $f9
    { "plx", MODE_IMPL },               // $fa
    UNDEFINED,                  // $fb
    UNDEFINED,                  // $fc
    { "sbc", MODE_ABS_X },      // $fd
    { "inc", MODE_ABS_X },      // $fe
    UNDEFINED                   // $ff

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
            quint16 addr = map->value(i+1);
            if (localLabels->contains(addr))
                hex = pref + QString("%1").arg(localLabels->value(addr));
            else if (userLabels.contains(addr))
                hex = pref + QString("%1").arg(userLabels.value(addr));
            else if (autoLabels.contains(addr))
                hex = pref + QString("%1").arg(autoLabels.value(addr));
            else
                hex = pref + QString("$%1").arg(addr, 4, 16, (QChar)'0');
        } else {  // XXX add check for operand < 256 and MODE_IMM
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

    if (address < start || address > end) {
        delete[] mark;
        return;
    }

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

            qDebug() << address;

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
    delete[] mark;
}
