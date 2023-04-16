#include "pch.h"
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

#ifndef EXPORTASSEMBLYWINDOW_H
#define EXPORTASSEMBLYWINDOW_H

namespace Ui {
class exportAssemblyWindow;
}

class exportAssemblyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit exportAssemblyWindow(QWidget *parent = nullptr);
    ~exportAssemblyWindow();


    int asm_format{};

public Q_SLOTS:
    void accept() override;

private:
    Ui::exportAssemblyWindow *ui;
};

enum {
    ASM_FORMAT_VERBATIM = 0,
    ASM_FORMAT_MADS
};

#endif // EXPORTASSEMBLYWINDOW_H
