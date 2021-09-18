#include <QApplication>
#include "frida.h"

QList<struct filetype> filetypes = {
{ "Raw",                        FT_RAW_FILE },
{ "Atari 8-bit Binary (.XEX)",  FT_ATARI8BIT_BINARY },
{ "C64 Binary (.PRG)",          FT_C64_BINARY },
{ "C64 PSID",                   FT_C64_PSID },
};
