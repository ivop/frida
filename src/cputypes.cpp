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
#include <QApplication>

const QVector<struct cputype> cputypes = {
{ "NMOS 6502",                          CT_NMOS6502 },
{ "NMOS 6502 w/ undefined opcodes",     CT_NMOS6502UNDEF },
{ "CMOS 65C02",                         CT_CMOS65C02 },
{ "Intel 8080",                         CT_INTEL_8080 },
};
