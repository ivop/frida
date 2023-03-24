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
#include "changesegmentwindow.h"
#include "commentwindow.h"
#include "disassembler.h"
#include "exportassembly.h"
#include "frida.h"
#include "jumptowindow.h"
#include "labelswindow.h"
#include "loadsaveproject.h"
#include "lowhighbytewindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBrush>
#include <QComboBox>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>

bool generateLocalLabels = true;
extern QPalette light_palette, dark_palette;

QBrush datatypeBrushes[DT_LAST] = {
    [DT_UNDEFINED_BYTES] = { QColor(255, 255, 255, 255), Qt::SolidPattern }, // white
    [DT_BYTES]           = { QColor(128, 128, 255, 255), Qt::SolidPattern }, // blueish
    [DT_UNDEFINED_CODE]  = { QColor(255,   0,   0, 255), Qt::SolidPattern }, // red
    [DT_CODE]            = { QColor(  0, 255,   0, 255), Qt::SolidPattern }, // green
    [DT_WORDLE]          = { QColor(  0, 255, 255, 255), Qt::SolidPattern }, // cyan
    [DT_WORDBE]          = { QColor(  0, 192, 192, 255), Qt::SolidPattern }, // dark cyan
    [DT_DWORDLE]         = { QColor(255,   0, 255, 255), Qt::SolidPattern }, // magenta
    [DT_DWORDBE]         = { QColor(192,   0, 192, 255), Qt::SolidPattern }, // dark magenta
    [DT_QWORDLE]         = { QColor(255, 255,   0, 255), Qt::SolidPattern }, // yellow
    [DT_QWORDBE]         = { QColor(192, 192,   0, 255), Qt::SolidPattern }, // dark yellow
    [DT_XWORDLE]         = { QColor(255, 128,   0, 255), Qt::SolidPattern }, // orange
    [DT_XWORDBE]         = { QColor(128,  64,   0, 255), Qt::SolidPattern }, // dark orange
    [DT_ASCII]           = { QColor( 96,  96,  96, 255), Qt::SolidPattern },
    [DT_ATASCII]         = { QColor(112, 112, 112, 255), Qt::SolidPattern },
    [DT_PETSCII]         = { QColor(128, 128, 128, 255), Qt::SolidPattern },
    [DT_ANTIC_SCREEN]    = { QColor(144, 144, 144, 255), Qt::SolidPattern },
    [DT_CBM_SCREEN]      = { QColor(160, 160, 160, 255), Qt::SolidPattern },
    [DT_INVERSE_ATASCII]      = { QColor(112, 112, 112, 255), Qt::Dense3Pattern },
    [DT_INVERSE_ANTIC_SCREEN] = { QColor(144, 144, 144, 255), Qt::Dense3Pattern },
};

const char *datatypeNames[DT_LAST] = {
    [DT_UNDEFINED_BYTES] = "Undefined Bytes",
    [DT_BYTES]           = "Bytes",
    [DT_UNDEFINED_CODE]  = "Undefined Code",
    [DT_CODE]            = "Code",
    [DT_WORDLE]          = "Word (16-bit Little-Endian)",
    [DT_WORDBE]          = "Word (16-bit Big-Endian)",
    [DT_DWORDLE]         = "DWord (32-bit Little-Endian)",
    [DT_DWORDBE]         = "DWord (32-bit Big-Endian)",
    [DT_QWORDLE]         = "QWord (64-bit Little-Endian)",
    [DT_QWORDBE]         = "QWord (64-bit Big-Endian)",
    [DT_XWORDLE]         = "XWord (128-bit Little-Endian)",
    [DT_XWORDBE]         = "XWord (128-bit Big-Endian)",
    [DT_ASCII]           = "ASCII String",
    [DT_ATASCII]         = "ATASCII String",
    [DT_PETSCII]         = "PETSCII String",
    [DT_ANTIC_SCREEN]    = "ANTIC Screen Codes",
    [DT_CBM_SCREEN]      = "CBM Screen Codes",
    [DT_INVERSE_ATASCII]      = "Inverse ATASCII String",
    [DT_INVERSE_ANTIC_SCREEN] = "Inverse ANTIC Screen Codes",
};

// Do not change the order, as its index is saved as part of the project

const char *fontnames[FONT_LAST] = {
    [FONT_NORMAL]        = "ASCII",
    [FONT_ATARI8BIT]     = "ATASCII",
    [FONT_C64]           = "PETSCII"
};

