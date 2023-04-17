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

#ifndef ADDCONSTANTTOGROUPWINDOW_H
#define ADDCONSTANTTOGROUPWINDOW_H

#include "pch.h"

namespace Ui {
class addConstantToGroupWindow;
}

class addConstantToGroupWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addConstantToGroupWindow(QWidget *parent = nullptr);
    ~addConstantToGroupWindow();

    qint64 constantValue {};
    QString constantName {};

public Q_SLOTS:
    void accept() override;

private:
    Ui::addConstantToGroupWindow *ui;
};

#endif // ADDCONSTANTTOGROUPWINDOW_H
