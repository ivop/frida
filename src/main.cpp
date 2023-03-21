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

#include "disassembler.h"
#include "frida.h"
#include "mainwindow.h"
#include "startdialog.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QFile>
#include <QFontDatabase>
#include <QHash>
#include <QMessageBox>
#include <QTextStream>

QVector<struct segment> segments;
int currentSegment;

QString globalNotes;

enum fonts altfont;

// use QMap because they are always sorted according to the documentation

QMap<quint64, QString> autoLabels, userLabels;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/AtariClassic-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/C64_Pro_Mono-STYLE.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/SpaceMono-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/DroidSansMono.ttf"));
    QFont font(QStringLiteral("Droid Sans Mono"), 10);

    a.setStyle(QStringLiteral("fusion"));
    a.setFont(font);

    StartDialog f;
    f.exec();

    if (!f.create_new_project && !f.load_existing_project)
        return 0;

    // Select Disassembler as set in start dialog or loaded from project

    switch(cputype) {
    case CT_NMOS6502:
        [[fallthrough]];
    case CT_NMOS6502UNDEF:
        [[fallthrough]];
    case CT_CMOS65C02:
        Disassembler = new Disassembler6502();
        break;
    case CT_INTEL_8080:
        Disassembler = new Disassembler8080();
    default:
        break;      // should not happen
    }

    Disassembler->cputype = cputype;        // be able to detect variants

    MainWindow m;
    m.showMaximized();
    return a.exec();
}