// --------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QTableWidget *t;
    ui->setupUi(this);

    ui->plainTextEditNotes->setPlainText(globalNotes);

    connect(ui->labelsButton, &QPushButton::clicked,
            this, &MainWindow::onLabelsButton_clicked);
    connect(ui->exitButton, &QPushButton::clicked,
            this, &MainWindow::onExitButton_clicked);
    connect(ui->saveButton, &QPushButton::clicked,
            this, &MainWindow::onSaveButton_clicked);
    connect(ui->exportAsmButton, &QPushButton::clicked,
            this, &MainWindow::onExportAsmButton_clicked);
    connect(ui->findButton, &QPushButton::clicked,
            this, &MainWindow::onFindButton_clicked);

    connect(ui->checkLocalLabels, &QCheckBox::toggled,
            this, &MainWindow::onCheckLocalLabels_toggled);
    connect(ui->checkDark, &QCheckBox::toggled,
            this, &MainWindow::onCheckDark_toggled);
    connect(ui->checkFullscreen, &QCheckBox::toggled,
            this, &MainWindow::onCheckFullscreen_toggled);

    connect(ui->tableSegments, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onTableSegments_itemSelectionChanged);
    connect(ui->tableSegments, &QTableWidget::cellChanged,
            this, &MainWindow::onTableSegments_cellChanged);

    connect(ui->comboFonts, QOverload<int>::of(&QComboBox::activated),
            this, &MainWindow::onComboFonts_activated);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onComboBox_currentIndexChanged);

    connect(ui->tableDisassembly, &QTableWidget::doubleClicked,
            this, &MainWindow::onTableDisassembly_doubleClicked);
    connect(ui->tableReferences, &QTableWidget::doubleClicked,
            this, &MainWindow::onTableReferences_doubleClicked);

    connect(ui->tableDisassembly, &QTableWidget::cellChanged,
            this, &MainWindow::onTableDisassembly_cellChanged);

    connect(ui->inputReference, &QLineEdit::returnPressed,
            this, &MainWindow::onReferences_returnPressed);

    t = ui->tableSegments;
    t->horizontalHeader()->setSectionsClickable(false);
    t->addAction(ui->actionDelete_Segment);
    t->addAction(ui->actionChange_Start_Address);
    showSegments();

    // create context menus for tableHexadecimal and tableASCII

    QTableWidget *two[2] = {ui->tableHexadecimal, ui->tableASCII};
    for (auto & i : two) {
        t = i;

        t->horizontalHeader()->setSectionsClickable(false);
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        t->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        t->setRowCount(0);

        t->addAction(ui->actionTrace);
        t->addAction(ui->actionSet_To_Byte);
        t->addAction(ui->actionSet_To_Undefined);
        t->addAction(ui->actionSet_To_Code);

        auto *men = new QMenu();
        men->addAction(ui->actionSet_To_WordLE);
        men->addAction(ui->actionSet_To_DWordLE);
        men->addAction(ui->actionSet_To_QWordLE);
        men->addAction(ui->actionSet_To_XWordLE);
        auto *act = new QAction(QStringLiteral("Set to Little-Endian"), ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

        men = new QMenu();
        men->addAction(ui->actionSet_To_WordBE);
        men->addAction(ui->actionSet_To_DWordBE);
        men->addAction(ui->actionSet_To_QWordBE);
        men->addAction(ui->actionSet_To_XWordBE);
        act = new QAction(QStringLiteral("Set to Big-Endian"), ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

        men = new QMenu();
        men->addAction(ui->actionSet_To_ASCII);
        men->addAction(ui->actionSet_To_ATASCII);
        men->addAction(ui->actionSet_To_PETSCII);
        men->addAction(ui->actionSet_To_ANTIC_Screen_Codes);
        men->addAction(ui->actionSet_To_CBM_Screen_Codes);
        act = new QAction(QStringLiteral("Set to String"), ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

        men = new QMenu();
        men->addAction(ui->actionSet_Flag_Labelled);
        men->addAction(ui->actionSet_Flag_Low_Byte);
        men->addAction(ui->actionSet_Flag_High_Byte);
        men->addAction(ui->actionSet_Flag_Clear);
        act = new QAction(QStringLiteral("Set Flag"), ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

    }

    // sync hex and ascii scrollbars and vice versa
    connect(ui->tableHexadecimal->verticalScrollBar(),
            &QAbstractSlider::valueChanged,
            ui->tableASCII->verticalScrollBar(),
            &QAbstractSlider::setValue);
    connect(ui->tableASCII->verticalScrollBar(),
            &QAbstractSlider::valueChanged,
            ui->tableHexadecimal->verticalScrollBar(),
            &QAbstractSlider::setValue);

    // disconnect existing Pressed and Clicked on vertical headers
    disconnect(ui->tableHexadecimal->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionPressed), 0, 0);
    disconnect(ui->tableHexadecimal->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionClicked), 0, 0);
    disconnect(ui->tableASCII->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionPressed), 0, 0);
    disconnect(ui->tableASCII->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionClicked), 0, 0);
    disconnect(ui->tableReferences->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionPressed), 0, 0);
    disconnect(ui->tableReferences->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionClicked), 0, 0);

    // connect our sectionClicked function on vertical headers
    connect(ui->tableHexadecimal->verticalHeader(),
            &QHeaderView::sectionClicked,
            this, &MainWindow::onHexSectionClicked);
    connect(ui->tableASCII->verticalHeader(),
            &QHeaderView::sectionClicked,
            this, &MainWindow::onHexSectionClicked);
    connect(ui->tableReferences->verticalHeader(),
            &QHeaderView::sectionClicked,
            this, &MainWindow::onReferencesSectionClicked);

    // set tableDisassembly

    t = ui->tableDisassembly;
    disconnect(ui->tableDisassembly->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionPressed), 0, 0);
    disconnect(ui->tableDisassembly->verticalHeader(),
               qOverload<int>(&QHeaderView::sectionClicked), 0, 0);
    connect(ui->tableDisassembly->verticalHeader(),
            &QHeaderView::sectionClicked,
            this, &MainWindow::onDisassemblySectionClicked);

    connect(ui->tableDisassembly->verticalScrollBar(),
            qOverload<int>(&QScrollBar::valueChanged),
            this,
            &MainWindow::rememberValue);

    t->addAction(ui->actionComment);
    t->addAction(ui->actionAdd_Label);
    t->addAction(ui->actionFind);
    t->setRowCount(0);

    t = ui->tableLegend;
    t->setRowCount(DT_LAST);
    int col = 0;
    for (int i=0; i<DT_LAST; i++) {
        t->setItem(i>>1,col, new QTableWidgetItem(datatypeNames[i]));
        t->item(i>>1,col)->setBackground(datatypeBrushes[i]);
        t->item(i>>1,col)->setForeground(Qt::black);
        col ^= 1;
    }

    for (auto & fontname : fontnames) {
        ui->comboFonts->addItem(fontname);
    }
    ui->comboFonts->setCurrentIndex(altfont);

    // use very large sizes so they will be scaled to fit the screen
    // (unless your screen is larger than 10kx10k :)

    ui->splitterSegNotes->setSizes(       { 3400, 3300, 3300 } );
    ui->splitterDisRef->setSizes(         { 7500, 2500 } );
    ui->splitterHexAsciiLegend->setSizes( { 7000, 3000 } );
    ui->splitterMain->setSizes(           { 2000, 4500, 3500 });

    ui->tableSegments->selectRow(0); // triggers all show functions

    ui->instructionDescription->setReadOnly(true);

    if (settings.value(QStringLiteral("Mode")) == "dark")
        darkMode = true;
    else
        darkMode = false;

    ui->checkDark->setChecked(darkMode);

    if (settings.value(QStringLiteral("Fullscreen")) == "on") {
        fullscreen = true;
        this->showFullScreen();
    } else {
        fullscreen = false;
        this->showMaximized();
    }

    ui->checkFullscreen->setChecked(fullscreen);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --------------------------------------------------------------------------
