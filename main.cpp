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
#include "frida.h"

QList<struct segment> segments;
int currentSegment;

enum fonts altfont;

// use QMap because they are always sorted according to the documentation

QMap<quint64, QString> autoLabels, userLabels, userConstants;

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

    MainWindow m;
    m.showMaximized();
    return a.exec();
}
