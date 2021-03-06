#include "startdialog.h"
#include "ui_startdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "frida.h"
#include "loader.h"
#include "disassembler.h"

QString FileToDisassemble;
QString ProjectFileToSaveTo;

int filetype, cputype;
Loader *Loader;
Disassembler *Disassembler;

StartDialog::StartDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartDialog)
{
    ui->setupUi(this);

    ui->comboFileType->clear();
    for (int i=0; i<filetypes.size(); i++) {
        ui->comboFileType->addItem(filetypes.at(i).name);
    }
    ui->comboFileType->setCurrentIndex(1);

    ui->comboCPUType->clear();
    for (int i=0; i<cputypes.size(); i++) {
        ui->comboCPUType->addItem(cputypes.at(i).name);
    }

    ui->labelBuild->setText(QString("build date: ") + QString(__DATE__) +
                            QString(" ") + QString(__TIME__));
}

StartDialog::~StartDialog()
{
    delete ui;
}

void StartDialog::on_buttonBrowseExistingProject_clicked()
{
    QString file = QFileDialog::getOpenFileName();
    if (!file.isEmpty())
        ui->lineExistingProject->setText(file);
}

void StartDialog::on_buttonBrowseFileDisasm_clicked()
{
    QString file = QFileDialog::getOpenFileName();
    if (!file.isEmpty())
        ui->lineFileDisasm->setText(file);
}

void StartDialog::on_buttonBrowseSaveFilename_clicked()
{
    QString file = QFileDialog::getSaveFileName();
    if (!file.isEmpty())
        ui->lineSaveDir->setText(file);
}

void StartDialog::on_buttonLoadExistingProject_clicked()
{
    qDebug() << "Open Existing Project";
    load_existing_project = true;
}

void StartDialog::on_buttonNewProject_clicked()
{
    if (   ui->lineSaveDir->text().isEmpty()
        || ui->lineFileDisasm->text().isEmpty()) {
        QMessageBox msg;
        msg.setText("Some fields are left empty!");
        msg.exec();
        return;
    }

    QMessageBox msg;

    FileToDisassemble.clear();
    FileToDisassemble.append(ui->lineFileDisasm->text());

    ProjectFileToSaveTo.clear();
    ProjectFileToSaveTo.append(ui->lineSaveDir->text());

    filetype = ui->comboFileType->currentIndex();
    cputype  = ui->comboCPUType->currentIndex();

    // Open file

    QFile file(FileToDisassemble);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        msg.setText("Failed to open " + FileToDisassemble);
        msg.exec();
        return;
    }

    // Select Loader type

    switch(filetypes.at(filetype).id) {
    case FT_RAW_FILE:
        Loader = new LoaderRaw();
        break;
    case FT_ATARI8BIT_BINARY:
        Loader = new LoaderAtari8bitBinary();
        altfont = FONT_ATARI8BIT;
        break;
    case FT_C64_BINARY:
        Loader = new LoaderC64Binary();
        altfont = FONT_C64;
        break;
    case FT_C64_PSID:
        Loader = new LoaderC64PSID();
        altfont = FONT_C64;
        break;
    default:
        msg.setText("Unknown filetype! (this shouldn't happen)");
        msg.exec();
        return;
    }

    if (!Loader->Load(file)) {
        msg.setText("Failed to load " + FileToDisassemble + "\n" + file.errorString());
        msg.exec();
        return;
    }

    file.close();

    if (!segments.size()) {
        msg.setText("File contains no segments (" + FileToDisassemble + ")");
        msg.exec();
        return;
    }

    // Select Disassembler (CPU Type)

    switch(cputypes.at(cputype).id) {
    case CT_NMOS6502:
        Disassembler = new Disassembler6502();
        break;
    default:
        msg.setText("Unknown cputype! (this shouldn't happen)");
        msg.exec();
        return;
    }

    create_new_project = true;
    close();
}
