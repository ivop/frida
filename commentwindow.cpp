#include "commentwindow.h"
#include "ui_commentwindow.h"
#include <QDebug>

commentwindow::commentwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::commentwindow)
{
    ui->setupUi(this);
}

commentwindow::commentwindow(QString s, QString c, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::commentwindow)
{
    ui->setupUi(this);
    ui->label->setText("Comment at " + s);
    ui->plainTextEdit->setPlainText(c);
}

commentwindow::~commentwindow()
{
    delete ui;
}

QString commentwindow::retrieveComment(void) {
    return ui->plainTextEdit->toPlainText();
}
