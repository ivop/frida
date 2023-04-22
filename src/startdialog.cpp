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

#include "architecture.h"
#include "compiler.h"
#include "disassembler.h"
#include "loaders.h"
#include "loadsaveproject.h"
#include "platform.h"
#include "startdialog.h"
#include "ui_startdialog.h"

QString FileToDisassemble;

QString AuthorString;
QString VersionString;
QString BuildString;

QString ArchitectureString;
QString PlatformString;
QString CompilerString;
QString QtVersionString;
QString DateTimeString;

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
    for (const auto & filetype : qAsConst(filetypes)) {
        ui->comboFileType->addItem(filetype.name);
    }
    ui->comboFileType->setCurrentIndex(1);

    ui->comboCPUType->clear();
    for (const auto & cputype : cputypes) {
        ui->comboCPUType->addItem(cputype.name);
    }

    ArchitectureString = QStringLiteral(ARCHITECTURE);
    PlatformString = QStringLiteral(PLATFORM);
    CompilerString = QStringLiteral("%1 %2").arg(COMPILER).arg(COMPILER_VERSION);
    QtVersionString = QStringLiteral("Qt %1.%2.%3").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR).arg(QT_VERSION_PATCH);
    DateTimeString = QStringLiteral("%1 %2").arg(__DATE__).arg(__TIME__);

    AuthorString = QStringLiteral("Copyright © 2017,2023 by Ivo van Poorten");
    VersionString = QStringLiteral("Version: %1").arg(QStringLiteral(FRIDA_VERSION_STRING));
    BuildString = QStringLiteral("Build: %1, %2, %3, %4, %5").arg(PlatformString, ArchitectureString, CompilerString, QtVersionString, DateTimeString);

    ui->labelAuthor->setText(AuthorString);
    ui->labelVersion->setText(VersionString);
    ui->labelBuild->setText(BuildString);
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
    QMessageBox msg;

    if (ui->lineFileDisasm->text().isEmpty()) {
        msg.setText(QStringLiteral("Some fields are left empty!"));
        msg.exec();
        return;
    }


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
    case FT_ATARI8BIT_CAR:
        Loader = new LoaderAtari8bitCar();
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
    case FT_BBC_UEF_TAPE:
        Loader = new LoaderBBCUEFTape();
        break;
    case FT_ZX_SPECTRUM_TAP:
        Loader = new LoaderZXSpectrumTape();
        break;
    default:
        msg.setText(QStringLiteral("Unknown filetype! (this shouldn't happen)"));
        msg.exec();
        return;
    }

    segments.clear();

    if (!Loader->Load(file)) {
        QString text = QString("Failed to load " + FileToDisassemble + "\n\n");
        if (Loader->error_message.isEmpty()) {
            text += QStringLiteral("File type mismatch or corrupted file!\n");
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

    // name empty segments

    for (int i = 0; i < segments.size(); i++) {
        if (segments[i].name.isEmpty())
            segments[i].name = QString(QStringLiteral("Segment %1")).arg(i);
    }

    // retrieve cputype from table as index is not necessarily the same
    // there can be holes in the enumeration

    cputype =  cputypes.at(ui->comboCPUType->currentIndex()).id;

    create_new_project = true;
    close();
}
