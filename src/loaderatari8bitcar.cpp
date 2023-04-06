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

// Based on: https://github.com/atari800/atari800/blob/master/DOC/cart.txt

#include "loaderatari8bitcar.h"
#include "loaders.h"
#include "selectcartridgewindow.h"

#define END { 0, 0, 0, 0, 0 }

// Implemented types
// The blocks describe the ROM layout, not necessarily the same banking scheme

// Type 1: Standard 8 kB left cartridge

struct block blocks_CARTRIDGE_STD_8[] = {
{ 1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 2: Standard 16 kB cartridge
// Type 26: MegaCart 16 kB cartridge

struct block blocks_CARTRIDGE_STD_16[] = {
{ 1, 0x8000, 0x4000, 0xbffa, 0xbffe },
END
};

// Type 44: OSS 8 kB Cartridge

struct block blocks_CARTRIDGE_OSS_8[] = {
{ 1, 0xb000, 0x1000, 0xbffa, 0xbff3 },
{ 1, 0xa000, 0x1000, 0, 0 },
END
};

// Type 3: OSS two chip 16 kB cartridge (034M) (wrong order!)
// Type 15: OSS one chip 16 kB cartridge (091M)
// Type 45: OSS two chip 16 kB cartridge (043M)

struct block blocks_CARTRIDGE_OSS_16[] = {
{ 3, 0xa000, 0x1000, 0, 0 },
{ 1, 0xb000, 0x1000, 0xbffa, 0xbffe },
END
};

// Type 4: Standard 32 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_32[] = {
{ 1, 0x4000, 0x8000, 0xbffe, 0 },
END
};

// Type 6: Two chip 16 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_EE_16[] = {
{ 1, 0x6000, 0x2000, 0, 0 },            // also at 0x4000
{ 1, 0xa000, 0x2000, 0xbbfe, 0 },       // also at 0x8000
END
};

// Type 7: Bounty Bob Strikes Back 40 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_40[] = {
{ 4, 0x4000, 0x1000, 0, 0 },
{ 4, 0x5000, 0x1000, 0, 0 },
{ 1, 0xa000, 0x2000, 0xbbfe, 0 },       // also visible at 0x8000
END
};

// Type 8: 64 kB Williams cartridge
// Type 9: Express 64 kB cartridge
// Type 10: Diamond 64 kB cartridge
// Type 11: SpartaDOS X 64 KB cartridge
// Type 50: Turbosoft 64 kB cartridge
// Type 70: aDawliah 64 KB cartridge

struct block blocks_CARTRIDGE_WILL_64[] = {
{ 1, 0xa000, 0x2000, 0xbffa, 0xbffe },      // assume bank 0 at start up
{ 7, 0xa000, 0x2000, 0, 0 },
END
};

// Type 5: DB 32 kB cartridge
// Type 12: XEGS 32 kB cartridge
// Type 33: Switchable XEGS 32 kB cartridge

