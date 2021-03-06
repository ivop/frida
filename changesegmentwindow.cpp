#include "changesegmentwindow.h"
#include "ui_changesegmentwindow.h"
#include <QMessageBox>
#include <QDebug>

changesegmentwindow::changesegmentwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::changesegmentwindow)
{
    ui->setupUi(this);
}

changesegmentwindow::changesegmentwindow(quint64 oldaddr, quint64 *newaddr,
                                         QWidget *parent) :
    QDialog(parent),
    ui(new Ui::changesegmentwindow)
{
    ui->setupUi(this);
    QString hex = QString("%1").arg(oldaddr, 4, 16, (QChar)'0');
    ui->oldAddress->setText(hex);
    newaddress = newaddr;
    ui->newAddress->setText(hex);
}

changesegmentwindow::~changesegmentwindow()
{
    delete ui;
}

void changesegmentwindow::on_buttonBox_accepted()
{
    bool ok;
    quint64 n = ui->newAddress->text().toULongLong(&ok, 16);
    if (ok) {
        *newaddress = n;
    } else {
        QMessageBox msg;
        msg.setText("Invalid address!");
        msg.setStandardButtons(QMessageBox::Close);
        msg.exec();
        ui->newAddress->setText(ui->oldAddress->text());
    }
}
