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

#include "addlabelwindow.h"
#include "frida.h"
#include "ui_addlabelwindow.h"
#include <QDebug>
#include <QMessageBox>

addLabelWindow::addLabelWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addLabelWindow)
{
    ui->setupUi(this);
    this->setFont(globalFont);

    connect(ui->pushButtonCancel, &QPushButton::clicked,
            this, &addLabelWindow::onPushButtonCancel_clicked);
    connect(ui->pushButtonAdd, &QPushButton::clicked,
            this, &addLabelWindow::onPushButtonAdd_clicked);
    ui->lineEditAddress->setFocus();
}

addLabelWindow::~addLabelWindow()
{
    delete ui;
}

void addLabelWindow::onPushButtonCancel_clicked() {
    close();
}

void addLabelWindow::onPushButtonAdd_clicked() {
    QMessageBox msg;
    QString address = ui->lineEditAddress->text();
    QString label   = ui->lineEditLabel->text();
    bool ok = false;
    quint64 addr = address.toULongLong(&ok,16);

    msg.setFont(globalFont);

    if (!ok) {
        msg.setText(QStringLiteral("Invalid address!"));
        msg.exec();
    } else if (label.isEmpty()) {
        msg.setText(QStringLiteral("No label specified!"));
        msg.exec();
    } else {
        if (ui->checkBoxLocalLabel->isChecked()) {
            segments[currentSegment].localLabels.insert(addr, label);
        } else {
            globalLabels.insert(addr, label);
        }
        close();
    }
}
