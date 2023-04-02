#include "constantsmanager.h"
#include "frida.h"
#include "ui_constantsmanager.h"

QMap<quint64, struct constantsGroup> constantsGroups;
quint64 nextNewGroup = 0;

constantsManager::constantsManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::constantsManager)
{
    ui->setupUi(this);

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

void constantsManager::showGroups() {
    QTableWidget *t = ui->tableGroups;
    QTableWidgetItem *item;

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
        t->setCurrentCell(0,0);
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

        item = new QTableWidgetItem(QString("%1").arg(val, 0, 16, QChar('0')));
        item->setTextAlignment(Qt::AlignTop);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        tv->setItem(row,0,item);

        item = new QTableWidgetItem(lbl);
        item->setTextAlignment(Qt::AlignTop);
        tv->setItem(row,1,item);

    }
}

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

void constantsManager::onValues_cellChanged(int row, int column) {
    QTableWidget *tg = ui->tableGroups;
    QTableWidget *tv = ui->tableValues;
    QTableWidgetItem *item, *index;

    // column is always 1 (second column)

    if (! tv->item(row, column)->isSelected()) return; // not a user action

    QList<QTableWidgetSelectionRange> ranges = tg->selectedRanges();
    QTableWidgetSelectionRange range = ranges.first();
    quint64 tgrow = range.topRow();

    quint64 groupID = tg->verticalHeaderItem(tgrow)->text().toULongLong();

    QMap<quint64, QString> *map = constantsGroups[groupID].map;

    index = tv->item(row, column-1);
    quint64 value = index->text().toULongLong();

    item = tv->item(row, column);
    QString contents = item->text();

    if (contents.isEmpty()) {
        item->setText(map->value(value));
    } else {
        map->insert(value, contents);
    }
}

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
