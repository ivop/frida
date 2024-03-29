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

#include "frida.h"

QVector<struct filetype> filetypes = {
{ "Raw",                                            FT_RAW_FILE },
{ "Atari 8-bit Binary (.XEX)",                      FT_ATARI8BIT_BINARY },
{ "Atari 8-bit Slight Atari Player (.SAP)",         FT_ATARI8BIT_SAP },
{ "Atari 8-bit Cartridge (.CAR)" ,                  FT_ATARI8BIT_CAR },
{ "Commodore PET/VIC-20/C16/C64/C128 Binary (.PRG)",FT_C64_BINARY },
{ "Commodore C64 PSID/RSID (.SID)",                 FT_C64_PSID },
{ "Atari 2600 2K/4K ROM (.A26)",                    FT_ATARI2600_2K4K },
{ "Oric Tape File (.TAP)",                          FT_ORIC_TAP },
{ "Apple ][ DOS3.3 4-byte header",                  FT_APPLE2_DOS33 },
{ "Apple ][ ProDOS AppleSingle",                    FT_APPLE2_APPLESINGLE },
{ "Nintendo NES Song File (.NSF)",                  FT_NES_SONG_FILE },
{ "CP/M Binary at 0100H (.COM)",                    FT_CPM_BINARY },
{ "BBC Micro, Electron, Master UEF Tape (.UEF)",    FT_BBC_UEF_TAPE },
{ "ZX Spectrum Tape File (.TAP)",                   FT_ZX_SPECTRUM_TAP },
};