// SEGMENTS

void MainWindow::showSegments(void) {
    QTableWidget *t = ui->tableSegments;
    QTableWidgetItem *item;

    t->setRowCount(0);
    t->setRowCount(segments.size());

    for (int i=0; i<segments.size(); i++) {
        const struct segment *s = &segments.at(i);

        QString hex = QStringLiteral("%1").arg(s->start, 8, 16, (QChar)'0');
        item = new QTableWidgetItem(hex);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(i,0, item);

        hex = QStringLiteral("%1").arg(s->end, 8, 16, (QChar)'0');
        item = new QTableWidgetItem(hex);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(i,1, item);

        item = new QTableWidgetItem(s->name);
        t->setItem(i,2, item);
    }
}

void MainWindow::onTableSegments_itemSelectionChanged()
{
    currentSegment = ui->tableSegments->currentRow();
    if (currentSegment < 0) return;
    Disassembler->generateDisassembly(generateLocalLabels);
    showHex();                              // after generate, can change dt's
    showAscii();
    showDisassembly();

    QTableWidget *t = ui->tableDisassembly;
    QScrollBar  *sb = t->verticalScrollBar();

    int value = segments[currentSegment].scrollbarValue;

    sb->setMaximum(t->rowCount() - sb->pageStep());
    sb->setValue(value);
}

void MainWindow::actionDelete_Segment() {
    QTableWidget *t = ui->tableSegments;
    QMessageBox msg;
    int cur = currentSegment;

    if (!t->hasFocus()) return;

    msg.setText(QStringLiteral("WARNING! Do you really want to delete this segment?"));
    msg.addButton(QStringLiteral("No"), QMessageBox::RejectRole);
    msg.addButton(QStringLiteral("Yes, DELETE this segment"), QMessageBox::AcceptRole);
    if (!msg.exec()) return;

    segments.removeAt(cur);
    if (cur) cur--;
    showSegments();
    ui->tableSegments->selectRow(cur);
}

void MainWindow::actionChange_Start_Address() {
    QTableWidget *t = ui->tableSegments;
    int cur = currentSegment;
    quint64 newaddr = segments[cur].start;

    if (!t->hasFocus()) return;

    changesegmentwindow csw(segments[cur].start, &newaddr);
    if (csw.exec()) {
        quint64 diff = newaddr - segments[cur].start;
        if (!diff) return;
        segments[cur].start += diff;
        segments[cur].end   += diff;
        showSegments();
        ui->tableSegments->selectRow(cur);
    }
}

void MainWindow::onTableSegments_cellChanged(int row, int column) {
    QTableWidget *t = ui->tableSegments;

    if (column != 2) return;
    if (!t->item(row,column)->isSelected()) return;

    segments[row].name = t->item(row, column)->text();
}

// --------------------------------------------------------------------------
// RENDER HEX

void MainWindow::showHex(void) {
    const struct segment *s = &segments.at(currentSegment);
    QTableWidget *t = ui->tableHexadecimal;

    t->setRowCount(0);
    t->verticalHeader()->setDefaultAlignment(Qt::AlignRight);

    qint64 size = s->end - s->start + 1;

    t->setRowCount(size/8+(size%8?1:0));

    quint8 *data = s->data;

    for (int i=0; i<size; i++) {
        const struct segment *s = &segments.at(currentSegment);

        QString hex = QStringLiteral("%1").arg(data[i], 2, 16, (QChar)'0');
        t->setItem(i/8, i%8, new QTableWidgetItem(hex));

        QBrush brush = datatypeBrushes[(quint8)s->datatypes[i]];
        if (s->flags[i] & (FLAG_USE_LABEL | FLAG_LOW_BYTE | FLAG_HIGH_BYTE))
            brush.setStyle(Qt::Dense3Pattern);
        t->item(i/8, i%8)->setBackground(brush);
        t->item(i/8, i%8)->setForeground(Qt::black);

        if (!(i%8)) {
            hex = QStringLiteral("%1").arg(s->start + i, 2, 16, (QChar)'0');
            t->setVerticalHeaderItem(i/8, new QTableWidgetItem(hex));
        }
    }
}

