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
#include "labelswindow.h"
#include "ui_labelswindow.h"

labelswindow::labelswindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::labelswindow)
{
    QTableWidget *t;
    ui->setupUi(this);

    t = ui->tableGlobalLabels;
    t->addAction(ui->actionChange_To_Local_Label);
    t->addAction(ui->actionDelete_Label);

    t = ui->tableLocalLabels;
    t->addAction(ui->actionChange_To_Global_Label);
    t->addAction(ui->actionDelete_Label);

    connect(ui->tableGlobalLabels, &QTableWidget::cellChanged,
            this, &labelswindow::onTableGlobalLabels_cellChanged);
    connect(ui->tableLocalLabels, &QTableWidget::cellChanged,
            this, &labelswindow::onTableLocalLabels_cellChanged);

    connect(ui->doneButton, &QPushButton::clicked,
            this, &labelswindow::onDoneButton_clicked);
    connect(ui->addLabelButton, &QPushButton::clicked,
            this, &labelswindow::onAddLabelButton_clicked);
    connect(ui->exportButton, &QPushButton::clicked,
            this, &labelswindow::onExportButton_clicked);
    connect(ui->importButton, &QPushButton::clicked,
            this, &labelswindow::onImportButton_clicked);

    showGlobalLabels();
    showLocalLabels();
}

labelswindow::~labelswindow()
{
    delete ui;
}

void labelswindow::onDoneButton_clicked() {
    close();
}

//-----------------------------------------------------------------------------
// RENDER TABLES OF LABELS

void labelswindow::showLabels(QTableWidget *t,
                                     QMap<quint64, QString> *labels) {
    QList<quint64> keys = labels->keys();
    QString hex;
    QTableWidgetItem *item;
    int n = keys.size();

    t->setRowCount(0);
    t->setRowCount(n);

    for (int i=0; i<n; i++) {
        hex = QStringLiteral("%1").arg(keys[i], 4, 16, QChar('0'));
        item = new QTableWidgetItem(hex);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(i,0, item);
        t->setItem(i,1, new QTableWidgetItem(labels->value(keys[i])));
    }
}

void labelswindow::showGlobalLabels(void) {
    showLabels(ui->tableGlobalLabels, &globalLabels);
}

void labelswindow::showLocalLabels(void) {
    showLabels(ui->tableLocalLabels, &segments[currentSegment].localLabels);
}

// ---------------------------------------------------------------------------
// INLINE EDITTING

static void get_contents(QTableWidget *t, int row, QString *label,
                                                        quint64 *address) {
    QString addr  = t->item(row, 0)->text();
    *label = t->item(row, 1)->text();
    *address = addr.toULongLong(0, 16);
}

void labelswindow::onTableGlobalLabels_cellChanged(int row, int column) {
    QTableWidget *t = ui->tableGlobalLabels;
    QString label;
    quint64 address;

    if (!t->item(row,column)->isSelected()) return; // not by user action

    get_contents(t, row, &label, &address);

    if (label.isEmpty()) {
        QString txt = globalLabels.value(address);
        t->item(row,column)->setText(txt);
        return;
    }

    globalLabels.insert(address, label);
    showGlobalLabels();
    t->setFocus();
    t->setCurrentCell(row, 1);
}

void labelswindow::onTableLocalLabels_cellChanged(int row, int column) {
    struct segment *s = &segments[currentSegment];
    QTableWidget *t = ui->tableLocalLabels;
    QString label;
    quint64 address;

    if (!t->item(row,column)->isSelected()) return; // not by user action

    get_contents(t, row, &label, &address);

    if (label.isEmpty()) {
        QString txt = s->localLabels.value(address);
        t->item(row,column)->setText(txt);
        return;
    }

    s->localLabels.insert(address, label);
    showLocalLabels();
    t->setFocus();
    t->setCurrentCell(row, 1);
}

//-----------------------------------------------------------------------------
// CHANGE BETWEEN USER AND LOCAL

void labelswindow::actionChange_To_Local_Label() {
    QTableWidget *t = ui->tableGlobalLabels;
    QString label;
    quint64 address;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();

    if (Ranges.isEmpty()) return;

    // UI is setup properly and only one cell is selected

    int row = Ranges.at(0).topRow();

    get_contents(t, row, &label, &address);

    globalLabels.remove(address);
    segments[currentSegment].localLabels.insert(address, label);
    showGlobalLabels();
    showLocalLabels();
    t->setFocus();
    t->setCurrentCell(row, 1);
}

