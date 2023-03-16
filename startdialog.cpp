#include "startdialog.h"
#include "ui_startdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "frida.h"
#include "loader.h"
#include "disassembler.h"
#include "loadsaveproject.h"

QString FileToDisassemble;
QString ProjectFileToSaveTo;

quint32 filetype, cputype;
Loader *Loader;

StartDialog::StartDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBrowseFileDisasm, &QPushButton::clicked,
            this, &StartDialog::onButtonBrowseFileDisasm_clicked);
    connect(ui->buttonLoadExistingProject, &QPushButton::clicked,
            this, &StartDialog::onButtonLoadExistingProject_clicked);
    connect(ui->buttonNewProject, &QPushButton::clicked,
            this, &StartDialog::onButtonNewProject_clicked);

    ui->comboFileType->clear();
    for (int i=0; i<filetypes.size(); i++) {
        ui->comboFileType->addItem(filetypes.at(i).name);
    }
    ui->comboFileType->setCurrentIndex(1);

    ui->comboCPUType->clear();
    for (int i=0; i<cputypes.size(); i++) {
        ui->comboCPUType->addItem(cputypes.at(i).name);
    }

    ui->labelVersion->setText(QString("Version: %1").arg(FRIDA_VERSION_STRING));
    ui->labelBuild->setText(QString("Build date: ") + QString(__DATE__) +
                            QString(" ") + QString(__TIME__));
}

StartDialog::~StartDialog()
{
    delete ui;
}

void StartDialog::onButtonBrowseFileDisasm_clicked()
{
    QString file = QFileDialog::getOpenFileName();
    if (!file.isEmpty())
        ui->lineFileDisasm->setText(file);
}

void StartDialog::onButtonLoadExistingProject_clicked()
{
    segments.clear();
    load_existing_project = load_project(this);
    if (load_existing_project) close();
}

void StartDialog::onButtonNewProject_clicked()
{
    if (ui->lineFileDisasm->text().isEmpty()) {
        QMessageBox msg;
        msg.setText("Some fields are left empty!");
        msg.exec();
        return;
    }

    QMessageBox msg;

    FileToDisassemble.clear();
    FileToDisassemble.append(ui->lineFileDisasm->text());

    ProjectFileToSaveTo.clear();
    // set filename to save to later

    filetype = ui->comboFileType->currentIndex();

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

    segments.clear();

    if (!Loader->Load(file)) {
        msg.setText("Failed to load " + FileToDisassemble + "\n"
                    + file.errorString()
                    + "\n\nFile type mismatch or corrupted file!\n");
        msg.exec();
        return;
    }

    file.close();

    if (!segments.size()) {
        msg.setText("File contains no segments (" + FileToDisassemble + ")");
        msg.exec();
        return;
    }

    // retrieve cputype from table as index is not necessarily the same
    // there can be holes in the enumeration

    cputype =  cputypes.at(ui->comboCPUType->currentIndex()).id;

    create_new_project = true;
    close();
}