// --------------------------------------------------------------------------
// RENDER ASCII

void MainWindow::showAscii(void) {
    const struct segment *s = &segments.at(currentSegment);
    QTableWidget *t = ui->tableASCII;
    QFont font;
    qint32 offset = 0;

    if (altfont == FONT_ATARI8BIT) {
        font = QFont(QStringLiteral("Atari Classic Int"), 10, 1);
        offset = 0xe000;
    }
    if (altfont == FONT_C64) {
        font = QFont(QStringLiteral("C64 Pro Mono"), 10, 1);
        offset = 0xe100;
    }

    t->setRowCount(0);
    t->verticalHeader()->setDefaultAlignment(Qt::AlignRight);

    qint64 size = s->end - s->start + 1;

    t->setRowCount(size/8+(size%8?1:0));

    quint8 *data = s->data;

    for (int i=0; i<size; i++) {
        t->setItem(i/8, i%8, new QTableWidgetItem(QChar(offset+data[i])));

        QBrush brush = datatypeBrushes[(quint8)s->datatypes[i]];
        if (s->flags[i] & (FLAG_USE_LABEL | FLAG_LOW_BYTE | FLAG_HIGH_BYTE))
            brush.setStyle(Qt::Dense3Pattern);
        t->item(i/8, i%8)->setBackground(brush);
        t->item(i/8, i%8)->setForeground(Qt::black);

        if (altfont)
            t->item(i/8, i%8)->setFont(font);
        if (!(i%8)) {
            QString hex = QStringLiteral("%1").arg(s->start + i, 2, 16, (QChar)'0');
            t->setVerticalHeaderItem(i/8, new QTableWidgetItem(hex));
        }
    }
}

// --------------------------------------------------------------------------
// CODE AND DATA ACTIONS

void MainWindow::Set_To_Foo(const QList<QTableWidgetSelectionRange>& Ranges,
                                                       quint8 datatype) {
    for (const auto & range : Ranges) {
        for (int y = range.topRow(); y <= range.bottomRow(); y++) {
            for (int x = range.leftColumn(); x <= range.rightColumn(); x++) {
                unsigned int pos = y*8+x;
                if (pos <= segments.at(currentSegment).end) {
                    segments.at(currentSegment).datatypes[pos] = datatype;
                    segments.at(currentSegment).flags[pos] = 0;
                }
            }
        }
    }
    // seems plenty fast to just call the show functions :)
    Disassembler->generateDisassembly(generateLocalLabels);
    showHex();
    showAscii();
    showDisassembly();
}

void MainWindow::actionSet_To_Undefined(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_UNDEFINED_BYTES);
}

void MainWindow::actionSet_To_Byte(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_BYTES);
}

void MainWindow::actionSet_To_Code(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_CODE);
}

void MainWindow::actionSet_To_WordLE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_WORDLE);
}

void MainWindow::actionSet_To_WordBE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_WORDBE);
}

void MainWindow::actionSet_To_DWordLE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_DWORDLE);
}

void MainWindow::actionSet_To_DWordBE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_DWORDBE);
}

void MainWindow::actionSet_To_QWordLE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_QWORDLE);
}

void MainWindow::actionSet_To_QWordBE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_QWORDBE);
}

void MainWindow::actionSet_To_XWordLE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_XWORDLE);
}

void MainWindow::actionSet_To_XWordBE(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_XWORDBE);
}

void MainWindow::actionSet_To_ASCII(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_ASCII);
}

void MainWindow::actionSet_To_ATASCII(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_ATASCII);
}

void MainWindow::actionSet_To_PETSCII(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_PETSCII);
}

void MainWindow::actionSet_To_ANTIC_Screen_Codes(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_ANTIC_SCREEN);
}

void MainWindow::actionSet_To_CBM_Screen_Codes(void) {
    Set_To_Foo(ui->tableHexadecimal->selectedRanges(), DT_CBM_SCREEN);
}

// --------------------------------------------------------------------------
// FLAGS ACTIONS

// Set_Flags can in fact flag complete regions. Labelled data uses that.
// But Low and High bytes must contain a single cell! That's guaranteed
// by their respective actions below.

void MainWindow::Set_Flag(const QList<QTableWidgetSelectionRange>& Ranges, quint8 flag) {
    const struct segment *s = &segments.at(currentSegment);
    unsigned int pos;

    for (const auto & range : Ranges) {
        for (int y = range.topRow(); y <= range.bottomRow(); y++) {
            for (int x = range.leftColumn(); x <= range.rightColumn(); x++) {
                pos = y*8+x;
                if (pos <= s->end)
                    s->flags[pos] = flag;
            }
        }
    }
    Disassembler->generateDisassembly(generateLocalLabels);
    showHex();
    showAscii();
    showDisassembly();
}

void MainWindow::actionSet_Flag_Labelled(void) {
    Set_Flag(ui->tableHexadecimal->selectedRanges(), FLAG_USE_LABEL);
}

