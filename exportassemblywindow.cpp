#include "exportassemblywindow.h"
#include "ui_exportassemblywindow.h"
#include <QDebug>

exportAssemblyWindow::exportAssemblyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::exportAssemblyWindow)
{
    ui->setupUi(this);
}

exportAssemblyWindow::~exportAssemblyWindow()
{
    delete ui;
}

void exportAssemblyWindow::accept() {
    asm_format = ASM_FORMAT_VERBATIM;

    if (ui->mads_assembler->isChecked())
        asm_format = ASM_FORMAT_MADS;

    setResult(QDialog::Accepted);
    hide();
}