struct block blocks_CARTRIDGE_XEGS_32[] = {
{ 3, 0x8000, 0x2000, 0, 0 },
{ 1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 13: XEGS 64 kB cartridge
// Type 34: Switchable XEGS 64 kB cartridge

struct block blocks_CARTRIDGE_XEGS_07_64[] = {
{ 7, 0x8000, 0x2000, 0, 0 },
{ 1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 14: XEGS 128 kB cartridge
// Type 35: Switchable XEGS 128 kB cartridge

struct block blocks_CARTRIDGE_XEGS_128[] = {
{ 15, 0x8000, 0x2000, 0, 0 },
{  1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 16: One chip 16 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_NS_16[] = {
{ 1, 0x8000, 0x4000, 0xbffe, 0 },
END
};

// Type 17: Decoded Atrax 128 kB cartridge

struct block blocks_CARTRIDGE_ATRAX_DEC_128[] = {
{ 15, 0xa000, 0x2000, 0, 0 },
{  1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 18: Bounty Bob Strikes Back 40 kB cartridge

struct block blocks_CARTRIDGE_BBSB_40[] = {
{ 4, 0x8000, 0x1000, 0, 0 },
{ 4, 0x9000, 0x1000, 0, 0 },
{ 1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 19: Standard 8 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_8[] = {
{ 1, 0xa000, 0x2000, 0xbffe, 0 },
END
};

// Type 20: Standard 4 KB 5200 cartridge

struct block blocks_CARTRIDGE_5200_4[] = {
{ 1, 0xb000, 0x1000, 0xbffe, 0 },
END
};

// Type 21: Standard 8 kB right cartridge
// Type 53: Low bank 8 kB cartridge

struct block blocks_CARTRIDGE_RIGHT_8[] = {
{ 1, 0x8000, 0x2000, 0x9ffa, 0x9ffe },
END
};

// Type 22: 32 kB Williams cartridge
// Type 52: Ultracart 32 kB cartridge
// Type 60: Blizzard 32 kB cartridge
// Type 69: aDawliah 32 KB cartridge

struct block blocks_CARTRIDGE_WILL_32[] = {
{ 1, 0xa000, 0x2000, 0xbffa, 0xbffe },      // assume bank 0 at startup
{ 3, 0xa000, 0x2000, 0, 0 },
END
};

// Type 23: XEGS 256 kB cartridge
// Type 36: Switchable XEGS 256 kB cartridge

struct block blocks_CARTRIDGE_XEGS_256[] = {
{ 31, 0x8000, 0x2000, 0, 0 },
{  1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 24: XEGS 512 kB cartridge
// Type 37: Switchable XEGS 512 kB cartridge

struct block blocks_CARTRIDGE_XEGS_512[] = {
{ 63, 0x8000, 0x2000, 0, 0 },
{  1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 25: XEGS 1024 kB cartridge
// Type 38: Switchable XEGS 1024 kB cartridge

struct block blocks_CARTRIDGE_XEGS_1024[] = {
{ 127, 0x8000, 0x2000, 0, 0 },
{   1, 0xa000, 0x2000, 0xbffa, 0xbffe },
END
};

// Type 27: MegaCart 32 kB cartridge

struct block blocks_CARTRIDGE_MEGA_32[] = {
{ 1, 0x8000, 0x4000, 0xbffa, 0xbffe },          // assume we start with bank 0
{ 1, 0x8000, 0x4000, 0, 0 },
END
};

// Type 28: MegaCart 64 kB cartridge

struct block blocks_CARTRIDGE_MEGA_64[] = {
{ 1, 0x8000, 0x4000, 0xbffa, 0xbffe },          // assume we start with bank 0
{ 3, 0x8000, 0x4000, 0, 0 },
END
};

// Type 29: MegaCart 128 kB cartridge
// Type 54: SIC! 128 kB cartridge

struct block blocks_CARTRIDGE_MEGA_128[] = {
{ 1, 0x8000, 0x4000, 0xbffa, 0xbffe },          // assume we start with bank 0
{ 7, 0x8000, 0x4000, 0, 0 },
END
};

// Type 30: MegaCart 256 kB cartridge
// Type 55: SIC! 256 kB cartridge

struct block blocks_CARTRIDGE_MEGA_256[] = {
{  1, 0x8000, 0x4000, 0xbffa, 0xbffe },          // assume we start with bank 0
{ 15, 0x8000, 0x4000, 0, 0 },
END
};

// Type 31: MegaCart 512 kB cartridge
// Type 56: SIC! 512 kB cartridge

struct block blocks_CARTRIDGE_MEGA_512[] = {
{  1, 0x8000, 0x4000, 0xbffa, 0xbffe },          // assume we start with bank 0
{ 31, 0x8000, 0x4000, 0, 0 },
END
};

// Type 32: MegaCart 1 MB cartridge

struct block blocks_CARTRIDGE_MEGA_1024[] = {
{  1, 0x8000, 0x4000, 0xbffa, 0xbffe },          // assume we start with bank 0
{ 63, 0x8000, 0x4000, 0, 0 },
END
};

// Type 41: Atarimax 128 kB Flash cartridge

struct block blocks_CARTRIDGE_ATMAX_128[] = {
{ 15, 0xa000, 0x2000, 0, 0 },
{  1, 0xa000, 0x2000, 0xbffa, 0xbffe },         // assume bank $0f on start up
END
};

// Type 42: Atarimax 1 MB Flash cartridge (old)

struct block blocks_CARTRIDGE_ATMAX_OLD_1024[] = {
{ 127, 0xa000, 0x2000, 0, 0 },
{   1, 0xa000, 0x2000, 0xbffa, 0xbffe },        // old starts with bank $7f
END
};

// Type 75: Atarimax 1 MB Flash cartridge (new)

struct block blocks_CARTRIDGE_ATMAX_NEW_1024[] = {
{   1, 0xa000, 0x2000, 0xbffa, 0xbffe },        // new starts with bank $00
{ 127, 0xa000, 0x2000, 0, 0 },
END
};

// Type 43: SpartaDOS X 128 kB cartridge

struct block blocks_CARTRIDGE_SDX_128[] = {
{  1, 0xa000, 0x2000, 0xbffa, 0xbffe },         // assume bank $00 on start up
{ 15, 0xa000, 0x2000, 0, 0 },
END
};

// Type 46: Blizzard 4 kB cartridge

struct block blocks_CARTRIDGE_BLIZZARD_4[] = {
{ 1, 0xb000, 0x1000, 0xbffa, 0xbffe },
END
};

// Type 51: Turbosoft 128 kB cartridge

struct block blocks_CARTRIDGE_TURBOSOFT_128[] = {
{  1, 0xa000, 0x2000, 0xbffa, 0xbff3 },
{ 15, 0xa000, 0x2000, 0, 0 },
END
};

// Type 47: AST 32 kB cartridge
// This one is weird. Every bank is 256 bytes and repeated 32 times between
// $a000-$bfff, and is also visible at $d500-$d5ff
// first bank contains code to setup a driver or something and then disables
// $a000-$bfff access and uses only the $d500 window to access some sort of
// ROM disk

struct block blocks_CARTRIDGE_AST_32[] = {
{   1, 0xbf00, 0x0100, 0xbffa, 0xbffe },
{ 127, 0xd500, 0x0100, 0, 0 },
END
};

// Type 48: Atrax SDX 64 kB cartridge   [SKIPPED see Type 11]
// Type 49: Atrax SDX 128 kB cartridge  [SKIPPED see Type 43]

// Type 57: Standard 2 kB cartridge

struct block blocks_CARTRIDGE_STD_2[] = {
{ 1, 0xb800, 0x0800, 0xbffa, 0xbffe },
END
};

// Type 58: Standard 4 kB cartridge

struct block blocks_CARTRIDGE_STD_4[] = {
{ 1, 0xb000, 0x1000, 0xbffa, 0xbffe },
END
};

// Type 59: Right slot 4 KB cartridge

struct block blocks_CARTRIDGE_RIGHT_4[] = {
{ 1, 0x9000, 0x1000, 0x9ffa, 0x9ffe },
END
};

// Type 61: MegaMax 2 MB cartridge

struct block blocks_CARTRIDGE_MEGAMAX_2048[] = {
{   1, 0x8000, 0x4000, 0xbffa, 0xbffe },
{ 127, 0x8000, 0x4000, 0, 0 },
END
};

// Type 62: The!Cart 128 MB cartridge   [SKIPPED it emulates other cartridges]

// Type 63: Flash MegaCart 4 MB cartridge
// Note that bank 255 is not available

struct block blocks_CARTRIDGE_MEGA_4096[] = {
{ 254, 0x8000, 0x4000, 0, 0 },
{   1, 0x8000, 0x4000, 0xbffa, 0xbffe },    // start at bank 254
END
};

// Type 64: MegaCart 2 MB cartridge

struct block blocks_CARTRIDGE_MEGA_2048[] = {
{   1, 0x8000, 0x4000, 0xbffa, 0xbffe },    // assume we start at bank 0
{ 127, 0x8000, 0x4000, 0, 0 },
END
};

// Type 65: The!Cart 32 MB cartridge    [SKIPPED it emulates other cartridges]
// Type 66: The!Cart 64 MB cartridge    [SKIPPED it emulates other cartridges]

// Type 68: Atrax 128 kB cartridge      [SKIPPED see Type 17]

// Type 71: Super Cart 64 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_SUPER_64[] = {
{ 1, 0x4000, 0x8000, 0, 0 },
{ 1, 0x4000, 0x8000, 0xbffe, 0 },
END
};

// Type 72: Super Cart 128 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_SUPER_128[] = {
{ 3, 0x4000, 0x8000, 0, 0 },
{ 1, 0x4000, 0x8000, 0xbffe, 0 },
END
};

// Type 73: Super Cart 256 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_SUPER_256[] = {
{ 7, 0x4000, 0x8000, 0, 0 },
{ 1, 0x4000, 0x8000, 0xbffe, 0 },
END
};

// Type 74: Super Cart 512 kB 5200 cartridge

struct block blocks_CARTRIDGE_5200_SUPER_512[] = {
{ 7, 0x4000, 0x8000, 0, 0 },
{ 1, 0x4000, 0x8000, 0xbffe, 0 },
END
};

// ---------------------------------------------------------------------------

struct cartridge_info cartridges[CARTRIDGE_LAST] = {

// 0-9
{ "None",                                   0, 0 },
{ "Standard 8 kB left cartridge",           8, blocks_CARTRIDGE_STD_8      },
{ "Standard 16 kB cartridge",              16, blocks_CARTRIDGE_STD_16     },
{ "OSS two chip 16 kB cartridge (034M)",   16, blocks_CARTRIDGE_OSS_16     },
{ "Standard 32 kB 5200 cartridge",         32, blocks_CARTRIDGE_5200_32    },
{ "DB 32 kB cartridge",                    32, blocks_CARTRIDGE_XEGS_32    },
{ "Two chip 16 kB 5200 cartridge",         16, blocks_CARTRIDGE_5200_EE_16 },
{ "Bounty Bob 40 kB 5200 cartridge",       40, blocks_CARTRIDGE_5200_40    },
{ "64 kB Williams cartridge",              64, blocks_CARTRIDGE_WILL_64    },
{ "Express 64 kB cartridge",               64, blocks_CARTRIDGE_WILL_64    },

// 10-19
{ "Diamond 64 kB cartridge",               64, blocks_CARTRIDGE_WILL_64       },
{ "SpartaDOS X 64 kB cartridge",           64, blocks_CARTRIDGE_WILL_64       },
{ "XEGS 32 kB cartridge",                  32, blocks_CARTRIDGE_XEGS_32       },
{ "XEGS 64 kB cartridge (banks 0-7)",      64, blocks_CARTRIDGE_XEGS_07_64    },
{ "XEGS 128 kB cartridge",                128, blocks_CARTRIDGE_XEGS_128      },
{ "OSS one chip 16 kB cartridge",          16, blocks_CARTRIDGE_OSS_16        },
{ "One chip 16 kB 5200 cartridge",         16, blocks_CARTRIDGE_5200_NS_16    },
{ "Decoded Atrax 128 kB cartridge",       128, blocks_CARTRIDGE_ATRAX_DEC_128 },
{ "Bounty Bob 40 kB cartridge",            40, blocks_CARTRIDGE_BBSB_40       },
{ "Standard 8 kB 5200 cartridge",           8, blocks_CARTRIDGE_5200_8        },

// 20-29
{ "Standard 4 kB 5200 cartridge",           4, blocks_CARTRIDGE_5200_4    },
{ "Standard 8 kB right cartridge",          8, blocks_CARTRIDGE_RIGHT_8   },
{ "32 kB Williams cartridge",              32, blocks_CARTRIDGE_WILL_32   },
{ "XEGS 256 kB cartridge",                256, blocks_CARTRIDGE_XEGS_256  },
{ "XEGS 512 kB cartridge",                512, blocks_CARTRIDGE_XEGS_512  },
{ "XEGS 1 MB cartridge",                 1024, blocks_CARTRIDGE_XEGS_1024 },
{ "MegaCart 16 kB cartridge",              16, blocks_CARTRIDGE_STD_16    },
{ "MegaCart 32 kB cartridge",              32, blocks_CARTRIDGE_MEGA_32   },
{ "MegaCart 64 kB cartridge",              64, blocks_CARTRIDGE_MEGA_64   },
{ "MegaCart 128 kB cartridge",            128, blocks_CARTRIDGE_MEGA_128  },

// 30-39
{ "MegaCart 256 kB cartridge",            256, blocks_CARTRIDGE_MEGA_256   },
{ "MegaCart 512 kB cartridge",            512, blocks_CARTRIDGE_MEGA_512   },
{ "MegaCart 1 MB cartridge",             1024, blocks_CARTRIDGE_MEGA_1024  },
{ "Switchable XEGS 32 kB cartridge",       32, blocks_CARTRIDGE_XEGS_32    },
{ "Switchable XEGS 64 kB cartridge",       64, blocks_CARTRIDGE_XEGS_07_64 },
{ "Switchable XEGS 128 kB cartridge",     128, blocks_CARTRIDGE_XEGS_128   },
{ "Switchable XEGS 256 kB cartridge",     256, blocks_CARTRIDGE_XEGS_256   },
{ "Switchable XEGS 512 kB cartridge",     512, blocks_CARTRIDGE_XEGS_512   },
{ "Switchable XEGS 1 MB cartridge",      1024, blocks_CARTRIDGE_XEGS_256   },
{ "Phoenix 8 KB cartridge",                 8, blocks_CARTRIDGE_STD_8      },

// 40-49
{ "Blizzard 16 kB cartridge",              16, blocks_CARTRIDGE_STD_16        },
{ "Atarimax 128 kB Flash cartridge",      128, blocks_CARTRIDGE_ATMAX_128     },
{ "Atarimax 1 MB Flash cartridge (old)", 1024, blocks_CARTRIDGE_ATMAX_OLD_1024},
{ "SpartaDOS X 128 kB cartridge",         128, blocks_CARTRIDGE_SDX_128       },
{ "OSS 8 KB cartridge",                     8, blocks_CARTRIDGE_OSS_8         },
{ "OSS two chip 16 kB cartridge (043M)",   16, blocks_CARTRIDGE_OSS_16        },
{ "Blizzard 4 kB cartridge",                4, blocks_CARTRIDGE_BLIZZARD_4    },
{ "AST 32 kB cartridge",                   32, blocks_CARTRIDGE_AST_32        },
{ "Atrax SDX 64 kB cartridge",             64, 0 },
{ "Atrax SDX 128 kB cartridge",           128, 0 },

// 50-59
{ "Turbosoft 64 kB cartridge",             64, blocks_CARTRIDGE_WILL_64       },
{ "Turbosoft 128 kB cartridge",           128, blocks_CARTRIDGE_TURBOSOFT_128 },
{ "Ultracart 32 kB cartridge",             32, blocks_CARTRIDGE_WILL_32       },
{ "Low bank 8 kB cartridge",                8, blocks_CARTRIDGE_RIGHT_8       },
{ "SIC! 128 kB cartridge",                128, blocks_CARTRIDGE_MEGA_128      },
{ "SIC! 256 kB cartridge",                256, blocks_CARTRIDGE_MEGA_256      },
{ "SIC! 512 kB cartridge",                512, blocks_CARTRIDGE_MEGA_512      },
{ "Standard 2 kB cartridge",                2, blocks_CARTRIDGE_STD_2         },
{ "Standard 4 kB cartridge",                4, blocks_CARTRIDGE_STD_4         },
{ "Right slot 4 kB cartridge",              4, blocks_CARTRIDGE_RIGHT_4       },

// 60-69
{ "Blizzard 32 kB cartridge",              32, blocks_CARTRIDGE_WILL_32      },
{ "MegaMax 2 MB cartridge",              2048, blocks_CARTRIDGE_MEGAMAX_2048 },
{ "The!Cart 128 MB cartridge",         131072, 0 },
{ "Flash MegaCart 4 MB cartridge",       4096, blocks_CARTRIDGE_MEGA_4096    },
{ "MegaCart 2 MB cartridge",             2048, blocks_CARTRIDGE_MEGA_2048    },
{ "The!Cart 32 MB cartridge",           32768, 0 },
{ "The!Cart 64 MB cartridge",           65536, 0 },
{ "XEGS 64 kB cartridge (banks 8-15)",     64, blocks_CARTRIDGE_XEGS_07_64   },
{ "Atrax 128 kB cartridge",               128, 0 },
{ "aDawliah 32 kB cartridge",              32, blocks_CARTRIDGE_WILL_32      },

// 70-75
{ "aDawliah 64 kB cartridge",              64, blocks_CARTRIDGE_WILL_64       },
{ "Super Cart 64 kB 5200 cartridge",       64, blocks_CARTRIDGE_5200_SUPER_64 },
{ "Super Cart 128 kB 5200 cartridge",     128, blocks_CARTRIDGE_5200_SUPER_128},
{ "Super Cart 256 kB 5200 cartridge",     256, blocks_CARTRIDGE_5200_SUPER_256},
{ "Super Cart 512 kB 5200 cartridge",     512, blocks_CARTRIDGE_5200_SUPER_512},
{ "Atarimax 1 MB Flash cartridge (new)", 1024, blocks_CARTRIDGE_ATMAX_NEW_1024}
};

// ---------------------------------------------------------------------------

bool LoaderAtari8bitCar::Load(QFile& file) {
    quint8 header[16];
    quint32 cartype;
    struct segment s;
    struct cartridge_info *carinfo;
    struct block *blocks;

    file.read((char *)header, 16);

    if (memcmp(header, "CART", 4) != 0) {
        file.seek(0);
        qint64 filesize = file.size();

        QVector<quint64> candidates;

        for (quint64 a = 1; a < CARTRIDGE_LAST; a++) {
            if (!cartridges[a].blocks)
                continue;
            if (cartridges[a].size_in_kB*1024 == filesize)
                candidates.append(a);
        }

        if (candidates.isEmpty()) {
            this->error_message = QStringLiteral("Binary file does not match size of any cartridges.");
            return false;
        }

        selectcartridgewindow scw(nullptr, candidates);

        if (scw.exec() == QDialog::Rejected) {
            this->error_message = QStringLiteral("Loading bin file rejected.");
            return false;
        }

        cartype = scw.cartridge_type;

        file.seek(0);

    } else {

        cartype = BE32(header+4);

    }

    if (cartype == 0 || cartype >= CARTRIDGE_LAST) {
        this->error_message = QStringLiteral("Unknown mapper type\n");
        return false;
    }

    carinfo = &cartridges[cartype];
    blocks = carinfo->blocks;

    if (blocks == 0) {
        this->error_message = QStringLiteral("Type ") + carinfo->description +
                              QStringLiteral("not supported");
        return false;
    }

    int bank = 0;

    for (int i = 0; blocks[i].count; i++) {

        quint16 start_address = blocks[i].start_address;
        quint16 size = blocks[i].size;
        quint16 end_address = start_address + size - 1;

        for (int j = 0; j < blocks[i].count; j++) {
            s = createEmptySegment(start_address, end_address);
            s.name = QStringLiteral("Bank %1").arg(bank++);
            if (file.read((char *) s.data, size) != size) {
                this->error_message = QStringLiteral("Premature end of file reached.");
                return false;
            }

            if (blocks[i].start_vector) {
                quint16 start_offset = blocks[i].start_vector - blocks[i].start_address;
                quint16 start_vector = LE16(s.data + start_offset);

                s.localLabels.insert(start_vector, QStringLiteral("start"));
                s.datatypes[start_offset]   =
                s.datatypes[start_offset+1] = DT_WORDLE;
                s.flags[start_offset] = FLAG_USE_LABEL;
            }
            if (blocks[i].init_vector) {
                quint16 init_offset  = blocks[i].init_vector  - blocks[i].start_address;
                quint16 init_vector  = LE16(s.data + init_offset);

                s.localLabels.insert(init_vector, QStringLiteral("init"));
                s.datatypes[init_offset]   =
                s.datatypes[init_offset+1] = DT_WORDLE;
                s.flags[init_offset] = FLAG_USE_LABEL;
            }

            segments.append(s);
        }
    }

    return true;
}
