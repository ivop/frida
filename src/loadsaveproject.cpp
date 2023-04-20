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

#include "loaders.h"
#include "loadsaveproject.h"

static const char *magic = "FRIDA";
static char checkmagic[5];
static quint8 fileformat;
static int error;
QString errorstring;

enum {
    FRIDA_FILE_FORMAT_1 = 0,
};

//-----------------------------------------------------------------------------
// LOAD PROJECT

bool load_project(QWidget *widget) {
    QString name = QFileDialog::getOpenFileName(widget, QStringLiteral("Loca Existing Project..."));

    if (name.isEmpty()) return false;

    QMessageBox msg;

    QFile file(name);

    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        msg.setText("Failed to open " + name + "\n" + file.errorString());
        msg.exec();
        return false;
    }

    QDataStream in(&file);

    in.readRawData(checkmagic, 5);

    if (memcmp(magic, checkmagic, 5) != 0) {
        error = QFile::OpenError;
        errorstring = QStringLiteral("This is not a Frida Project file");
        goto error_out;
    }

    in >> fileformat;

    if (fileformat > FRIDA_FILE_FORMAT_1) {
        msg.setText("Unable to load " + file.errorString() +
                    "\nProject is from a newer version of Frida\n");
        msg.exec();
        file.close();
        return false;
    }

    // future: select loader for older file formats

    in.setVersion(QDataStream::Qt_5_15);

    in >> cputype;

    qint32 numsegments;

    in >> numsegments;

    for (int i=0; i<numsegments; i++) {
        quint64 start;
        quint64 end;

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

    in >> globalLabels;
    in >> globalNotes;
    in >> altfont;

    quint64 numGroups;

    in >> numGroups;

    for (quint64 i = 0; i<numGroups; i++) {
        auto *group = new struct constantsGroup;
        group->map = new QMap<quint64, QString>;

        in >> group->name;
        in >> *group->map;

        constantsGroups.insert(nextNewGroup, *group);
        nextNewGroup++;
    }

    error = file.error();
    errorstring = file.errorString();

error_out:          // goto here with error and errorstring set.
    file.close();

    if (error != file.NoError) {
        msg.setText("Failed to load " + name + "\n\n" + errorstring);
        msg.exec();
        return false;
    }

    msg.setText("Succesfully loaded " + name + "\n");
    msg.exec();
    return true;
}

//-----------------------------------------------------------------------------
// SAVE PROJECT

void save_project(QWidget *widget) {
    QString name = QFileDialog::getSaveFileName(widget, QStringLiteral("Save project as..."));

    if (name.isEmpty()) return;

    QMessageBox msg;

    QFile file(name);

    file.open(QIODevice::WriteOnly);
    if (!file.isOpen()) {
        msg.setText("Failed to open " + name + "\n\n" + file.errorString());
        msg.exec();
        return;
    }

    QDataStream out(&file);

    out.writeRawData(magic, 5);
    out << (quint8) FRIDA_FILE_FORMAT_1;

    out.setVersion(QDataStream::Qt_5_15);

    out << cputype;

    qint32 numsegments = segments.size();

    out << numsegments;

    for (int i=0; i<numsegments; i++) {
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

    out << globalLabels;
    out << globalNotes;
    out << altfont;

    quint64 numGroups = constantsGroups.size();

    out << numGroups;

    QMap<quint64, struct constantsGroup>::iterator iter;

    for (iter = constantsGroups.begin(); iter != constantsGroups.end(); ++iter) {
        auto const &group = iter.value();
        out << group.name;
        out << *group.map;
    }

    error = file.error();
    errorstring = file.errorString();

    file.close();

    if (error != file.NoError) {
        msg.setText("Failed to save " + name + "\n\n" + errorstring);
        msg.exec();
    } else {
        msg.setText("Succesfully saved " + name + "\n");
        msg.exec();
    }
}
