#include "constantsmanager.h"
#include "ui_constantsmanager.h"

struct constantsGroup {
    QString name;
    QMap<quint64, QString> *map;
};

QMap<quint64, struct constantsGroup> constantsGroups;

constantsManager::constantsManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::constantsManager)
{
    ui->setupUi(this);

    connect(ui->buttonDone, &QPushButton::clicked,
            this, &constantsManager::close);

    connect(ui->tableGroups, &QTableWidget::itemSelectionChanged,
            this, &constantsManager::onGroups_itemSelectionChanged);

    // test... {

    quint64 groupID = 123;
    struct constantsGroup * cg = new struct constantsGroup;

    cg->name = "Boolean";
    cg->map = new QMap<quint64,QString>;

    constantsGroups.insert(groupID, *cg);

    auto map = constantsGroups[groupID].map;

    map->insert(0, "FALSE");
    map->insert(1, "TRUE");

    cg = new struct constantsGroup;

    cg->name = "Primes";
    cg->map = new QMap<quint64,QString>;

    constantsGroups.insert(groupID+1, *cg);

    map = constantsGroups[groupID+1].map;

    map->insert(13, "thirteen");
    map->insert(3, "three");
    map->insert(5, "five");
    map->insert(11, "eleven");
    map->insert(7, "seven");

    // } test

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
    QTableWidget *tv = ui->tableConstantValues;
    QTableWidgetItem *item;

    quint64 row = tg->currentRow();
    quint64 groupID = tg->verticalHeaderItem(row)->text().toULongLong();

    tv->setRowCount(0);

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
        tv->setItem(row,0,item);

        item = new QTableWidgetItem(lbl);
        item->setTextAlignment(Qt::AlignTop);
        tv->setItem(row,1,item);

    }
}
