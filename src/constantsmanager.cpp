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
#include "addconstanttogroupwindow.h"
#include "constantsmanager.h"
#include "ui_constantsmanager.h"

// Later, add BOTH constantsGroups and nextNewGroup to saved project!
// nextNewGroup could be derived from last groupID+1, but saving it is
// eassier.

QMap<quint64, struct constantsGroup> constantsGroups;
quint64 nextNewGroup = 0;

constantsManager::constantsManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::constantsManager)
{
    ui->setupUi(this);

    connect(ui->buttonAddGroup, &QPushButton::clicked,
            this, &constantsManager::onAddGroup_clicked);
    connect(ui->buttonAddValue, &QPushButton::clicked,
            this, &constantsManager::onAddValue_clicked);
    connect(ui->buttonImport, &QPushButton::clicked,
            this, &constantsManager::onImport_clicked);
    connect(ui->buttonExport, &QPushButton::clicked,
            this, &constantsManager::onExport_clicked);

    connect(ui->buttonDone, &QPushButton::clicked,
            this, &constantsManager::close);

    connect(ui->tableGroups, &QTableWidget::itemSelectionChanged,
            this, &constantsManager::onGroups_itemSelectionChanged);

    connect(ui->tableGroups, &QTableWidget::cellChanged,
            this, &constantsManager::onGroups_cellChanged);

    connect(ui->tableValues, &QTableWidget::cellChanged,
            this, &constantsManager::onValues_cellChanged);

    QTableWidget *tg = ui->tableGroups;
    tg->addAction(ui->actionDeleteGroup);

    QTableWidget *tv = ui->tableValues;
    tv->addAction(ui->actionDeleteValue);

    showGroups();
}

constantsManager::~constantsManager()
{
    delete ui;
}

// ---------------------------------------------------------------------------

void constantsManager::showGroups() {
    QTableWidget *t = ui->tableGroups;
    QTableWidgetItem *item;
    QList<QTableWidgetSelectionRange> ranges = t->selectedRanges();
    QTableWidgetSelectionRange range;
    qint64 tgrow = -1;

    if (!ranges.isEmpty()) {
        range = ranges.first();
        tgrow = range.topRow();
    }

    t->setRowCount(0);
    t->verticalHeader()->setDefaultAlignment(Qt::AlignRight);

    QMap<quint64, struct constantsGroup>::iterator iter;
    quint64 row;

    for (iter = constantsGroups.begin(); iter != constantsGroups.end(); iter++) {
        row = t->rowCount();
        t->setRowCount(row+1);

        quint64 groupID = iter.key();
        QString groupName = iter.value().name;

        QString vhi = QStringLiteral("%1").arg(groupID);
        t->setVerticalHeaderItem(row, new QTableWidgetItem(vhi));
        t->verticalHeaderItem(row)->setTextAlignment(Qt::AlignTop | Qt::AlignRight);

        item = new QTableWidgetItem(groupName);
        item->setTextAlignment(Qt::AlignTop);
        t->setItem(row, 0, item);
    }
    if (!constantsGroups.isEmpty()) {
        if (tgrow == t->rowCount())
            tgrow--;                    // last item was deleted
        if (tgrow >= 0)
            t->setCurrentCell(tgrow,0); // restore old position
        else if (t->rowCount() != 0)
            t->setCurrentCell(0,0);     // first call when table was empty
        t->setFocus();
    }
}

void constantsManager::onGroups_itemSelectionChanged() {
    QTableWidget *tg = ui->tableGroups;
    QTableWidget *tv = ui->tableValues;
    QTableWidgetItem *item;

    tv->setRowCount(0);

    qint64 row = tg->currentRow();
    if (row < 0) {
        return;
    }

    quint64 groupID = tg->verticalHeaderItem(row)->text().toULongLong();

    QMap<quint64, QString> *map = constantsGroups[groupID].map;

    QMap<quint64, QString>::iterator iter;

    for (iter = map->begin(); iter != map->end(); iter++) {
        row = tv->rowCount();
        tv->setRowCount(row+1);

        item = new QTableWidgetItem();
        tv->setVerticalHeaderItem(row, item);

        quint64 val = iter.key();
        QString lbl = iter.value();

        item = new QTableWidgetItem(QStringLiteral("%1").arg(val, 0, 16, QChar('0')));
        item->setTextAlignment(Qt::AlignTop);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        tv->setItem(row,0,item);

        item = new QTableWidgetItem(lbl);
        item->setTextAlignment(Qt::AlignTop);
        tv->setItem(row,1,item);

    }
}