void labelswindow::actionChange_To_Global_Label() {
    QTableWidget *t = ui->tableLocalLabels;
    QString label;
    quint64 address;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();

    if (Ranges.isEmpty()) return;

    // UI is setup properly and only one cell is selected

    int row = Ranges.at(0).topRow();

    get_contents(t, row, &label, &address);

    segments[currentSegment].localLabels.remove(address);
    globalLabels.insert(address, label);
    showGlobalLabels();
    showLocalLabels();
    t->setFocus();
    t->setCurrentCell(row, 1);
}

//-----------------------------------------------------------------------------
// DELETE LABEL

void labelswindow::actionDelete_Label() {
    QTableWidget *t;
    QMap<quint64,QString> *labels;
    QString label;
    quint64 address;

    if (ui->tableGlobalLabels->hasFocus()) {
        t = ui->tableGlobalLabels;
        labels = &globalLabels;
    } else if (ui->tableLocalLabels->hasFocus()) {
        t = ui->tableLocalLabels;
        labels = &segments[currentSegment].localLabels;
    } else {
        return;
    }

    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();
    if (Ranges.isEmpty())
        return;

    // UI is setup properly and only one cell is selected

    int row = Ranges.at(0).topRow();

    get_contents(t, row, &label, &address);


    QMessageBox msg;
    msg.setText("Do you really want to delete " + label);
    msg.setStandardButtons(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    msg.setDefaultButton(QMessageBox::No);
    if(msg.exec() != QMessageBox::Yes) return;

    labels->remove(address);
    showLabels(t, labels);
}

void labelswindow::onAddLabelButton_clicked() {
    addLabelWindow alw;
    alw.exec();
    showGlobalLabels();
    showLocalLabels();
}

// ---------------------------------------------------------------------------
// EXPORT

void labelswindow::onExportButton_clicked() {
    QString name = QFileDialog::getSaveFileName(this, QStringLiteral("Export labels to..."));

    if (name.isEmpty()) return;

    QFile file(name);

    QMessageBox msg;

    file.open(QIODevice::WriteOnly);
    if (!file.isOpen()) {
        msg.setText(QStringLiteral("Failed to open ") + name + QStringLiteral("\n") + file.errorString());
        msg.exec();
        return;
    }

    QTextStream out(&file);

    QList<quint64> keys = globalLabels.keys();
    for (quint64 key : keys) {
        out << Qt::hex << Qt::showbase << key << " "
            << globalLabels.value(key) << Qt::endl;
        if (out.status() != QTextStream::Ok)
            break;
    }

    if (file.error()) {
        msg.setText(QStringLiteral("Error: ") + file.errorString());
    } else if (out.status() != QTextStream::Ok) {
        msg.setText(QStringLiteral("Error writing file to disk!"));
    } else {
        msg.setText(QStringLiteral("Labels exported successfully!"));
    }

    file.close();
    msg.exec();
}

//-----------------------------------------------------------------------------
// IMPORT

void labelswindow::onImportButton_clicked() {
    QString name = QFileDialog::getOpenFileName(this, QStringLiteral("Import labels from..."));

    if (name.isEmpty()) return;

    QMessageBox msg;

    QFile file(name);
    if (!file.exists()) {
        msg.setText(QStringLiteral("No such file!"));
        msg.exec();
        return;
    }

    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        msg.setText(QStringLiteral("Failed to open ") + name + QStringLiteral("\n") + file.errorString());
        msg.exec();
        return;
    }

    QTextStream in(&file);

    quint64 addr;
    QString label;

    msg.setText(QStringLiteral("Labels imported successfully!"));

    while (!in.atEnd()) {
        in >> addr >> label;
        if (in.atEnd()) break;
        if (in.status() != QTextStream::Ok) {
            msg.setText(QStringLiteral("Error reading file ") + name);
            break;
        }
        if (!label.isEmpty())
            globalLabels.insert(addr, label);
    }
    file.close();
    msg.exec();
    showGlobalLabels();
}
