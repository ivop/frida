#ifndef FRIDA_H
#define FRIDA_H

#include <QApplication>
#include <QFile>
#include <QMap>
#include "disassembler.h"

// --------------------------------------------------------------------------

#define FRIDA_VERSION_MAJOR     0
#define FRIDA_VERSION_MINOR     9
#define FRIDA_VERSION_STRING    "0.9 beta"

// --------------------------------------------------------------------------

// Do change the numbers as they are saved in the project file(!)

enum cputypeid {
    CT_NMOS6502         = 0x00,
    CT_NMOS6502UNDEF    = 0x01,
    CT_CMOS65C02        = 0x02,
//    CT_Z80              = 0x10,       // example, not set in stone yet
//    CT_6800             = 0x20,
};

struct cputype {
    QString name;
    enum cputypeid id;
};

extern QList<struct cputype> cputypes;

// --------------------------------------------------------------------------

// leave space for other file types
enum filetypeid {
    FT_RAW_FILE             = 0x00,
    FT_ATARI8BIT_BINARY     = 0x10,
    FT_C64_BINARY           = 0x20,
    FT_C64_PSID             = 0x21,
    FT_ATARI2600_2K4K       = 0x30,
    FT_ORIC_TAP             = 0x40,
    FT_APPLE2_DOS33         = 0x50,
    FT_APPLE2_APPLESINGLE   = 0x51,
    FT_NES_SONG_FILE        = 0x60,
};

struct filetype {
    QString name;
    enum filetypeid id;
};

extern QList<struct filetype> filetypes;

// --------------------------------------------------------------------------

enum fonts {
    FONT_NORMAL = 0,
    FONT_ATARI8BIT,
    FONT_C64,
    FONT_LAST
};

extern enum fonts altfont;

// --------------------------------------------------------------------------

enum datatypes {
    DT_UNDEFINED_BYTES,
    DT_BYTES,
    DT_UNDEFINED_CODE,
    DT_CODE,
    DT_WORDLE,
    DT_WORDBE,
    DT_DWORDLE,
    DT_DWORDBE,
    DT_QWORDLE,
    DT_QWORDBE,
    DT_XWORDLE,
    DT_XWORDBE,
    DT_ASCII,
    DT_ATASCII,
    DT_PETSCII,
    DT_ANTIC_SCREEN,
    DT_CBM_SCREEN,
    DT_INVERSE_ATASCII,
    DT_INVERSE_ANTIC_SCREEN,
    DT_LAST
};

// --------------------------------------------------------------------------

struct disassembly {
    quint64 address;
    QString instruction;
    QString arguments;
    int size;           // number of bytes "consumed" incl. instruction
    bool changes_pc;
    quint64 opcode;
    quint64 operand_address;
    quint64 operand_offset;
};

// --------------------------------------------------------------------------

struct segment {
    quint64 start, end;
    QString name;
    quint8 *data;
    quint8 *datatypes;
    quint8 *flags;
    QMap<quint64, QString> comments;
    QMap<quint64, QString> localLabels;

// maps addresses of bytes to the address of which it is a low or high byte
    QMap<quint64, quint16> lowbytes;
    QMap<quint64, quint16> highbytes;

    QList<struct disassembly> disassembly;
    int scrollbarValue;
};

extern QList<struct segment> segments;      // currently in main.cpp
extern int currentSegment;
extern QMap<quint64, QString> autoLabels, userLabels;
extern quint32 cputype;
extern QString globalNotes;

// --------------------------------------------------------------------------

#define FLAG_USE_LABEL      0x01
#define FLAG_LOW_BYTE       0x02
#define FLAG_HIGH_BYTE      0x04

#endif // FRIDA_H
