#include "constantsmanager.h"
#include "ui_constantsmanager.h"

QMap <quint64, QPair<QString, QMap <quint64, QString>>> constants;

constantsManager::constantsManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::constantsManager)
{
    ui->setupUi(this);


    quint64 id = 0;
    const QString &groupname = constants.value(id).first;
    const QMap <quint64, QString> &mapping = constants.value(0).second;
    quint64 constantvalue = 0;
    const QString &constantname = mapping.value(constantvalue);

}

constantsManager::~constantsManager()
{
    delete ui;
}
