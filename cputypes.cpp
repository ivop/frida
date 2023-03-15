#include <QApplication>
#include "frida.h"

QList<struct cputype> cputypes = {
{ "NMOS 6502",                          CT_NMOS6502 },
{ "NMOS 6502 w/ undefined opcodes",     CT_NMOS6502UNDEF },
//{ "CMOS 65C02",                         CT_CMOS65C02 }
};
