#include "lowandhighbytepairswindow.h"
#include "ui_lowandhighbytepairswindow.h"

LowAndHighBytePairsWindow::LowAndHighBytePairsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LowAndHighBytePairsWindow)
{
    ui->setupUi(this);

    connect(ui->radioNoLabels, &QRadioButton::toggled,
            this, &LowAndHighBytePairsWindow::onNoLabels_toggled);
}

LowAndHighBytePairsWindow::~LowAndHighBytePairsWindow()
{
    delete ui;
}

void LowAndHighBytePairsWindow::accept() {
    pairsLowLowHighHigh = ui->radioLowLowHighHigh->isChecked();
    minusLabels = ui->checkMinusOne->isChecked();

    if (ui->radioNoLabels->isChecked())
        generateLabels = NoLabels;
    else if (ui->radioLocalLabels->isChecked())
        generateLabels = LocalLabels;
    else
        generateLabels = GlobalLabels;

    setResult(QDialog::Accepted);
    hide();
}

void LowAndHighBytePairsWindow::onNoLabels_toggled() {
    if (ui->radioNoLabels->isChecked())
        ui->checkMinusOne->setHidden(true);
    else
        ui->checkMinusOne->setHidden(false);
}
