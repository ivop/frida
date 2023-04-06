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

#include "changesegmentwindow.h"
#include "ui_changesegmentwindow.h"

changesegmentwindow::changesegmentwindow(quint64 oldaddr, quint64 *newaddr,
                                         QWidget *parent) :
    QDialog(parent),
    ui(new Ui::changesegmentwindow)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
        this, &changesegmentwindow::onButtonBox_accepted);

    QString hex = QStringLiteral("%1").arg(oldaddr, 4, 16, QChar('0'));
    ui->oldAddress->setText(hex);
    newaddress = newaddr;
    ui->newAddress->setText(hex);
    ui->newAddress->setFocus();
}

changesegmentwindow::~changesegmentwindow()
{
    delete ui;
}

void changesegmentwindow::onButtonBox_accepted()
{
    bool ok = false;
    quint64 n = ui->newAddress->text().toULongLong(&ok, 16);
    if (ok) {
        *newaddress = n;
    } else {
        QMessageBox msg;
        msg.setText(QStringLiteral("Invalid address!"));
        msg.setStandardButtons(QMessageBox::Close);
        msg.exec();
        ui->newAddress->setText(ui->oldAddress->text());
    }
}