void MainWindow::Set_Flag_Low_or_High_Byte(bool bLow) {

    // check that only one byte is selected, otherwise error out

    QList<QTableWidgetSelectionRange> ranges =
                                    ui->tableHexadecimal->selectedRanges();

    int nranges = ranges.size();

    if (!nranges) return;

    int ypos_equal = ranges.at(0).bottomRow() == ranges.at(0).topRow();
    int xpos_equal = ranges.at(0).leftColumn() == ranges.at(0).rightColumn();
    bool single_cell = xpos_equal && ypos_equal;

    if (nranges != 1 || !single_cell) {
        QMessageBox msg;
        msg.setText(QStringLiteral("Select a single byte to flag as Low or High Byte     "));
        msg.addButton(QStringLiteral("OK"), QMessageBox::AcceptRole);
        msg.exec();
        return;
    }

    struct segment *s = &segments[currentSegment];

    int x = ranges.at(0).leftColumn();
    int y = ranges.at(0).topRow();
    int relpos = y*8 + x;
    int pos = s->start + relpos;

    uint16_t fulladdr;
    lowhighbytewindow lhbw(pos, bLow, s->data[relpos], &fulladdr);
    if (lhbw.exec()) {
        if (bLow) {
            if (s->datatypes[relpos] == DT_UNDEFINED_BYTES)
                s->datatypes[relpos] = DT_BYTES;
            s->flags[relpos] = FLAG_LOW_BYTE;
            s->lowbytes.remove(relpos);
            s->lowbytes.insert(relpos, fulladdr);
        } else {
            if (s->datatypes[relpos] == DT_UNDEFINED_BYTES)
                s->datatypes[relpos] = DT_BYTES;
            s->flags[relpos] = FLAG_HIGH_BYTE;
            s->highbytes.remove(relpos);
            s->highbytes.insert(relpos, fulladdr);
        }
    } else {

    }

    Disassembler->generateDisassembly(generateLocalLabels);
    showHex();
    showAscii();
    showDisassembly();
 }

void MainWindow::actionSet_Flag_Low_Byte(void) {
    Set_Flag_Low_or_High_Byte(true);
}

void MainWindow::actionSet_Flag_High_Byte(void) {
    Set_Flag_Low_or_High_Byte(false);
}

void MainWindow::actionSet_Flag_Clear(void) {
    Set_Flag(ui->tableHexadecimal->selectedRanges(), 0);
}

// --------------------------------------------------------------------------
// RENDER DISASSEMBLY

void MainWindow::showDisassembly(void) {
    struct segment *s = &segments[currentSegment];
//    quint64 start = s->start, end = s->end, size = end - start + 1;
    QTableWidget *t = ui->tableDisassembly;
    QList<struct disassembly> *dislist  = &s->disassembly;
    QMap<quint64, QString> *comments     = &s->comments;
    QTableWidgetItem *item;
    struct disassembly dis;
    QString com;
    int row;
    bool zero_comment_done = false;

    t->setRowCount(0);
    t->verticalHeader()->setDefaultAlignment(Qt::AlignRight);

    qint64 di = 0;
    while (di < dislist->size()) {
        dis = dislist->at(di);

        if (comments->contains(dis.address)) {
            if (dis.address == 0 && zero_comment_done)
                goto skip_comment;
            if (dis.address == 0)
                zero_comment_done = true; // no double comments for raw files

            com = comments->value(dis.address);

            row = t->rowCount();
            t->setRowCount(row+1);
            QString hex = QStringLiteral("%1").arg(dis.address, 0, 16, (QChar)'0');
            t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));
            t->verticalHeaderItem(row)->setTextAlignment(Qt::AlignTop | Qt::AlignRight);

            t->setSpan(row,1,1,2);

            item = new QTableWidgetItem(QStringLiteral(";"));
            item->setTextAlignment(Qt::AlignTop);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            t->setItem(row, 0, item);

            item = new QTableWidgetItem(com);
            item->setForeground(Qt::darkGray);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            t->setItem(row, 1, item);

            t->resizeRowToContents(row);
        }

        skip_comment:

        if (   globalLabels.contains(dis.address)
            || s->localLabels.contains(dis.address)) {

            QString label;
            if (s->localLabels.contains(dis.address))
                label = s->localLabels.value(dis.address);
            else
                label = globalLabels.value(dis.address);

            // we do not print labels with + or -

            if (!label.contains(QChar('+')) && !label.contains(QChar('-'))) {
                row = t->rowCount();
                t->setRowCount(row+1);
                QString hex = QStringLiteral("%1").arg(dis.address, 0, 16, (QChar)'0');
                t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));

                t->setSpan(row,0,1,3);
                t->setItem(row, 0, new QTableWidgetItem(label));
                if (s->localLabels.contains(dis.address))
                    t->item(row,0)->setForeground(Qt::darkCyan);
                else
                    t->item(row,0)->setForeground(Qt::darkMagenta);
            }
        }

        row = t->rowCount();
        t->setRowCount(row+1);

        QString hex = QStringLiteral("%1").arg(dis.address, 0, 16, (QChar)'0');
        t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));

        item = new QTableWidgetItem(QLatin1String(""));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(row, 0, item);

        item = new QTableWidgetItem(dis.instruction);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(row, 1, item);

        item = new QTableWidgetItem(dis.arguments);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(row, 2, item);

        if (dis.instruction.at(0) == QStringLiteral("."))
            t->item(row,1)->setForeground(QColor(0,96,0));

        if (dis.changes_pc) {
            row = t->rowCount();
            t->setRowCount(row+1);
            t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));
            item = new QTableWidgetItem(QLatin1String(""));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            t->setSpan(row,0,1,3);
            t->setItem(row, 0, item);
        }

        di++;
    }
}

