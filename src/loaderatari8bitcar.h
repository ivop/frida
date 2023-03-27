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

#ifndef LOADERATARI8BITCAR_H
#define LOADERATARI8BITCAR_H

#include <QString>

struct block {
    quint16 count;
    quint16 start_address;
    quint16 size;
    quint16 start_vector;
    quint16 init_vector;
};

struct cartridge_info {
    QString description;
    int size_in_kB;
    struct block *blocks;
};

enum cartridge_types {
	CARTRIDGE_NONE           =  0,
	CARTRIDGE_STD_8          =  1,
	CARTRIDGE_STD_16         =  2,
    CARTRIDGE_OSS_034M_16    =  3,      // broken 043M, see type 45
	CARTRIDGE_5200_32        =  4,
	CARTRIDGE_DB_32          =  5,
	CARTRIDGE_5200_EE_16     =  6,
	CARTRIDGE_5200_40        =  7,
	CARTRIDGE_WILL_64        =  8,
	CARTRIDGE_EXP_64         =  9,

	CARTRIDGE_DIAMOND_64     = 10,
	CARTRIDGE_SDX_64         = 11,
	CARTRIDGE_XEGS_32        = 12,
	CARTRIDGE_XEGS_07_64     = 13,
	CARTRIDGE_XEGS_128       = 14,
    CARTRIDGE_OSS_091M_16    = 15,      // 091M not M091
	CARTRIDGE_5200_NS_16     = 16,
	CARTRIDGE_ATRAX_DEC_128  = 17,
	CARTRIDGE_BBSB_40        = 18,
	CARTRIDGE_5200_8         = 19,

    CARTRIDGE_5200_4         = 20,
	CARTRIDGE_RIGHT_8        = 21,
	CARTRIDGE_WILL_32        = 22,
	CARTRIDGE_XEGS_256       = 23,
	CARTRIDGE_XEGS_512       = 24,
	CARTRIDGE_XEGS_1024      = 25,
	CARTRIDGE_MEGA_16        = 26,
	CARTRIDGE_MEGA_32        = 27,
	CARTRIDGE_MEGA_64        = 28,
	CARTRIDGE_MEGA_128       = 29,

    CARTRIDGE_MEGA_256       = 30,
	CARTRIDGE_MEGA_512       = 31,
	CARTRIDGE_MEGA_1024      = 32,
	CARTRIDGE_SWXEGS_32      = 33,
	CARTRIDGE_SWXEGS_64      = 34,
	CARTRIDGE_SWXEGS_128     = 35,
	CARTRIDGE_SWXEGS_256     = 36,
	CARTRIDGE_SWXEGS_512     = 37,
	CARTRIDGE_SWXEGS_1024    = 38,
	CARTRIDGE_PHOENIX_8      = 39,

    CARTRIDGE_BLIZZARD_16    = 40,
	CARTRIDGE_ATMAX_128      = 41,
	CARTRIDGE_ATMAX_OLD_1024 = 42,
	CARTRIDGE_SDX_128        = 43,
	CARTRIDGE_OSS_8          = 44,
	CARTRIDGE_OSS_043M_16    = 45,
	CARTRIDGE_BLIZZARD_4     = 46,
	CARTRIDGE_AST_32         = 47,
	CARTRIDGE_ATRAX_SDX_64   = 48,
	CARTRIDGE_ATRAX_SDX_128  = 49,

    CARTRIDGE_TURBOSOFT_64   = 50,
	CARTRIDGE_TURBOSOFT_128  = 51,
	CARTRIDGE_ULTRACART_32   = 52,
	CARTRIDGE_LOW_BANK_8     = 53,
	CARTRIDGE_SIC_128        = 54,
	CARTRIDGE_SIC_256        = 55,
	CARTRIDGE_SIC_512        = 56,
	CARTRIDGE_STD_2          = 57,
	CARTRIDGE_STD_4          = 58,
	CARTRIDGE_RIGHT_4        = 59,

    CARTRIDGE_BLIZZARD_32    = 60,
	CARTRIDGE_MEGAMAX_2048   = 61,
	CARTRIDGE_THECART_128M   = 62,
	CARTRIDGE_MEGA_4096      = 63,
	CARTRIDGE_MEGA_2048      = 64,
	CARTRIDGE_THECART_32M    = 65,
	CARTRIDGE_THECART_64M    = 66,
	CARTRIDGE_XEGS_8F_64     = 67,
	CARTRIDGE_ATRAX_128      = 68,
	CARTRIDGE_ADAWLIAH_32    = 69,

    CARTRIDGE_ADAWLIAH_64    = 70,
	CARTRIDGE_5200_SUPER_64  = 71,
	CARTRIDGE_5200_SUPER_128 = 72,
	CARTRIDGE_5200_SUPER_256 = 73,
	CARTRIDGE_5200_SUPER_512 = 74,
	CARTRIDGE_ATMAX_NEW_1024 = 75,
    CARTRIDGE_LAST           = 76
};

#endif
