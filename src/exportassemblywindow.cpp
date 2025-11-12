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

#include "exportassemblywindow.h"
#include "ui_exportassemblywindow.h"

exportAssemblyWindow::exportAssemblyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::exportAssemblyWindow)
{
    ui->setupUi(this);
}

exportAssemblyWindow::~exportAssemblyWindow()
{
    delete ui;
}

void exportAssemblyWindow::accept() {
    asm_format = ASM_FORMAT_VERBATIM;

    if (ui->mads_assembler->isChecked())
        asm_format = ASM_FORMAT_MADS;
    else if (ui->ca65_assembler->isChecked())
        asm_format = ASM_FORMAT_CA65;

    setResult(QDialog::Accepted);
    hide();
}
