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

#include "lowandhighbytepairswindow.h"
#include "ui_lowandhighbytepairswindow.h"

LowAndHighBytePairsWindow::LowAndHighBytePairsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LowAndHighBytePairsWindow)
{
    ui->setupUi(this);

    connect(ui->radioNoLabels, &QRadioButton::toggled,
            this, &LowAndHighBytePairsWindow::onNoLabels_toggled);
}

LowAndHighBytePairsWindow::~LowAndHighBytePairsWindow()
{
    delete ui;
}

void LowAndHighBytePairsWindow::accept() {
    pairsLowLowHighHigh = ui->radioLowLowHighHigh->isChecked();
    minusLabels = ui->checkMinusOne->isChecked();

    if (ui->radioNoLabels->isChecked())
        generateLabels = NoLabels;
    else if (ui->radioLocalLabels->isChecked())
        generateLabels = LocalLabels;
    else
        generateLabels = GlobalLabels;

    setResult(QDialog::Accepted);
    hide();
}

void LowAndHighBytePairsWindow::onNoLabels_toggled() {
    if (ui->radioNoLabels->isChecked())
        ui->checkMinusOne->setHidden(true);
    else
        ui->checkMinusOne->setHidden(false);
}
