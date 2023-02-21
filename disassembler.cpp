#include <QDebug>
#include <QMessageBox>
#include <lowhighbytewindow.h>
#include "frida.h"
#include "disassembler.h"

// ---------------------------------------------------------------------------

static inline bool xisprint_ascii(quint8 v) {
    return v >= 0x20 && v <= 0x7e;
}

// ATASCII
// -------
// 0x20-0x5f identical
// 0x60      diamond
// 0x61-0x7a identical
// 0x7b      {
// 0x7c      identical
// 0x7d-0x7f } ~ DEL

static inline quint8 atascii_to_ascii(quint8 v) {
    if ( (v>=0x20 && v<=0x5f) || (v>=0x61 && v<=0x7a) || v==0x7c)
        return v;
    return 255;         // unprintable ascii, results in .byte
}

// PETSCII
// -------
// 0x20-0x40 identical
// 0x41-0x5a lower case, convert +=0x20
// 0x5b      identical
// 0x5c      pound sign
// 0x5d      identical
// 0x5e-0x60 arrow up, left, none
// 0x61-0x7a upper case, convert -=0x20
// 0x7b-0x7c gfx
// 0x7d      |, convert -=0x01
// 0x7e-0x7f gfx

static inline quint8 petscii_to_ascii(quint8 v) {
    if ( (v>=0x20 && v<=0x40) || v==0x5b || v==0x5d)
        return v;
    if (v>=0x41 && v<=0x5a)
        return v + 0x20;
    if (v>=0x61 && v<=0x7a)
        return v - 0x20;
    if (v==0x7d)
        return v-1;
    return 255;         // unprintable ascii, results in .byte
}

// ANTIC Screen Codes
// ------------------
// ANTIC Screen Code 0x00-0x3f  --> ATASCII 0x20-0x5f   (+0x20)
// ANTIC Screen Code 0x40-0x5f  --> ATASCII 0x00-0x1f   (-0x40)
// ANTIC Screen Code 0x60-0x7f  --> ATASCII 0x60-0x7f

static inline quint8 antic_screen_to_ascii(quint8 v) {
    if (           v<=0x3f)
        return atascii_to_ascii(v+0x20);
    if (v>=0x40 && v<=0x5f)
        return atascii_to_ascii(v-0x40);
    return atascii_to_ascii(v);
}

// CBM Screen Codes
// ----------------
// CBM Screen Code 0x80-0x9f    --> PETSCII 0x00-0x1f   (-0x80)
// CBM Screen Code 0x20-0x3f    --> PETSCII 0x20-0x3f
// CBM Screen Code 0x00-0x1f    --> PETSCII 0x40-0x5f   (+0x40)
// CBM Screen Code 0x40-0x5f    --> PETSCII 0x60-0x7f   (+0x20)

static inline quint8 cbm_screen_to_ascii(quint8 v) {
    if (v>=0x80 && v<=0x9f)
        return petscii_to_ascii(v-0x80);
    if (v>=0x20 && v<=0x3f)
        return petscii_to_ascii(v);
    if (           v<=0x1f)
        return petscii_to_ascii(v+0x40);
    if (v>=0x40 && v<=0x5f)
        return petscii_to_ascii(v+0x20);
    return 255;
}

// ---------------------------------------------------------------------------

// Side effects:
//   it checks the consistency of the datatypes specified and might fix
//   them if they don't compute (ascii which is non-printable, etc...)