// --------------------------------------------------------------------------
// HEX AND ASCII LINKED

void MainWindow::linkSelections(QTableWidget *from, QTableWidget *to) {
    QList<QTableWidgetSelectionRange> Ranges = from->selectedRanges();

    to->clearSelection();
    for (const auto & Range : Ranges) {
        to->setRangeSelected(Range, true);
    }

}

void MainWindow::linkHexASCIISelection() {
    if (ui->tableHexadecimal->hasFocus())
        linkSelections(ui->tableHexadecimal, ui->tableASCII);
}

void MainWindow::linkASCIIHexSelection() {
    if (ui->tableASCII->hasFocus())
        linkSelections(ui->tableASCII, ui->tableHexadecimal);
}

// Used same function for both Hex and Ascii Sections

void MainWindow::onHexSectionClicked(int index) {
    QTableWidget *th = ui->tableHexadecimal;
    QTableWidget *td = ui->tableDisassembly;
    QString s = th->verticalHeaderItem(index)->text();
    int n = td->rowCount();

    for (int i=0; i<n; i++) {
        if (td->verticalHeaderItem(i)->text().toULongLong(0,16) < s.toULongLong(0,16))
            continue;
        td->scrollToItem(td->item(i,0), QAbstractItemView::PositionAtCenter);
        break;
    }
}

// --------------------------------------------------------------------------
// TRACE

void MainWindow::actionTrace(void) {
    QTableWidget *t = ui->tableHexadecimal;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();
    int x = INT_MAX;
    int y = INT_MAX;

    if (Ranges.isEmpty())
        return;

    for (const auto & range : Ranges) {
        if (range.topRow() < y)
            y = range.topRow();
        if (range.leftColumn() < x)
            x = range.leftColumn();
    }
    int pos = y*8 + x;
    pos += segments[currentSegment].start;

    Disassembler->trace(pos);
    Disassembler->generateDisassembly(generateLocalLabels);
    showHex();
    showAscii();
    showDisassembly();
}

// --------------------------------------------------------------------------
// COMMENTS

void MainWindow::actionComment() {
    QTableWidget *t = ui->tableDisassembly;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();

    if (Ranges.isEmpty())
        return;

    // assume UI is properly setup to only allow ONE cell to be selected

    int i = Ranges.at(0).topRow();
    QString s = t->verticalHeaderItem(i)->text();
    quint64 a = s.toULongLong(0, 16);
    QString c = segments[currentSegment].comments.value(a);

    auto *cw = new commentwindow(s, c);
    cw->exec();
    c = cw->retrieveComment();
    if (c.isEmpty())
        segments[currentSegment].comments.remove(a);
    else
        segments[currentSegment].comments.insert(a, c);
    showDisassembly();
}

// ----------------------------------------------------------------------------
// BUTTONS

void MainWindow::onLabelsButton_clicked() {
    labelswindow lw;
    lw.exec();
    Disassembler->generateDisassembly(generateLocalLabels);
    showHex();
    showAscii();
    showDisassembly();
}

void MainWindow::onExitButton_clicked() {
    close();
}

void MainWindow::onComboBox_currentIndexChanged(int index) {
    QFont f = this->font();
    f.setPointSize(10+index);
    this->setFont(f);
}

void MainWindow::onComboFonts_activated(int index) {
    altfont = (enum fonts) index;
    showAscii();
    linkSelections(ui->tableHexadecimal, ui->tableASCII);
}

void MainWindow::onSaveButton_clicked() {
    globalNotes = ui->plainTextEditNotes->toPlainText();
    save_project(this);
}


void MainWindow::onExportAsmButton_clicked() {
    export_assembly(this, generateLocalLabels);
}
void MainWindow::actionAdd_Label(void) {
    addLabelWindow alw;
    alw.exec();
    showDisassembly();
}

void MainWindow::onCheckLocalLabels_toggled() {
    generateLocalLabels = ui->checkLocalLabels->isChecked();
}

void MainWindow::onCheckDark_toggled() {
    darkMode = ui->checkDark->isChecked();
    if (darkMode) {
        QApplication::setPalette(dark_palette);
        settings.setValue(QStringLiteral("Mode"), "dark");
    } else {
        QApplication::setPalette(light_palette);
        settings.setValue(QStringLiteral("Mode"), "light");
    }
    settings.sync();
}

void MainWindow::onCheckFullscreen_toggled() {
    fullscreen = ui->checkFullscreen->isChecked();
    settings.setValue(QStringLiteral("Fullscreen"), "on");
    if (fullscreen) {
        this->showFullScreen();
        settings.setValue(QStringLiteral("Fullscreen"), "on");
    } else {
        this->showMaximized();
        settings.setValue(QStringLiteral("Fullscreen"), "off");
    }
    settings.sync();
}

// ----------------------------------------------------------------------------
// TABLE DISASSEMBLY

void MainWindow::rememberValue(int value) {
    if (currentSegment < 0) return;
    segments[currentSegment].scrollbarValue = value;
}

