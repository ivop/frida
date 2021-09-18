#include "lowhighbytewindow.h"
#include "ui_lowhighbytewindow.h"
#include <QDebug>
#include <QMessageBox>

lowhighbytewindow::lowhighbytewindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lowhighbytewindow)
{
    ui->setupUi(this);
}

lowhighbytewindow::lowhighbytewindow(quint64 location, bool low, quint8 byte,
                  quint16 *full, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lowhighbytewindow)
{
    ui->setupUi(this);

    ui->location->setText(QString("%1").arg(location, 4, 16, (QChar)'0'));
    if (low)
        ui->lowhighbyteLabel->setText(QString("Low Byte"));
    else
        ui->lowhighbyteLabel->setText(QString("High Byte"));
    ui->lowhighbyte->setText(QString("%1").arg(byte, 2, 16, (QChar)'0'));
    fulladdr = full;

    ui->fullAddress->setFocus();
}

lowhighbytewindow::~lowhighbytewindow()
{
    delete ui;
}

void lowhighbytewindow::on_buttonBox_accepted()
{
    bool ok;
    quint64 addr = ui->fullAddress->text().toULongLong(&ok, 16);

    if (ok && addr <= 0xffff) {
        *fulladdr = addr;
        accept();
    } else {
        QMessageBox msg;
        if (addr > 0xffff)
            msg.setText("Address larger than 16-bit");
        else
            msg.setText("Invalid address");
        msg.exec();
    }
}
