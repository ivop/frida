#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commentwindow.h"
#include "labelswindow.h"
#include "changesegmentwindow.h"
#include <QDebug>
#include <QScrollBar>
#include <QMenu>
#include <QMessageBox>
#include <QBrush>
#include <sstream>
#include "frida.h"
#include "disassembler.h"

extern Disassembler *Disassembler;

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
};

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

    t = ui->tableSegments;
    t->horizontalHeader()->setSectionsClickable(false);
    t->addAction(ui->actionDelete_Segment);
    t->addAction(ui->actionChange_Start_Address);
    showSegments();

    // create context menus for tableHexadecimal and tableASCII

    QTableWidget *two[2] = {ui->tableHexadecimal, ui->tableASCII};
    for (int i=0; i<2; i++) {
        t = two[i];

        t->horizontalHeader()->setSectionsClickable(false);
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        t->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        t->setRowCount(0);

        t->addAction(ui->actionTrace);
        t->addAction(ui->actionSet_To_Byte);
        t->addAction(ui->actionSet_To_Undefined);
        t->addAction(ui->actionSet_To_Code);

        QMenu *men = new QMenu();
        men->addAction(ui->actionSet_To_WordLE);
        men->addAction(ui->actionSet_To_DWordLE);
        men->addAction(ui->actionSet_To_QWordLE);
        men->addAction(ui->actionSet_To_XWordLE);
        QAction *act = new QAction("Set to Little-Endian", ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

        men = new QMenu();
        men->addAction(ui->actionSet_To_WordBE);
        men->addAction(ui->actionSet_To_DWordBE);
        men->addAction(ui->actionSet_To_QWordBE);
        men->addAction(ui->actionSet_To_XWordBE);
        act = new QAction("Set to Big-Endian", ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

        men = new QMenu();
        men->addAction(ui->actionSet_To_ASCII);
        men->addAction(ui->actionSet_To_ATASCII);
        men->addAction(ui->actionSet_To_PETSCII);
        men->addAction(ui->actionSet_To_ANTIC_Screen_Codes);
        men->addAction(ui->actionSet_To_CBM_Screen_Codes);
        act = new QAction("Set to String", ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

        men = new QMenu();
        men->addAction(ui->actionSet_Flag_Labelled);
        men->addAction(ui->actionSet_Flag_Low_Byte);
        men->addAction(ui->actionSet_Flag_High_Byte);
        act = new QAction("Set Flag", ui->tableHexadecimal);
        act->setMenu(men);
        t->addAction(act);

    }

    // sync hex and ascii scrollbars and vice versa
    connect(ui->tableHexadecimal->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            ui->tableASCII->verticalScrollBar(),
            SLOT(setValue(int)));
    connect(ui->tableASCII->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            ui->tableHexadecimal->verticalScrollBar(),
            SLOT(setValue(int)));

    // disconnect existing Pressed and Clicked on vertical headers
    disconnect(ui->tableHexadecimal->verticalHeader(),
               SIGNAL(sectionPressed(int)), 0, 0);
    disconnect(ui->tableHexadecimal->verticalHeader(),
               SIGNAL(sectionClicked(int)), 0, 0);
    disconnect(ui->tableASCII->verticalHeader(),
               SIGNAL(sectionPressed(int)), 0, 0);
    disconnect(ui->tableASCII->verticalHeader(),
               SIGNAL(sectionClicked(int)), 0, 0);

    // connect our sectionClicked function on vertical headers
    connect(ui->tableHexadecimal->verticalHeader(),
            SIGNAL(sectionClicked(int)),
            this, SLOT(onHexSectionClicked(int)));
    connect(ui->tableASCII->verticalHeader(),
            SIGNAL(sectionClicked(int)),
            this, SLOT(onHexSectionClicked(int)));

    // set tableDisassembly

    t = ui->tableDisassembly;
    disconnect(ui->tableDisassembly->verticalHeader(),
               SIGNAL(sectionPressed(int)), 0, 0);
    disconnect(ui->tableDisassembly->verticalHeader(),
               SIGNAL(sectionClicked(int)), 0, 0);
    connect(ui->tableDisassembly->verticalHeader(),
            SIGNAL(sectionClicked(int)),
            this, SLOT(onDisassemblySectionClicked(int)));

    connect(ui->tableDisassembly->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            this,
            SLOT(rememberValue(int)));

    t->addAction(ui->actionComment);
    t->setRowCount(0);

    t = ui->tableLegend;
    t->setRowCount(DT_LAST);
    int col = 0;
    for (int i=0; i<DT_LAST; i++) {
        t->setItem(i>>1,col, new QTableWidgetItem(datatypeNames[i]));
        t->item(i>>1,col)->setBackground(datatypeBrushes[i]);
        col ^= 1;
    }

    for (int i=0; i<FONT_LAST; i++) {
        ui->comboFonts->addItem(fontnames[i]);
    }
    ui->comboFonts->setCurrentIndex(altfont);

    // use very large sizes so they will be scaled to fit the screen
    // (unless your screen is larger than 10kx10k :)

    ui->splitterSegNotes->setSizes(       { 7500, 2500 } );
    ui->splitterDisRef->setSizes(         { 8500, 1500 } );
    ui->splitterHexAsciiLegend->setSizes( { 7000, 3000 } );
    ui->splitterMain->setSizes(           { 2000, 4500, 3500 });

    ui->tableSegments->selectRow(0); // triggers all show functions
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --------------------------------------------------------------------------

void MainWindow::showSegments(void) {
    QTableWidget *t = ui->tableSegments;
    QTableWidgetItem *item;

    t->setRowCount(0);
    t->setRowCount(segments.size());

    for (int i=0; i<segments.size(); i++) {
        const struct segment *s = &segments.at(i);

        QString hex = QString("%1").arg(s->start, 8, 16, (QChar)'0');
        item = new QTableWidgetItem(hex);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(i,0, item);

        hex = QString("%1").arg(s->end, 8, 16, (QChar)'0');
        item = new QTableWidgetItem(hex);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(i,1, item);

        item = new QTableWidgetItem(s->name);
        t->setItem(i,2, item);
    }
}

void MainWindow::on_tableSegments_itemSelectionChanged()
{
    currentSegment = ui->tableSegments->currentRow();
    if (currentSegment < 0) return;
    Disassembler->generateDisassembly();
    showHex();                              // after generate, can change dt's
    showAscii();
    showDisassembly();

    QTableWidget *t = ui->tableDisassembly;
    QScrollBar  *sb = t->verticalScrollBar();

    int value = segments[currentSegment].scrollbarValue;

    sb->setMaximum(t->rowCount() - sb->pageStep());
    sb->setValue(value);
}

void MainWindow::rememberValue(int value) {
    if (currentSegment < 0) return;
    segments[currentSegment].scrollbarValue = value;
}

void MainWindow::actionDelete_Segment() {
    QTableWidget *t = ui->tableSegments;
    QMessageBox msg;
    int cur = currentSegment;

    if (!t->hasFocus()) return;

    msg.setText("WARNING! Do you really want to delete this segment?");
    msg.addButton("No", QMessageBox::RejectRole);
    msg.addButton("Yes, DELETE this segment", QMessageBox::AcceptRole);
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

void MainWindow::on_tableSegments_cellChanged(int row, int column) {
    QTableWidget *t = ui->tableSegments;

    if (column != 2) return;
    if (!t->item(row,column)->isSelected()) return;

    segments[row].name = t->item(row, column)->text();
}

// --------------------------------------------------------------------------

void MainWindow::showHex(void) {
    const struct segment *s = &segments.at(currentSegment);
    QTableWidget *t = ui->tableHexadecimal;

    t->setRowCount(0);

    qint64 size = s->end - s->start + 1;

    t->setRowCount(size/8+(size%8?1:0));

    quint8 *data = s->data;

    for (int i=0; i<size; i++) {
        const struct segment *s = &segments.at(currentSegment);

        QString hex = QString("%1").arg(data[i], 2, 16, (QChar)'0');
        t->setItem(i/8, i%8, new QTableWidgetItem(hex));

        QBrush brush = datatypeBrushes[(quint8)s->datatypes[i]];
        if (s->flags[i] & FLAG_USE_LABEL)
            brush.setStyle(Qt::Dense2Pattern);
        t->item(i/8, i%8)->setBackground(brush);

        if (!(i%8)) {
            hex = QString("%1").arg(s->start + i, 2, 16, (QChar)'0');
            t->setVerticalHeaderItem(i/8, new QTableWidgetItem(hex));
        }
    }
}

void MainWindow::showAscii(void) {
    const struct segment *s = &segments.at(currentSegment);
    QTableWidget *t = ui->tableASCII;
    QFont font;
    qint32 offset = 0;

    if (altfont == FONT_ATARI8BIT) {
        font = QFont("Atari Classic", 10, 1);
        offset = 0xe000;
    }
    if (altfont == FONT_C64) {
        font = QFont("C64 Pro Mono", 10, 1);
        offset = 0xe100;
    }

    t->setRowCount(0);

    qint64 size = s->end - s->start + 1;

    t->setRowCount(size/8+(size%8?1:0));

    quint8 *data = s->data;

    for (int i=0; i<size; i++) {
        t->setItem(i/8, i%8, new QTableWidgetItem(QChar(offset+data[i])));

        QBrush brush = datatypeBrushes[(quint8)s->datatypes[i]];
        if (s->flags[i] & FLAG_USE_LABEL)
            brush.setStyle(Qt::Dense2Pattern);
        t->item(i/8, i%8)->setBackground(brush);

        if (altfont)
            t->item(i/8, i%8)->setFont(font);
        if (!(i%8)) {
            QString hex = QString("%1").arg(s->start + i, 2, 16, (QChar)'0');
            t->setVerticalHeaderItem(i/8, new QTableWidgetItem(hex));
        }
    }
}

// --------------------------------------------------------------------------

void MainWindow::linkSelections(QTableWidget *from, QTableWidget *to) {
    QList<QTableWidgetSelectionRange> Ranges = from->selectedRanges();

    to->clearSelection();
    for (int i=0; i<Ranges.size(); i++) {
        to->setRangeSelected(Ranges.at(i), true);
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

// --------------------------------------------------------------------------

void MainWindow::Set_To_Foo(QList<QTableWidgetSelectionRange> Ranges,
                                                       quint8 datatype) {
    for (int i=0; i<Ranges.size(); i++) {
        QTableWidgetSelectionRange range = Ranges.at(i);
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
    Disassembler->generateDisassembly();
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

void MainWindow::Set_Flag(QList<QTableWidgetSelectionRange> Ranges, quint8 flag) {
    const struct segment *s = &segments.at(currentSegment);
    unsigned int pos;

    for (int i=0; i<Ranges.size(); i++) {
        QTableWidgetSelectionRange range = Ranges.at(i);
        for (int y = range.topRow(); y <= range.bottomRow(); y++) {
            for (int x = range.leftColumn(); x <= range.rightColumn(); x++) {
                pos = y*8+x;
                if (pos <= s->end)
                    s->flags[pos] |=  flag;
            }
        }
    }
    Disassembler->generateDisassembly();
    showHex();
    showAscii();
    showDisassembly();
}

void MainWindow::actionSet_Flag_Labelled(void) {
    Set_Flag(ui->tableHexadecimal->selectedRanges(), FLAG_USE_LABEL);
}

void MainWindow::actionSet_Flag_Low_Byte(void) {
    Set_Flag(ui->tableHexadecimal->selectedRanges(), FLAG_LOW_BYTE);
}

void MainWindow::actionSet_Flag_High_Byte(void) {
    Set_Flag(ui->tableHexadecimal->selectedRanges(), FLAG_HIGH_BYTE);
}

// --------------------------------------------------------------------------

void MainWindow::showDisassembly(void) {
    struct segment *s = &segments[currentSegment];
//    quint64 start = s->start, end = s->end, size = end - start + 1;
    QTableWidget *t = ui->tableDisassembly;
    QList<struct disassembly> *dislist  = &s->disassembly;
    QMap<quint64, QString> *comments     = &s->comments;
    struct disassembly dis;
    QString com;
    int row;

    t->setRowCount(0);

    qint64 di = 0;
    while (di < dislist->size()) {
        dis = dislist->at(di);

        if (comments->contains(dis.address)) {
            com = comments->value(dis.address);

            row = t->rowCount();
            t->setRowCount(row+1);
            QString hex = QString("%1").arg(dis.address, 4, 16, (QChar)'0');
            t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));
            t->verticalHeaderItem(row)->setTextAlignment(Qt::AlignTop);

            t->setSpan(row,1,1,2);

            t->setItem(row, 0, new QTableWidgetItem(";"));
            t->item(row,0)->setTextAlignment(Qt::AlignTop);

            t->setItem(row, 1, new QTableWidgetItem(com));
            t->item(row,1)->setForeground(Qt::gray); // XXX comment color
            t->resizeRowToContents(row);
        }

        if (   userLabels.contains(dis.address)
            || autoLabels.contains(dis.address)
            || s->localLabels.contains(dis.address)) {

            QString label;
            if (s->localLabels.contains(dis.address)) {
                label = s->localLabels.value(dis.address);
            } else if (userLabels.contains(dis.address)) {
                label = userLabels.value(dis.address);
            } else {
                label = autoLabels.value(dis.address);
            }

            // we do not print labels with + or -

            if (!label.contains(QChar('+')) && !label.contains(QChar('-'))) {
                row = t->rowCount();
                t->setRowCount(row+1);
                QString hex = QString("%1").arg(dis.address, 4, 16, (QChar)'0');
                t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));

                t->setSpan(row,0,1,3);
                t->setItem(row, 0, new QTableWidgetItem(label));
            }
        }

        row = t->rowCount();
        t->setRowCount(row+1);

        QString hex = QString("%1").arg(dis.address, 4, 16, (QChar)'0');
        t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));

        t->setItem(row, 1, new QTableWidgetItem(dis.instruction));
        t->setItem(row, 2, new QTableWidgetItem(dis.arguments));

        if (dis.instruction.at(0) == QString("."))
            t->item(row,1)->setForeground(QColor(0,96,0));

        if (dis.changes_pc) {
            row = t->rowCount();
            t->setRowCount(row+1);
            t->setVerticalHeaderItem(row, new QTableWidgetItem(hex));
        }

        di++;
    }
}

// --------------------------------------------------------------------------

void MainWindow::onHexSectionClicked(int index) {
    QTableWidget *th = ui->tableHexadecimal;
    QTableWidget *td = ui->tableDisassembly;
    QString s = th->verticalHeaderItem(index)->text();
    int n = td->rowCount();

    for (int i=0; i<n; i++) {
        if (td->verticalHeaderItem(i)->text() < s) continue;
        td->scrollToItem(td->item(i,1), QAbstractItemView::PositionAtCenter);
        break;
    }
}

void MainWindow::on_comboFonts_activated(int index) {
    altfont = (enum fonts) index;
    showAscii();
    linkSelections(ui->tableHexadecimal, ui->tableASCII);
}

// --------------------------------------------------------------------------

void MainWindow::onDisassemblySectionClicked(int index) {
    QTableWidget *th = ui->tableHexadecimal;
    QTableWidget *td = ui->tableDisassembly;
    QString s = td->verticalHeaderItem(index)->text();
    int n = th->rowCount();

    for (int i=0; i<n; i++) {
        if (th->verticalHeaderItem(i)->text() < s) continue;
        th->scrollToItem(th->item(i,1), QAbstractItemView::PositionAtCenter);
        break;
    }

    quint64 a = s.toULongLong(0,16), row, column;
    a -= segments[currentSegment].start;
    row    = a / 8;
    column = a % 8;

    QTableWidgetSelectionRange range(row, column, row, column);
    ui->tableHexadecimal->clearSelection();
    ui->tableHexadecimal->setRangeSelected(range, true);
    ui->tableASCII->clearSelection();
    ui->tableASCII->setRangeSelected(range, true);
}

// --------------------------------------------------------------------------

void MainWindow::actionTrace(void) {
    QTableWidget *t = ui->tableHexadecimal;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();
    int x = INT_MAX, y = INT_MAX;

    if (Ranges.isEmpty())
        return;

    for (int i=0; i<Ranges.size(); i++) {
        QTableWidgetSelectionRange range = Ranges.at(i);
        if (range.topRow() < y)
            y = range.topRow();
        if (range.leftColumn() < x)
            x = range.leftColumn();
    }
    int pos = y*8 + x;
    pos += segments[currentSegment].start;

    Disassembler->trace(pos);
    Disassembler->generateDisassembly();
    showHex();
    showAscii();
    showDisassembly();
}

// --------------------------------------------------------------------------

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

    commentwindow *cw = new commentwindow(s, c);
    cw->exec();
    c = cw->retrieveComment();
    if (c.isEmpty())
        segments[currentSegment].comments.remove(a);
    else
        segments[currentSegment].comments.insert(a, c);
    showDisassembly();
}

void MainWindow::on_labelsButton_clicked() {
    labelswindow lw;
    lw.exec();
    Disassembler->generateDisassembly();
    showHex();
    showAscii();
    showDisassembly();
}

void MainWindow::on_exitButton_clicked() {
    close();
}

void MainWindow::on_radioButtonFullscreen_toggled(bool checked) {
    checked ? this->showFullScreen() : this->showMaximized();
}

void MainWindow::on_comboBox_currentIndexChanged(int index) {
    QFont f = this->font();
    f.setPointSize(8+index);
    this->setFont(f);
}

void MainWindow::on_tableDisassembly_doubleClicked(const QModelIndex &index) {
    QTableWidget *t = ui->tableDisassembly;
    QTableWidgetItem *item = t->item(index.row(),0);

    if (!item) return;

    QString s = t->item(index.row(), 0)->text();

    if (s == ";") {
        actionComment();
        return;
    }
}
