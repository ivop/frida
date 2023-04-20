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

#include "selectconstantsgoupwindow.h"
#include "ui_selectconstantsgoupwindow.h"

selectConstantsGoupWindow::selectConstantsGoupWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::selectConstantsGoupWindow)
{
    ui->setupUi(this);

    QTableWidget *t = ui->tableGroups;
    QTableWidgetItem *item;
    qint64 row = 0;

    t->setRowCount(0);

    QMap<quint64, struct constantsGroup>::iterator iter;

    for (iter = constantsGroups.begin(); iter != constantsGroups.end(); ++iter) {
        row = t->rowCount();
        t->setRowCount(row+1);

        quint64 groupID = iter.key();
        QString groupName = iter.value().name;

        QString vhi = QStringLiteral("%1").arg(groupID);
        t->setVerticalHeaderItem(row, new QTableWidgetItem(vhi));
        t->verticalHeaderItem(row)->setTextAlignment(Qt::AlignTop | Qt::AlignRight);

        item = new QTableWidgetItem(groupName);
        item->setTextAlignment(Qt::AlignTop);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(row, 0, item);
    }
    if (!constantsGroups.isEmpty()) {
        t->setCurrentCell(0, 0);
        t->setFocus();
    }
}

selectConstantsGoupWindow::~selectConstantsGoupWindow()
{
    delete ui;
}

void selectConstantsGoupWindow::accept() {
    QTableWidget *t = ui->tableGroups;
    QList<QTableWidgetSelectionRange> ranges = t->selectedRanges();

    if (!ranges.isEmpty()) {
        QTableWidgetSelectionRange r = ranges.first();
        qint64 row = r.topRow();

        groupID = t->verticalHeaderItem(row)->text().toULongLong();
    }

    setResult(QDialog::Accepted);
    hide();
}