void MainWindow::onTableDisassembly_cellChanged(int row, int column) {
    struct segment *s = &segments[currentSegment];
    QTableWidget *t = ui->tableDisassembly;

    if (! t->item(row,column)->isSelected()) return; // not a user action

    if (!(t->item(row,column)->flags() & Qt::ItemIsEditable)) return;

    // editable, so it's a label cell

    quint64 address = t->verticalHeaderItem(row)->text().toULongLong(0,16);
    QString label = t->item(row,column)->text();

    if (label.isEmpty()) {
        // restore old key value

        QString txt = s->localLabels.value(address);

        if (txt.isEmpty())
            txt = globalLabels.value(address);

        t->item(row,column)->setText(txt);

        return;
    }

    if (s->localLabels.contains(address))
        s->localLabels.insert(address, label);
    else if (globalLabels.contains(address))
        globalLabels.insert(address,label);

    Disassembler->generateDisassembly(generateLocalLabels);
    showDisassembly();
}

void MainWindow::onDisassemblySectionClicked(int index) {
    QTableWidget *th = ui->tableHexadecimal;
    QTableWidget *td = ui->tableDisassembly;
    QString s = td->verticalHeaderItem(index)->text();
    int n = th->rowCount();

    for (int i=0; i<n; i++) {
        if (th->verticalHeaderItem(i)->text().toULongLong(0,16) < s.toULongLong(0,16))
            continue;
        th->scrollToItem(th->item(i,0), QAbstractItemView::PositionAtCenter);
        break;
    }

    quint64 a = s.toULongLong(0,16);
    quint64 row;
    quint64 column;
    a -= segments[currentSegment].start;
    row    = a / 8;
    column = a % 8;

    QTableWidgetSelectionRange range(row, column, row, column);
    ui->tableHexadecimal->clearSelection();
    ui->tableHexadecimal->setRangeSelected(range, true);
    ui->tableASCII->clearSelection();
    ui->tableASCII->setRangeSelected(range, true);
}

void MainWindow::onTableDisassembly_doubleClicked(const QModelIndex &index) {
    struct segment *s = &segments[currentSegment];
    QTableWidget *t = ui->tableDisassembly;
    QTableWidgetItem *item = t->item(index.row(),0);

    if (!item) return;

    // coment lines:

    QString first = t->item(index.row(), 0)->text();

    if (first == QLatin1String(";")) {
        actionComment();
        return;
    }

    // empty lines span three columns

    if (t->columnSpan(index.row(), index.column()) == 3) {

    }

    // other lines have columnSpan of one (space, opcode, operand/label)

    // opcode

    if (t->columnSpan(index.row(), index.column()) == 1 && index.column() == 1) {
        QTableWidgetItem *vhitem = t->verticalHeaderItem(index.row());
        quint64 address = vhitem->text().toULongLong(nullptr, 16);

        QString descr = Disassembler->getDescriptionAt(address);

        if (descr.isEmpty())
            ui->instructionDescription->document()->setHtml(QStringLiteral("Not available."));
        else
            ui->instructionDescription->document()->setHtml(descr);
    }

    // operand(s)

    if (t->columnSpan(index.row(), index.column()) == 1 && index.column() == 2) {

        QString operand = t->item(index.row(), 2)->text();

        // check for #<label and #>label

        if ((operand.left(2) == QLatin1String("#<")) || (operand.left(2) == QLatin1String("#>"))) {
            operand = operand.mid(2);
        }

        // check for line of addresses/labels

        QStringList operand_list;

        if (operand.contains(QLatin1String(","))) {
            operand_list = operand.split(QStringLiteral(","));

            operand_list.replaceInStrings(QStringLiteral("("), QLatin1String("")); // (label)
            operand_list.replaceInStrings(QStringLiteral(")"), QLatin1String(""));
            operand_list.replaceInStrings(QStringLiteral("<"), QLatin1String("")); // <label in .byte
            operand_list.replaceInStrings(QStringLiteral(">"), QLatin1String(""));

            for (int i = 0; i < operand_list.size(); i++) {
                operand_list.replace(i, operand_list.at(i).trimmed());
            }

            auto *jtw = new jumpToWindow(nullptr, &operand_list);

            jtw->exec();

            operand = jtw->jump_to_location;
        }

        QMap<quint64, QString>::const_iterator iter;

        // find value for key if it exists

        // find local label

        for (iter  = s->localLabels.constBegin();
             iter != s->localLabels.constEnd();
             iter++) {

            if (iter.value() == operand) {
                break;      // found!
            }
        }

        // if not found, find global label

        if (iter == s->localLabels.constEnd()) {
            for (iter  = globalLabels.constBegin();
                 iter != globalLabels.constEnd();
                 iter++) {

                if (iter.value() == operand) {
                    break;      // found!
                }
            }
        }

        quint64 addr;
        QString hexPrefix = Disassembler->hexPrefix;
        QString hexSuffix = Disassembler->hexSuffix;

        // if no label is found, check for hexPrefixed or hexSuffixed address

        if (iter == globalLabels.constEnd()) {
            if (hexPrefix.size() && operand.left(hexPrefix.size()) == hexPrefix) {
                operand = operand.mid(hexPrefix.size());
                addr = operand.toULongLong(0,16);
            } else if (hexSuffix.size() && operand.right(hexSuffix.size()) == hexSuffix) {
                operand = operand.left(operand.size()-hexSuffix.size());
                addr = operand.toULongLong(0,16);
            } else {
                return;     // nothing found to jump to
            }
        } else {
            addr = iter.key();
        }

        // find segment that contains address

        int i;
        for (i = 0; i<segments.size(); i++) {
            if ((addr >= segments[i].start) && (addr <= segments[i].end))
                break;
        }

        if (i == segments.size()) // not found
            return;

        // otherwise,
        // jump to segment and scroll disassembly to the right place

        jumpToSegmentAndAddress(i, addr);
    }

    // labeled lines span three columns, too, but are editable and don't reach
    // this function.
}