// ---------------------------------------------------------------------------

void constantsManager::onGroups_cellChanged(int row, int column) {
    QTableWidget *tg = ui->tableGroups;
    QTableWidgetItem *item;

    if (! tg->item(row,column)->isSelected()) return; // not a user action

    quint64 groupID = tg->verticalHeaderItem(row)->text().toULongLong();

    item = tg->item(row, column);
    QString contents = item->text();

    if (contents.isEmpty()) {
        item->setText(constantsGroups[groupID].name);
    } else {
        constantsGroups[groupID].name = contents;
    }
}

// ---------------------------------------------------------------------------

void constantsManager::onValues_cellChanged(int row, int column) {
    QTableWidget *tg = ui->tableGroups;
    QTableWidget *tv = ui->tableValues;
    QTableWidgetItem *item;
    QTableWidgetItem *index;

    // column is always 1 (second column)

    if (! tv->item(row, column)->isSelected()) return; // not a user action

    QList<QTableWidgetSelectionRange> ranges = tg->selectedRanges();
    QTableWidgetSelectionRange range = ranges.first();
    quint64 tgrow = range.topRow();

    quint64 groupID = tg->verticalHeaderItem(tgrow)->text().toULongLong();

    QMap<quint64, QString> *map = constantsGroups[groupID].map;

    index = tv->item(row, column-1);
    quint64 value = index->text().toULongLong(nullptr, 16);

    item = tv->item(row, column);
    QString contents = item->text();

    if (contents.isEmpty()) {
        item->setText(map->value(value));
    } else {
        map->insert(value, contents);
    }
}

// ---------------------------------------------------------------------------

