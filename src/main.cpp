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
#include "mainwindow.h"
#include "startdialog.h"
#include "ui_mainwindow.h"

QVector<struct segment> segments;
int currentSegment;

QString globalNotes;

enum fonts altfont;

QSettings settings(QStringLiteral("frida"), QStringLiteral("frida"));
QPalette dark_palette, light_palette;

QMap<quint64, QString> globalAutoLabels, globalLabels;

int main(int argc, char *argv[])
{
#if defined(Q_OS_UNIX)
    unsetenv("QT_QPA_PLATFORMTHEME");       // STOP messing with MY fonts!
#endif

    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/AtariClassic-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/C64_Pro_Mono-STYLE.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/SpaceMono-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/DroidSansMono.ttf"));

    int pointSize = QApplication::font().pointSize();

    QFont font = QFont(QStringLiteral("Droid Sans Mono"), pointSize + 1, 0);

    QApplication::setStyle(QStringLiteral("fusion"));
    QApplication::setFont(font);

    dark_palette.setColor(QPalette::Window, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::WindowText, Qt::white);
    dark_palette.setColor(QPalette::Base, QColor(35, 35, 35));
    dark_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
    dark_palette.setColor(QPalette::ToolTipText, Qt::white);
    dark_palette.setColor(QPalette::Text, Qt::white);
    dark_palette.setColor(QPalette::BrightText, Qt::red);
    dark_palette.setColor(QPalette::Button, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::ButtonText, Qt::white);
    dark_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    dark_palette.setColor(QPalette::HighlightedText, Qt::black);
    dark_palette.setColor(QPalette::PlaceholderText, Qt::gray);

    light_palette.setColor(QPalette::Window, QColor(240, 240, 240));
    light_palette.setColor(QPalette::WindowText, Qt::black);
    light_palette.setColor(QPalette::Base, QColor(Qt::white));
    light_palette.setColor(QPalette::AlternateBase, QColor(248, 248, 248));
    light_palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    light_palette.setColor(QPalette::ToolTipText, Qt::black);
    light_palette.setColor(QPalette::Text, Qt::black);
    light_palette.setColor(QPalette::BrightText, Qt::red);
    light_palette.setColor(QPalette::Button, QColor(240, 240, 240));
    light_palette.setColor(QPalette::ButtonText, Qt::black);
    light_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    light_palette.setColor(QPalette::HighlightedText, Qt::white);
    light_palette.setColor(QPalette::PlaceholderText, Qt::gray);


    if (!settings.contains(QStringLiteral("Mode")))
        settings.setValue(QStringLiteral("Mode"), "light");
    if (!settings.contains(QStringLiteral("Fullscreen")))
        settings.setValue(QStringLiteral("Fullscreen"), "off");
    settings.sync();

    if (settings.value(QStringLiteral("Mode")) == "dark")
        QApplication::setPalette(dark_palette);
    else
        QApplication::setPalette(light_palette);

    StartDialog startdialog;
    startdialog.exec();

    if (!startdialog.create_new_project && !startdialog.load_existing_project)
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

    MainWindow mainwindow;
    mainwindow.show();

    return QApplication::exec();
}