void Disassembler::generateDisassembly(void) {
    struct segment *s = &segments[currentSegment];
    quint64 start = s->start, end = s->end, size = end - start + 1;
    quint64 val = 0, val2 = 0;
    quint8 *data  = s->data;
    quint8 *datatypes = s->datatypes;
    QList<struct disassembly> *dislist  = &s->disassembly;
    struct disassembly dis;
    QString hex, instr;
    int n, perline, prevtype;
    bool labelled;

    initTables();
    dislist->clear();

    // Check consistency of datatypes

    for (quint64 i = 0; i < size; i++) {
        enum datatypes type = (enum datatypes)datatypes[i];
        n = 0;

        switch(type) {
        case DT_XWORDBE:
        case DT_XWORDLE:
            if (!n) n = 16;
            [[fallthrough]];
        case DT_QWORDBE:
        case DT_QWORDLE:
            if (!n) n = 8;
            [[fallthrough]];
        case DT_DWORDBE:
        case DT_DWORDLE:
            if (!n) n = 4;
            [[fallthrough]];
        case DT_WORDBE:
        case DT_WORDLE:
            if (!n) n = 2;

            if (i+n-1 >= size) {
also_wrong:
                hex = QString("%1").arg(start+i, 4, 16, (QChar)'0');
                QMessageBox msg;
                msg.setText(hex + ": type needs " + QString("%1").arg(n) +
                                                                " bytes");
                msg.exec();
                return;
            }
            // check that all bytes are of the same type
            // plus check if one of the bytes is flagged labelled
            labelled = s->flags[i] & FLAG_USE_LABEL;
            for (int j=1; j<n; j++) {
                if (datatypes[i+j] != type) goto also_wrong;
                if (s->flags[i+j] & FLAG_USE_LABEL) labelled = true;
            }
            // if one byte is flagged, flag all of them
            if (labelled) for (int j=0; j<n; j++) {
                s->flags[i+j] |= FLAG_USE_LABEL;
            }
            i += n-1;
            break;

        case DT_CODE:
            n = getInstructionSizeAt(i);

            if (i+n-1 >= size) {
also_wrong2:
                hex = QString("%1").arg(start+i, 4, 16, (QChar)'0');
                QMessageBox msg;
                msg.setText(hex + ": full instruction needs " +
                            QString("%1").arg(n) + " bytes");
                msg.exec();
                return;
            }
            for (int j=1; j<n; j++) {
                if (datatypes[i+j] != type) goto also_wrong2;
            }


            // while we're at it, generate labels in the same loop?
            createOperandLabel(i);
            i += n-1;
            break;

        case DT_UNDEFINED_BYTES:
        case DT_BYTES:
        case DT_UNDEFINED_CODE:
        case DT_LAST:
            break;                  // allways "good"

        case DT_ASCII:
            if (!xisprint_ascii(data[i])) {
                datatypes[i] = DT_BYTES;
            }
            break;
        case DT_ATASCII:
            if (!xisprint_ascii(atascii_to_ascii(data[i]))) {
                datatypes[i] = DT_BYTES;
            }
            break;
        case DT_PETSCII:
            if (!xisprint_ascii(petscii_to_ascii(data[i]))) {
                datatypes[i] = DT_BYTES;
            }
            break;
        case DT_ANTIC_SCREEN:
            if (!xisprint_ascii(antic_screen_to_ascii(data[i]))) {
                datatypes[i] = DT_BYTES;
            }
            break;
        case DT_CBM_SCREEN:
            if (!xisprint_ascii(cbm_screen_to_ascii(data[i]))) {
                datatypes[i] = DT_BYTES;
            }
            break;
        }
    }

    // Generate disassembly

    perline = 0;
    prevtype = -1;
    for (quint64 i = 0; i < size; i++) {
        enum datatypes type = (enum datatypes)datatypes[i];
        n = 0;
        switch(type) {

        case DT_XWORDBE:
            val2 = (quint64) data[i+15] <<  0 | (quint64) data[i+14]<< 8 |
                   (quint64) data[i+13] << 16 | (quint64) data[i+12]<<24 |
                   (quint64) data[i+11] << 32 | (quint64) data[i+10]<<40 |
                   (quint64) data[i+ 9] << 48 | (quint64) data[i+ 8]<<56;
            val  = (quint64) data[i+ 7] <<  0 | (quint64) data[i+ 6]<< 8 |
                   (quint64) data[i+ 5] << 16 | (quint64) data[i+ 4]<<24 |
                   (quint64) data[i+ 3] << 32 | (quint64) data[i+ 2]<<40 |
                   (quint64) data[i+ 1] << 48 | (quint64) data[i+ 0]<<56;
            instr = ".xwordbe";
            n = 16;
            goto do_directive;

        case DT_XWORDLE:
            val2 = (quint64) data[i+ 0] <<  0 | (quint64) data[i+ 1]<< 8 |
                   (quint64) data[i+ 2] << 16 | (quint64) data[i+ 3]<<24 |
                   (quint64) data[i+ 4] << 32 | (quint64) data[i+ 5]<<40 |
                   (quint64) data[i+ 6] << 48 | (quint64) data[i+ 7]<<56;
            val  = (quint64) data[i+ 8] <<  0 | (quint64) data[i+ 9]<< 8 |
                   (quint64) data[i+10] << 16 | (quint64) data[i+11]<<24 |
                   (quint64) data[i+12] << 32 | (quint64) data[i+13]<<40 |
                   (quint64) data[i+14] << 48 | (quint64) data[i+15]<<56;
            instr = ".xwordle";
            n = 16;
            goto do_directive;

        case DT_QWORDBE:
            val = (quint64) data[i+7] <<  0 | (quint64) data[i+6]<< 8 |
                  (quint64) data[i+5] << 16 | (quint64) data[i+4]<<24 |
                  (quint64) data[i+3] << 32 | (quint64) data[i+2]<<40 |
                  (quint64) data[i+1] << 48 | (quint64) data[i+0]<<56;
            instr = ".qwordbe";
            n = 8;
            goto do_directive;

        case DT_QWORDLE:
            val = (quint64) data[i+0] <<  0 | (quint64) data[i+1]<< 8 |
                  (quint64) data[i+2] << 16 | (quint64) data[i+3]<<24 |
                  (quint64) data[i+4] << 32 | (quint64) data[i+5]<<40 |
                  (quint64) data[i+6] << 48 | (quint64) data[i+7]<<56;
            instr = ".qwordle";
            n = 8;
            goto do_directive;

        case DT_DWORDBE:
            val = (quint32) data[i+3] <<  0 | (quint32) data[i+2]<<8 |
                  (quint32) data[i+1] << 16 | (quint32) data[i+0]<<24;
            instr = ".dwordbe";
            n = 4;
            goto do_directive;

        case DT_DWORDLE:
            val = (quint32) data[i+0] <<  0 | (quint32) data[i+1]<<8 |
                  (quint32) data[i+2] << 16 | (quint32) data[i+3]<<24;
            instr = ".dwordle";
            n = 4;
            goto do_directive;

        case DT_WORDBE:
            val = (quint16) data[i+1] | (quint16) data[i]<<8;
            instr = ".wordbe";
            n = 2;
            goto do_directive;

        case DT_WORDLE:
            val = (quint16) data[i] | (quint16) data[i+1]<<8;
            instr = ".wordle";
            n = 2;
            goto do_directive;

        case DT_BYTES:
        case DT_UNDEFINED_BYTES:
            val = data[i];
            instr = ".byte";
            n = 1;

do_directive:
            if (   autoLabels.contains(start+i)
                || userLabels.contains(start+i)
                || s->localLabels.contains(start+i)) {
                perline = 0; // always start new directive at label locations
            }
            hex.clear();
            if (s->flags[i] & FLAG_USE_LABEL) {
                hex = s->localLabels.value(val,"");
                if (hex.isEmpty())
                    hex = userLabels.value(val,"");
                if (hex.isEmpty())
                    hex = autoLabels.value(val,"");
            }
            if (hex.isEmpty()) {
                hex  = hexPrefix;
                if (n>8) {
                    hex += QString("%1").arg(val,  16, 16, (QChar)'0');
                    hex += QString("%1").arg(val2, 16, 16, (QChar)'0');
                } else {
                    hex += QString("%1").arg(val, n*2, 16, (QChar)'0');
                }
            }
            hex += hexSuffix;
            if (perline <= 0 || prevtype != type) {
                dis = { start + i, instr, hex, n, false, 256, 0, 0 };
                perline = 8;
                prevtype = type;
                dislist->append(dis);
            } else {
                dislist->last().arguments += ", " + hex;
            }
            perline -= n;
            i += n-1;
            break;

        case DT_ATASCII:
            val = atascii_to_ascii(data[i]);
            instr = ".atascii";
            goto do_string_directive;

        case DT_PETSCII:
            val = petscii_to_ascii(data[i]);
            instr = ".petscii";
            goto do_string_directive;

        case DT_ANTIC_SCREEN:
            val = antic_screen_to_ascii(data[i]);
            instr = ".anticscreen";
            goto do_string_directive;

        case DT_CBM_SCREEN:
            val = cbm_screen_to_ascii(data[i]);
            instr = ".cbmscreen";
            goto do_string_directive;

        case DT_ASCII:
            val = data[i];
            instr = ".ascii";

do_string_directive:
            n = 1;

            // XXX do not start new directive when label contains + or -
            if (   autoLabels.contains(start+i)
                || userLabels.contains(start+i)
                || segments[currentSegment].localLabels.contains(start+i)
                || perline <= 0
                || prevtype != type) {
                // start new directive at label location
                dis = { start + i, instr, QString("\"\""), n, false, 256, 0, 0 };
                perline = 40;
                prevtype = type;
                dislist->append(dis);
            }
            dislist->last().arguments.remove(dislist->last().arguments.size()-1,1);
            dislist->last().arguments += QChar((quint8)val);
            dislist->last().arguments += QChar('"');
            perline -= n;

            break;

        case DT_UNDEFINED_CODE:
        case DT_CODE:
            perline = 0;
            disassembleInstructionAt(i, dis, n);
            dislist->append(dis);
            i += n-1;
            break;

        default:
            qDebug() << "should not be here ;)\n";
            break;
        } // switch
    } // for
};