void constantsManager::actionDeleteGroup(void) {
    QTableWidget *t = ui->tableGroups;

    QList<QTableWidgetSelectionRange> ranges = t->selectedRanges();

    if (ranges.isEmpty())
        return;

    QTableWidgetSelectionRange range = ranges.first();
    quint64 tgrow = range.topRow();

    quint64 groupID = t->verticalHeaderItem(tgrow)->text().toULongLong();
    QString groupName = constantsGroups[groupID].name;

    QMessageBox msg;
    msg.setText("Do you really want to delete " + groupName + "?");
    msg.setStandardButtons(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    msg.setDefaultButton(QMessageBox::No);
    if(msg.exec() != QMessageBox::Yes) return;

    constantsGroups.remove(groupID);
    showGroups();
}

// ---------------------------------------------------------------------------

void constantsManager::actionDeleteValue(void) {
    QTableWidget *tg = ui->tableGroups;
    QTableWidget *tv = ui->tableValues;

    QList<QTableWidgetSelectionRange> ranges = tg->selectedRanges();

    if (ranges.isEmpty())
        return;

    QTableWidgetSelectionRange range = ranges.first();
    quint64 tgrow = range.topRow();

    quint64 groupID = tg->verticalHeaderItem(tgrow)->text().toULongLong();

    ranges = tv->selectedRanges();
    if (ranges.isEmpty())
        return;

    range = ranges.first();
    quint64 tvrow = range.topRow();

    quint64 value = tv->item(tvrow,0)->text().toULongLong(nullptr,16);
    QString label = tv->item(tvrow,1)->text();

    QMessageBox msg;
    msg.setText(QString("Do you really want to delete (0x%1," + label + ")?").arg(value,0,16));
    msg.setStandardButtons(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    msg.setDefaultButton(QMessageBox::No);
    if(msg.exec() != QMessageBox::Yes) return;

    QMap<quint64, QString> *map = constantsGroups[groupID].map;

    map->remove(value);
    onGroups_itemSelectionChanged();    // reshow constants with one removed
}

// ---------------------------------------------------------------------------

void constantsManager::onAddGroup_clicked(void) {
    addConstantsGroupWindow acgw;
    if (acgw.exec() == QDialog::Accepted)
        showGroups();
}

// ---------------------------------------------------------------------------

void constantsManager::onAddValue_clicked(void) {
    QMessageBox msg;
    addConstantToGroupWindow actgw;

    QTableWidget *t = ui->tableGroups;
    QList<QTableWidgetSelectionRange> ranges = t->selectedRanges();

    if (ranges.isEmpty()) {
        msg.setText(QStringLiteral("Please select a group first!"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }

    if (actgw.exec() == QDialog::Rejected)
        return;

    QTableWidgetSelectionRange range = ranges.first();
    quint64 tgrow = range.topRow();

    quint64 groupID = t->verticalHeaderItem(tgrow)->text().toULongLong();

    if (actgw.constantValue < 0) {
        msg.setText(QStringLiteral("Invalid value specified!"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }

    QMap<quint64, QString> *map = constantsGroups[groupID].map;
    map->insert(actgw.constantValue, actgw.constantName);
    onGroups_itemSelectionChanged();    // reshow constants with new item
}

// ---------------------------------------------------------------------------

void constantsManager::onImport_clicked(void) {
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
        msg.setText("Failed to open " + name + "\n" + file.errorString());
        msg.exec();
        return;
    }

    QTextStream in(&file);

    QString groupName;

    // "parser" instead of serialization so input can easily be prepared
    // in a normal text editor like vi(m).

    while (true) {
        in.skipWhiteSpace();

        groupName = in.readLine();

        if (in.status() != QTextStream::Ok)
            goto errout;

        if (groupName.at(0) != QChar('['))
            goto errout;

        if (groupName == QStringLiteral("[]"))
            break;
        if (groupName.isEmpty())
            break;

        groupName = groupName.replace(QStringLiteral("["), QLatin1String(""));
        groupName = groupName.replace(QStringLiteral("]"), QLatin1String(""));

        auto *group = new struct constantsGroup;

        group->name = groupName;
        group->map = new QMap<quint64, QString>;

        constantsGroups.insert(nextNewGroup, *group);

        nextNewGroup++;

        while (true) {
            quint64 key;
            QString value;

            qint64 savepos = in.pos();

            in.skipWhiteSpace();

            in >> key;

            if (in.status() == QTextStream::ReadCorruptData) {
                in.resetStatus();
                in.seek(savepos);
                break;
            }

            in >> value;

            if (value.isEmpty())  // empty line
                break;

            group->map->insert(key,value);
        }
    }

    if (in.status() != QTextStream::Ok) {
errout:
        msg.setText(QStringLiteral("Errors occurred during import!\n\nFile might be corrupt."));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        file.close();
        showGroups();
        return;
    }

    msg.setText(QStringLiteral("Successfully imported constants!"));
    msg.setStandardButtons(QMessageBox::Ok);
    msg.exec();

    file.close();
    showGroups();
}

// ---------------------------------------------------------------------------

void constantsManager::onExport_clicked(void) {
    QString name = QFileDialog::getSaveFileName(this, QStringLiteral("Export Constants As..."));

    if (name.isEmpty()) return;

    QMessageBox msg;

    QFile file(name);

    file.open(QIODevice::WriteOnly);
    if (!file.isOpen()) {
        msg.setText("Failed to open " + name + "\n\n" + file.errorString());
        msg.exec();
        return;
    }

    QTextStream out(&file);

    for (const auto &group : qAsConst(constantsGroups)) {
        out << "[" << group.name << "]" << Qt::endl;

        QMap<quint64, QString>::iterator iter;

        for (iter = group.map->begin(); iter != group.map->end(); iter++) {
            out << Qt::hex << Qt::showbase << iter.key() << " "
                << iter.value() << Qt::endl;
        }

        out << Qt::endl;    // for readability
    }

    out << "[]" << Qt::endl;        // mark EOF

    if (out.status() != QTextStream::Ok) {
        msg.setText(QStringLiteral("Exporting constants failed!\n\nWrite error!"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        file.close();
        return;
    }

    msg.setText(QStringLiteral("Successfully exported constants!"));
    msg.setStandardButtons(QMessageBox::Ok);
    msg.exec();

    file.close();
}

// ---------------------------------------------------------------------------