// ----------------------------------------------------------------------------
// MISC

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox msg;

    msg.setText(QStringLiteral("Do you really want to exit Frida?"));
    msg.addButton(QStringLiteral("No"), QMessageBox::RejectRole);
    msg.addButton(QStringLiteral("Yes"), QMessageBox::AcceptRole);
    if (!msg.exec()) {
        event->ignore();
        return;
    }

    event->accept();
}

void MainWindow::jumpToSegmentAndAddress(quint64 segment, quint64 address) {

    // jump to segment and scroll disassembly to the right place

    ui->tableSegments->selectRow(segment);

    QCoreApplication::processEvents(QEventLoop::AllEvents); // direct triggers
    QCoreApplication::processEvents(QEventLoop::AllEvents); // indirect triggers

    // search row and center

    QTableWidget *td = ui->tableDisassembly;

    for (int row = 0; row < td->rowCount(); row++) {
        if (td->verticalHeaderItem(row)->text().toULongLong(0,16) < address)
            continue;

        // found row:
        if (td->item(row,0)->text() == QStringLiteral(";")) row++; // skip comment
        td->scrollToItem(td->item(row,0), QAbstractItemView::PositionAtCenter);
        td->clearSelection();   // if segment is the same, clear selected
        td->clearFocus();       // and focus
        td->item(row,0)->setSelected(true);
        break;
    }
}

// ----------------------------------------------------------------------------
// REFERENCES

void MainWindow::onTableReferences_doubleClicked(const QModelIndex &index) {
    onReferencesSectionClicked(index.row());
}

void MainWindow::onReferencesSectionClicked(int row) {
    QTableWidget *t = ui->tableReferences;

    int segment = t->item(row, 0)->text().toInt(nullptr, 16);
    quint64 address = t->verticalHeaderItem(row)->text().toULongLong(nullptr, 16);

    jumpToSegmentAndAddress(segment, address);
}

void MainWindow::onReferences_returnPressed() {
    onFindButton_clicked();
}

void MainWindow::actionFind(void) {
    QTableWidget *t = ui->tableDisassembly;
    int column = t->currentColumn();
    int row = t->currentRow();

    ui->inputReference->setFocus();
    ui->inputReference->selectAll();

    if (column != 0)
        return;

    QString text = t->item(row, column)->text();

    if (text.isEmpty() || text == ";")
        return;

    ui->inputReference->setText(text);
    ui->inputReference->selectAll();
}

void MainWindow::addRefEntry(QTableWidget *t, quint64 segment, quint64 address,
                             const QString &line, const QString &highlight) {
    int row;
    QTableWidgetItem *item;

    row = t->rowCount();
    t->setRowCount(row+1);

    QString hex = QStringLiteral("%1").arg(address, 0, 16);
    t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));

    item = new QTableWidgetItem(QString("%1").arg(segment,0,16));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    t->setItem(row, 0, item);

    QModelIndex index = t->model()->index(row,1);
    QLabel *label = new QLabel();

    QString text = line;
    text = text.replace(highlight, "<span style=\"color: red\">" + highlight + "</span>");

    label->setTextFormat(Qt::RichText);
    label->setText(text);
    t->setIndexWidget(index, label);
}

void MainWindow::onFindButton_clicked(void) {
    struct segment *s = &segments[currentSegment];
    QMap<quint64, QString> *localLabels = &s->localLabels;
    int saveCurrent = currentSegment;
    QTableWidget *t = ui->tableReferences;
    QString what = ui->inputReference->text();
    quint64 address;

    if (what.size() < 3)    // limit search pattern to at least 3 characters
        return;

    t->setRowCount(0);
    t->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
    t->setColumnWidth(0,32);

    // first, check if any of the global labels match and if they are defined
    // in any segment

    QMap<quint64, QString>::const_iterator iter;
    for(iter = globalLabels.constBegin(); iter != globalLabels.constEnd(); iter++) {
        if (iter.value().contains(what)) {
            address = iter.key();
            for (int i=0; i<segments.size(); i++) {
                struct segment *s = &segments[i];
                if (address >= s->start && address <= s->end) {
                    QString line = iter.value();
                    addRefEntry(t, i, address, line, what);
                }
            }
            break;
        }
    }

    //second, find a local label in the current segment

    for(iter = localLabels->constBegin(); iter != localLabels->constEnd(); iter++) {
        if (iter.value().contains(what)) {
            address = iter.key();
            if (address >= s->start && address <= s->end) {
                QString line = iter.value();
                addRefEntry(t, currentSegment, address, line, what);
                break;
                }
        }
    }

    // third, find in disassembly of each segment

    for (int i=0; i<segments.size(); i++) {
        struct segment *s = &segments[i];

        currentSegment = i;
        Disassembler->generateDisassembly(generateLocalLabels);

        for (const auto &disitem : qAsConst(s->disassembly)) {
            if (disitem.arguments.contains(what)) {
                QString line = disitem.instruction + QStringLiteral(" ") + disitem.arguments;
                addRefEntry(t, i, disitem.address, line, what);
            }
        }
    }

    currentSegment = saveCurrent;
}
