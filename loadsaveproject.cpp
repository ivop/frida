#include "loadsaveproject.h"
#include "frida.h"
#include "loader.h"
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QMap>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>
#include <QDataStream>

static const char *magic = "FRIDA";
static char checkmagic[5];
static quint8 fileformat;
static int error;
QString errorstring;

enum {
    FRIDA_FILE_FORMAT_1
};

bool load_project(QWidget *widget) {
    QString name = QFileDialog::getOpenFileName(widget, "Open existing project...");

    if (name.isEmpty()) return false;

    QFile file(name);

    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        QMessageBox msg;
        msg.setText("Failed to open " + name + "\n" + file.errorString());
        msg.exec();
        return false;
    }

    QDataStream in(&file);

    in.readRawData(checkmagic, 5);

    if (memcmp(magic, checkmagic, 5)) {
        error = QFile::OpenError;
        errorstring = "This is not a Frida Project file";
        goto error_out;
    }

    in >> fileformat;

    // future: select loader for older file formats

    in.setVersion(QDataStream::Qt_5_15);

    in >> cputype;

    quint32 numsegments;

    in >> numsegments;

    for (unsigned int i=0; i<numsegments; i++) {
        quint64 start, end;

        in >> start >> end;

        quint64 length = end - start + 1;

        struct segment s = Loader::createEmptySegment(start, end);

        in >> s.name;

        in.readRawData((char *) s.data,      length);
        in.readRawData((char *) s.datatypes, length);
        in.readRawData((char *) s.flags,     length);

        in >> s.comments;
        in >> s.localLabels;
        in >> s.lowbytes;
        in >> s.highbytes;

        segments.append(s);
    }

    in >> autoLabels;
    in >> userLabels;
    in >> globalNotes;

    error = file.error();
    errorstring = file.errorString();

error_out:          // goto here with error and errorstring set.
    file.close();

    if (error != file.NoError) {
        QMessageBox msg;
        msg.setText("Failed to load " + name + "\n\n" + errorstring);
        msg.exec();
        return false;
    } else {
        QMessageBox msg;
        msg.setText("Succesfully loaded " + name + "\n");
        msg.exec();
        return true;
    }
}

void save_project(QWidget *widget) {
    QString name = QFileDialog::getSaveFileName(widget, "Save project as...");

    if (name.isEmpty()) return;

    QFile file(name);
    if (file.exists()) {
        QMessageBox msg;
        msg.setText("File " + name + " exists. Overwrite?");
        msg.setStandardButtons(QMessageBox::Yes);
        msg.addButton(QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);
        if(msg.exec() != QMessageBox::Yes)
            return;
    }

    file.open(QIODevice::WriteOnly);
    if (!file.isOpen()) {
        QMessageBox msg;
        msg.setText("Failed to open " + name + "\n\n" + file.errorString());
        msg.exec();
        return;
    }

    QDataStream out(&file);

    out.writeRawData(magic, 5);
    out << (quint8) FRIDA_FILE_FORMAT_1;

    out.setVersion(QDataStream::Qt_5_15);

    out << cputype;

    quint32 numsegments = segments.size();

    out << numsegments;

    for (unsigned int i=0; i<numsegments; i++) {
        struct segment *s = &segments[i];

        out << s->start << s->end << s->name;

        quint64 length = s->end - s->start + 1;

        out.writeRawData((const char *) s->data,      length);
        out.writeRawData((const char *) s->datatypes, length);
        out.writeRawData((const char *) s->flags,     length);

        out << s->comments;
        out << s->localLabels;
        out << s->lowbytes;
        out << s->highbytes;
    }

    out << autoLabels;
    out << userLabels;
    out << globalNotes;

    error = file.error();
    errorstring = file.errorString();

    file.close();

    if (error != file.NoError) {
        QMessageBox msg;
        msg.setText("Failed to save " + name + "\n\n" + errorstring);
        msg.exec();
    } else {
        QMessageBox msg;
        msg.setText("Succesfully saved " + name + "\n");
        msg.exec();
    }
}