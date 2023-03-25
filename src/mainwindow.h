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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetSelectionRange>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showSegments(void);
    void showHex(void);
    void showAscii(void);
    void showDisassembly(void);

    void closeEvent (QCloseEvent *event);

    bool darkMode, fullscreen;

public Q_SLOTS:
    void actionTrace(void);
    void actionSet_To_Undefined(void);
    void actionSet_To_Byte(void);
    void actionSet_To_Code(void);
    void actionSet_To_WordLE(void);
    void actionSet_To_WordBE(void);
    void actionSet_To_DWordLE(void);
    void actionSet_To_DWordBE(void);
    void actionSet_To_QWordLE(void);
    void actionSet_To_QWordBE(void);
    void actionSet_To_XWordLE(void);
    void actionSet_To_XWordBE(void);
    void actionSet_To_ASCII(void);
    void actionSet_To_ATASCII(void);
    void actionSet_To_PETSCII(void);
    void actionSet_To_ANTIC_Screen_Codes(void);
    void actionSet_To_CBM_Screen_Codes(void);
    void actionSet_Flag_Labelled(void);
    void actionSet_Flag_Low_Byte(void);
    void actionSet_Flag_High_Byte(void);
    void actionSet_Flag_Clear(void);
    void actionComment(void);
    void actionDelete_Segment(void);
    void actionChange_Start_Address(void);
    void actionAdd_Label(void);
    void actionFind(void);
    void actionLowAndHighBytePairs(void);

private Q_SLOTS:
    void linkHexASCIISelection(void);
    void linkASCIIHexSelection(void);
    static void linkSelections(QTableWidget *from, QTableWidget *to);
    void onHexSectionClicked(int index);
    void onDisassemblySectionClicked(int index);
    void onReferencesSectionClicked(int index);

    void onLabelsButton_clicked();
    void onExitButton_clicked();
    void onSaveButton_clicked();
    void onExportAsmButton_clicked();

    void onCheckLocalLabels_toggled();
    void onCheckDark_toggled();
    void onCheckFullscreen_toggled();

    void onTableSegments_itemSelectionChanged();
    void onTableSegments_cellChanged(int row, int column);
    void onTableDisassembly_cellChanged(int row, int column);

    void onComboFonts_activated(int index);
    void onComboBox_currentIndexChanged(int index);
    void onTableDisassembly_doubleClicked(const QModelIndex &index);
    void onTableReferences_doubleClicked(const QModelIndex &index);

    void onReferences_returnPressed();
    void onFindButton_clicked();

    static void rememberValue(int value);
    static void addRefEntry(QTableWidget *t, quint64 segment, quint64 address,
                       const QString &line, const QString &highlight);
    void jumpToSegmentAndAddress(quint64 segment, quint64 address);

private:
    Ui::MainWindow *ui;
    void Set_To_Foo(const QList<QTableWidgetSelectionRange>& ranges, quint8 datatype);
    void Set_Flag(const QList<QTableWidgetSelectionRange>& ranges, quint8 flag);
    void Set_Flag_Low_or_High_Byte(bool bLow);
};

#endif // MAINWINDOW_H
