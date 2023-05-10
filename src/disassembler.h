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

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "pch.h"

extern class Disassembler *Disassembler;

class Disassembler {
public:
	Disassembler() = default;
    void generateDisassembly(bool generateLocalLabels);
    virtual void trace(quint64 address) = 0;
    virtual QString getDescriptionAt(quint64 address) = 0;

    QString hexPrefix, hexSuffix;
    quint64 cputype;
    bool toUpper;

protected:
    virtual void initTables(void) = 0;
    virtual int getInstructionSizeAt(quint64 relpos) = 0;
    virtual void createOperandLabels(quint64 relpos, bool generateLocalLabels) = 0;
    virtual void disassembleInstructionAt(quint64 relpos,
                                          struct disassembly &dis, int&n) = 0;
	Q_DISABLE_COPY(Disassembler)
};

class Disassembler6502 : public Disassembler {
public:
    void trace(quint64 address) override;
    QString getDescriptionAt(quint64 address) override;

protected:
    void initTables(void) override;
    int getInstructionSizeAt(quint64 relpos) override;
    void createOperandLabels(quint64 relpos, bool generateLocalLabels) override;
    void disassembleInstructionAt(quint64 relpos,
                                  struct disassembly &dis, int &n) override;
};

class Disassembler8080 : public Disassembler {
public:
    void trace(quint64 address) override;
    QString getDescriptionAt(quint64 address) override;

protected:
    void initTables(void) override;
    int getInstructionSizeAt(quint64 relpos) override;
    void createOperandLabels(quint64 relpos, bool generateLocalLabels) override;
    void disassembleInstructionAt(quint64 relpos,
                                  struct disassembly &dis, int &n) override;
};

class DisassemblerZ80 : public Disassembler {
public:
    void trace(quint64 address) override;
    QString getDescriptionAt(quint64 address) override;

protected:
    void initTables(void) override;
    int getInstructionSizeAt(quint64 relpos) override;
    void createOperandLabels(quint64 relpos, bool generateLocalLabels) override;
    void disassembleInstructionAt(quint64 relpos,
                                  struct disassembly &dis, int &n) override;
};

#endif // DISASSEMBLER_H
