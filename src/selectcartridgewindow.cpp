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
#include "loaderatari8bitcar.h"
#include "selectcartridgewindow.h"
#include "ui_selectcartridgewindow.h"
#include <QDebug>

selectcartridgewindow::selectcartridgewindow(QWidget *parent, const QVector<quint64> &candidates) :
    QDialog(parent),
    ui(new Ui::selectcartridgewindow)
{
    ui->setupUi(this);

    this->setFont(globalFont);

    QTableWidget *t = ui->tableCartridges;
    t->setColumnCount(1);

    for (auto a : candidates) {
        int row = t->rowCount();
        t->setRowCount(row+1);

        QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(a));
        t->setVerticalHeaderItem(row, item);

        item = new QTableWidgetItem(cartridges[a].description);
        t->setItem(row, 0, item);
    }

    t->setCurrentCell(0,0);
    t->setFocus();
}

selectcartridgewindow::~selectcartridgewindow()
{
    delete ui;
}

void selectcartridgewindow::accept() {
    QTableWidget *t = ui->tableCartridges;
    QList<QTableWidgetSelectionRange> ranges = t->selectedRanges();

    int row = ranges.at(0).topRow();

    cartridge_type = t->verticalHeaderItem(row)->text().toULongLong();

    setResult(QDialog::Accepted);
    hide();
}
