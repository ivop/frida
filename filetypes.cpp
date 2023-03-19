#include <QApplication>
#include "frida.h"

QList<struct filetype> filetypes = {
{ "Raw",                                            FT_RAW_FILE },
{ "Atari 8-bit Binary (.XEX)",                      FT_ATARI8BIT_BINARY },
{ "Atari 8-bit Slight Atari Player (.SAP)",         FT_ATARI8BIT_SAP },
{ "Commodore C64 Binary (.PRG)",                    FT_C64_BINARY },
{ "Commodore C64 PSID/RSID (.SID)",                 FT_C64_PSID },
{ "Atari 2600 2K/4K ROM (.A26)",                    FT_ATARI2600_2K4K },
{ "Oric Tape File (.TAP)",                          FT_ORIC_TAP },
{ "Apple ][ DOS3.3 4-byte header",                  FT_APPLE2_DOS33 },
{ "Apple ][ ProDOS AppleSingle",                    FT_APPLE2_APPLESINGLE },
{ "Nintendo NES Song File (.NSF)",                  FT_NES_SONG_FILE },
};
