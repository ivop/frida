#include "addlabelwindow.h"
#include "ui_addlabelwindow.h"
#include <QDebug>
#include <QMessageBox>
#include "frida.h"

addLabelWindow::addLabelWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addLabelWindow)
{
    ui->setupUi(this);
}

addLabelWindow::~addLabelWindow()
{
    delete ui;
}

void addLabelWindow::on_pushButtonCancel_clicked() {
    close();
}

void addLabelWindow::on_pushButtonAdd_clicked() {
    QMessageBox msg;
    QString address = ui->lineEditAddress->text();
    QString label   = ui->lineEditLabel->text();
    bool ok;
    quint64 addr = address.toULongLong(&ok,16);

    if (!ok) {
        msg.setText("Invalid address!");
        msg.exec();
    } else if (label.isEmpty()) {
        msg.setText("No label specified!");
        msg.exec();
    } else {
        userLabels.insert(addr, label);
        close();
    }
}
