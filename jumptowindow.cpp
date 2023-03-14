#include "jumptowindow.h"
#include "ui_jumptowindow.h"
#include <QDebug>

jumpToWindow::jumpToWindow(QWidget *parent, QStringList *list) :
    QDialog(parent),
    ui(new Ui::jumpToWindow)
{
    ui->setupUi(this);

    for (int i=0; i<list->size(); i++) {
        ui->listWidget->addItem(list->at(i));
    }
    ui->listWidget->item(0)->setSelected(true);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked,
            this, &jumpToWindow::accept);
}

jumpToWindow::~jumpToWindow()
{
    delete ui;
}

void jumpToWindow::accept() {
    jump_to_location = ui->listWidget->selectedItems().at(0)->text();
    setResult(QDialog::Accepted);
    hide();
}
