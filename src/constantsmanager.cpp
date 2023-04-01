#include "constantsmanager.h"
#include "ui_constantsmanager.h"

QMap <quint64, QPair<QString, QMap <quint64, QString>>> constants;

constantsManager::constantsManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::constantsManager)
{
    ui->setupUi(this);

    connect(ui->tableGroups, &QTableWidget::itemSelectionChanged,
            this, &constantsManager::onGroups_itemSelectionChanged);
    /*
    quint64 id = 0;
    const QString &groupname = constants.value(id).first;
    const QMap <quint64, QString> &mapping = constants.value(id).second;
    quint64 constantvalue = 0;
    const QString &constantname = mapping.value(constantvalue);
    */


    showGroups();
}

constantsManager::~constantsManager()
{
    delete ui;
}

void constantsManager::showGroups() {
    QTableWidget *t = ui->tableGroups;

    t->setRowCount(0);
    t->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
}

void constantsManager::onGroups_itemSelectionChanged() {

}
