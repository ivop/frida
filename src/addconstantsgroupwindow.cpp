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

#include "addconstantsgroupwindow.h"
#include "ui_addconstantsgroupwindow.h"
#include "frida.h"

addConstantsGroupWindow::addConstantsGroupWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addConstantsGroupWindow)
{
    ui->setupUi(this);
}

addConstantsGroupWindow::~addConstantsGroupWindow()
{
    delete ui;
}

void addConstantsGroupWindow::accept() {
    QLineEdit *le = ui->lineEdit;

    QString groupName = le->text();

    struct constantsGroup *group = new struct constantsGroup;
    group->name = groupName;
    group->map = new QMap<quint64, QString>;

    constantsGroups.insert(nextNewGroup, *group);

    nextNewGroup++;

    setResult(QDialog::Accepted);
    hide();
}
