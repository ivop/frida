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

#include "lowhighbytewindow.h"
#include "ui_lowhighbytewindow.h"
#include <QDebug>
#include <QMessageBox>

lowhighbytewindow::lowhighbytewindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lowhighbytewindow)
{
    ui->setupUi(this);
}

lowhighbytewindow::lowhighbytewindow(quint64 location, bool low, quint8 byte,
                  quint16 *full, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lowhighbytewindow)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &lowhighbytewindow::onButtonBox_accepted);

    ui->location->setText(QString("%1").arg(location, 4, 16, (QChar)'0'));
    if (low)
        ui->lowhighbyteLabel->setText(QString("Low Byte"));
    else
        ui->lowhighbyteLabel->setText(QString("High Byte"));
    ui->lowhighbyte->setText(QString("%1").arg(byte, 2, 16, (QChar)'0'));
    fulladdr = full;

    ui->fullAddress->setFocus();
}

lowhighbytewindow::~lowhighbytewindow()
{
    delete ui;
}

void lowhighbytewindow::onButtonBox_accepted()
{
    bool ok;
    quint64 addr = ui->fullAddress->text().toULongLong(&ok, 16);

    if (ok && addr <= 0xffff) {
        *fulladdr = addr;
        accept();
    } else {
        QMessageBox msg;
        if (addr > 0xffff)
            msg.setText("Address larger than 16-bit");
        else
            msg.setText("Invalid address");
        msg.exec();
    }
}
