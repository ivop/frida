#include "pch.h"
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

#ifndef CONSTANTSMANAGER_H
#define CONSTANTSMANAGER_H

namespace Ui {
class constantsManager;
}

class constantsManager : public QDialog
{
    Q_OBJECT

public:
    explicit constantsManager(QWidget *parent = nullptr);
    ~constantsManager();

private Q_SLOTS:
    void onGroups_itemSelectionChanged(void);

    void onGroups_cellChanged(int row, int column);
    void onValues_cellChanged(int row, int column);

    void actionDeleteGroup(void);
    void actionDeleteValue(void);

    void onAddGroup_clicked(void);
    void onAddValue_clicked(void);
    void onImport_clicked(void);
    void onExport_clicked(void);

private:
    Ui::constantsManager *ui;
    void showGroups();
};

#endif // CONSTANTSMANAGER_H
