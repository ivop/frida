#include "lowandhighbytepairswindow.h"
#include "ui_lowandhighbytepairswindow.h"

LowAndHighBytePairsWindow::LowAndHighBytePairsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LowAndHighBytePairsWindow)
{
    ui->setupUi(this);
}

LowAndHighBytePairsWindow::~LowAndHighBytePairsWindow()
{
    delete ui;
}

void LowAndHighBytePairsWindow::accept() {
    pairsLowLowHighHigh = ui->radioLowLowHighHigh->isChecked();

    if (ui->radioNoLabels->isChecked())
        generateLabels = NoLabels;
    else if (ui->radioLocalLabels->isChecked())
        generateLabels = LocalLabels;
    else
        generateLabels = GlobalLabels;

    setResult(QDialog::Accepted);
    hide();
}
