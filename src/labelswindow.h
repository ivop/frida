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

#ifndef LABELSWINDOW_H
#define LABELSWINDOW_H

#include "pch.h"

namespace Ui {
class labelswindow;
}

class labelswindow : public QDialog
{
    Q_OBJECT

public:
    explicit labelswindow(QWidget *parent = nullptr);
    ~labelswindow() override;

private Q_SLOTS:
    void actionChange_To_Local_Label();
    void actionChange_To_Global_Label();
    void actionDelete_Label();

    void onTableGlobalLabels_cellChanged(int row, int column);
    void onTableLocalLabels_cellChanged(int row, int column);

    void onDoneButton_clicked();
    void onAddLabelButton_clicked();
    void onExportButton_clicked();
    void onImportButton_clicked();

private:
    Ui::labelswindow *ui;

    static void showLabels(QTableWidget *t, QMap<quint64, QString> *labels);
    void showGlobalLabels(void);
    void showLocalLabels(void);

};

#endif // LABELSWINDOW_H
