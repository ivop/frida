#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startdialog.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QHash>
#include <QDesktopWidget>
#include "disassembler.h"
#include "frida.h"

QList<struct segment> segments;
int currentSegment;

QString globalNotes;

enum fonts altfont;

// use QMap because they are always sorted according to the documentation

QMap<quint64, QString> autoLabels, userLabels;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/AtariClassic-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/C64_Pro_Mono-STYLE.ttf");
    QFontDatabase::addApplicationFont(":/fonts/SpaceMono-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/DroidSansMono.ttf");
    QFont font("Droid Sans Mono", 10);

    a.setStyle("fusion");
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
        Disassembler = new Disassembler6502();
        break;
    }

    Disassembler->cputype = cputype;        // be able to detect variants

    MainWindow m;
    m.showMaximized();
    return a.exec();
}
