// ---------------------------------------------------------------------------
//
// This file is part of:
//
// FRIDA - FRee Interactive DisAssembler
// Copyright (C) 2017,2023 Ivo van Poorten
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; ONLY version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ---------------------------------------------------------------------------

#include "startdialog.h"
#include "disassembler.h"
#include "frida.h"
#include "loader.h"
#include "loadsaveproject.h"
#include "ui_startdialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

QString FileToDisassemble;

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
    case FT_ATARI8BIT_SAP:
        Loader = new LoaderAtari8bitSAP();
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
    case FT_ATARI2600_2K4K:
        Loader = new LoaderAtari2600ROM2K4K();
         break;
    case FT_ORIC_TAP:
        Loader = new LoaderOricTap();
        break;
    case FT_APPLE2_DOS33:
        Loader = new LoaderApple2DOS33();
        break;
    case FT_APPLE2_APPLESINGLE:
        Loader = new LoaderApple2AppleSingle();
        break;
    case FT_NES_SONG_FILE:
        Loader = new LoaderNESSongFile();
        break;
    case FT_CPM_BINARY:
        Loader = new LoaderCPMBinary();
        break;
    default:
        msg.setText("Unknown filetype! (this shouldn't happen)");
        msg.exec();
        return;
    }

    segments.clear();

    if (!Loader->Load(file)) {
        QString text = QString("Failed to load " + FileToDisassemble + "\n\n");
        if (Loader->error_message.isEmpty()) {
            text += "File type mismatch or corrupted file!\n";
        } else {
            text += Loader->error_message;
        }
        msg.setText(text);
        msg.exec();
        return;
    }

    file.close();

    if (segments.empty()) {
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