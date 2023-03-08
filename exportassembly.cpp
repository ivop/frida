#include <QWidget>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QMap>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>
#include <QDataStream>
#include "frida.h"
#include "exportassembly.h"

static int error;
extern QString errorstring;

static void write_line(QTextStream& out) {
    out << "; ----------------------------------------------------------";
    out << "------------------\n";
}

void export_assembly(QWidget *widget) {
    QString name = QFileDialog::getSaveFileName(widget, "Export assembly as...");

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

    QString hexPrefix = Disassembler->hexPrefix;
    QString hexSuffix = Disassembler->hexSuffix;

    QTextStream out(&file);

    write_line(out);
    out << "\n; Generated by Frida version ";
    out << FRIDA_VERSION_STRING;
    out << "\n\n";
    write_line(out);

    out << "\n; GENERATED LABELS\n\n";

    QMap<quint64, QString>::const_iterator iter;;

    // output all generated labels that are not overruled by user or local
    // labels and are not within any of the segments address ranges.
    // also omit labels that have +/- math inside them

    for(iter = autoLabels.constBegin(); iter != autoLabels.constEnd(); iter++) {

        if (iter.value().contains("+") || iter.value().contains("-"))
            continue;

        if (userLabels.contains(iter.key()))
            continue;

        bool found_local = false;
        for (int i=0; i<segments.size(); i++) {
            if (segments[i].localLabels.contains(iter.key())) {
                found_local = true;
                break;
            }
        }
        if (found_local)
            continue;

        bool inside_segment = false;
        for (int i=0; i<segments.size(); i++) {
            quint64 start = segments[i].start;
            quint64 end   = segments[i].end;
            quint64 key = iter.key();
            if ((key >= start) && (key <= end)) {
                inside_segment = true;
                break;
            }
        }
        if (inside_segment)
            continue;

        out << iter.value() << '=' << hexPrefix;
        out << QString("%1").arg(iter.key(), 0, 16) << hexSuffix << "\n";
    }

    out << "\n";
    write_line(out);

    out << "\n; USER LABELS\n\n";

    // output all user labels that are not overruled by local labels and are
    // not within any of the segments address ranges.
    // also omit labels that have +/- math inside them

    for(iter = userLabels.constBegin(); iter != userLabels.constEnd(); iter++) {

        if (iter.value().contains("+") || iter.value().contains("-"))
            continue;

        bool found_local = false;
        for (int i=0; i<segments.size(); i++) {
            if (segments[i].localLabels.contains(iter.key())) {
                found_local = true;
                break;
            }
        }
        if (found_local)
            continue;

        bool inside_segment = false;
        for (int i=0; i<segments.size(); i++) {
            quint64 start = segments[i].start;
            quint64 end   = segments[i].end;
            quint64 key = iter.key();
            if ((key >= start) && (key <= end)) {
                inside_segment = true;
                break;
            }
        }
        if (inside_segment)
            continue;

        out << iter.value() << '=' << hexPrefix;
        out << QString("%1").arg(iter.key(), 0, 16) << hexSuffix << "\n";
    }

    out << "\n";
    write_line(out);

    // output each segment, starting with its local labels, and then its
    // dissassembly

    int saveCurrentSegment = currentSegment;

    for (int i=0; i<segments.size(); i++) {
        struct segment *s = &segments[i];

        currentSegment = i;
        Disassembler->generateDisassembly();

        out << "\n; SEGMENT: " << i+1 << "\n\n";
        out << "; Name    : " << s->name << "\n";
        out << "; Start   : ";
        out << hexPrefix << QString("%1").arg(s->start, 0, 16) << hexSuffix;
        out << "\n; End     : ";
        out << hexPrefix << QString("%1").arg(s->end, 0, 16) << hexSuffix;
        out << "\n\n; LOCAL LABELS\n\n";

        // output local labels that are not inside a segment or contain +/-

        for(iter  = s->localLabels.constBegin();
            iter != s->localLabels.constEnd(); iter++) {

            if (iter.value().contains("+") || iter.value().contains("-"))
                continue;

            bool inside_segment = false;
            for (int j=0; j<segments.size(); j++) {
                quint64 start = segments[j].start;
                quint64 end   = segments[j].end;
                quint64 key = iter.key();
                if ((key >= start) && (key <= end)) {
                    inside_segment = true;
                    break;
                }
            }
            if (inside_segment)
                continue;

            out << iter.value() << '=' << hexPrefix;
            out << QString("%1").arg(iter.key(), 0, 16) << hexSuffix << "\n";
        }

        out << "\n    org ";
        out << hexPrefix << QString("%1").arg(s->start, 0, 16) << hexSuffix;
        out << "\n\n";

        // output disassembly

        QList<struct disassembly> *dislist  = &s->disassembly;
        QMap<quint64, QString> *comments     = &s->comments;
        struct disassembly dis;
        QString com;

        qint64 di = 0;
        while (di < dislist->size()) {
            dis = dislist->at(di);

            if (comments->contains(dis.address)) {
                com = comments->value(dis.address);
                out << ";\n";       // XXX print multi-line comment
            }

            if (   userLabels.contains(dis.address)
                || autoLabels.contains(dis.address)
                || s->localLabels.contains(dis.address)) {

                QString label;
                if (s->localLabels.contains(dis.address)) {
                    label = s->localLabels.value(dis.address);
                } else if (userLabels.contains(dis.address)) {
                    label = userLabels.value(dis.address);
                } else {
                    label = autoLabels.value(dis.address);
                }

                // we do not print labels with + or -

                if (!label.contains(QChar('+')) && !label.contains(QChar('-'))) {
                    out << label << "\n";
                }
            }

            out << "    " << dis.instruction << " " << dis.arguments << "\n";

            if (dis.changes_pc) {
                out << "\n";
            }

            di++;
        }

        out << "\n";
        write_line(out);
    }

    currentSegment = saveCurrentSegment;

    error = file.error();
    errorstring = file.errorString();

    file.close();

    if (error != file.NoError) {
        QMessageBox msg;
        msg.setText("Failed to export " + name + "\n\n" + errorstring);
        msg.exec();
    } else {
        QMessageBox msg;
        msg.setText("Succesfully exported " + name + "\n");
        msg.exec();
    }
}
