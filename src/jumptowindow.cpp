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

#include "jumptowindow.h"
#include "frida.h"
#include "ui_jumptowindow.h"
#include <QDebug>

jumpToWindow::jumpToWindow(QWidget *parent, QStringList *list) :
    QDialog(parent),
    ui(new Ui::jumpToWindow)
{
    ui->setupUi(this);

    for (int i=0; i<list->size(); i++) {
        ui->listWidget->addItem(list->at(i));
    }
    ui->listWidget->item(0)->setSelected(true);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked,
            this, &jumpToWindow::accept);
}

jumpToWindow::~jumpToWindow()
{
    delete ui;
}

void jumpToWindow::accept() {
    jump_to_location = ui->listWidget->selectedItems().at(0)->text();
    setResult(QDialog::Accepted);
    hide();
}
