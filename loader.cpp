#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <string.h>
#include <stdio.h>
#include "frida.h"
#include "loader.h"
#include "disassembler.h"

#define LE16(x) ((x)[0] | (x)[1]<<8)
#define BE16(x) ((x)[0]<<8 | (x)[1])
#define LE32(x) ((x)[0] | (x)[1]<<8 | (x)[2]<<16 | (x)[3]<<24)
#define BE32(x) ((x)[0]<<24 | (x)[1]<<16 | (x)[2]<<8 | (x)[3])

// note on zeroed memory:
// new char[size]   is similar to malloc()
// new char[size]() is similar to calloc()

void Loader::genericComment(QFile& file, struct segment *segment) {
    segment->comments.insert(0,
        QString("\n") +
        QString("Disassembled from: ") + file.fileName().section("/",-1,-1) +
        QString("\n"));
}

struct segment Loader::createEmptySegment(quint64 start, quint64 end) {
    quint64 size = end - start + 1;
     struct segment segment = {
         start, end, "",
         new quint8[size](), new quint8[size](), new quint8[size](),
         QMap<quint64, QString>(),
         QMap<quint64, QString>(),
         QMap<quint64, quint16>(),
         QMap<quint64, quint16>(),
         QList<struct disassembly>(),
         0
     };
     return segment;
}

bool LoaderRaw::Load(QFile& file) {
    quint64 size = file.size();
    struct segment segment = createEmptySegment(0, size-1);
    if ((quint64)file.read((char*)segment.data, size) != size)
        return false;

    genericComment(file, &segment);
    segments.append(segment);

    return true;
}

// ---------------------------------------------------------------------------
// ATARI 8-BIT

bool LoaderAtari8bitBinary::Load(QFile& file) {
    quint8 tmp[2];
    quint64 ffff, start, end, size;

    file.read((char *)tmp, 2);
    ffff = LE16(tmp);
    if (ffff != 0xffff) {
        this->error_message = "Start marker (0xffff) not found!\n";
        return false;
    }

    while (1) {
        if (file.read((char *)tmp, 2) != 2)
            break;
        start = LE16(tmp);
        if (start == 0xffff) {
            file.read((char *)tmp, 2);
            start = LE16(tmp);
        }
        file.read((char *)tmp, 2);
        end = LE16(tmp);
        size = end - start + 1;

        struct segment segment = createEmptySegment(start, end);
        if ((quint64)file.read((char*)segment.data, size) != size) {
            this->error_message = "Premature end of file!\n";
            return false;
        }

        if (start == 0x02e0 && end == 0x02e1)
            segment.name = "Run Address";
        if (start == 0x02e2 && end == 0x02e3)
            segment.name = "Init Address";
        if (start == 0x02e0 && end == 0x02e3)
            segment.name = "Run/Init Addresses";

        genericComment(file, &segment);
        segments.append(segment);
    }

    return true;
}

bool LoaderAtari8bitSAP::Load(QFile& file) {
    quint8 tmp[5];
    unsigned int init, play;
    quint8 c;
    QChar type;
    QString header;

    file.read((char*) tmp, 5);

    if (strncmp((char *) tmp, "SAP\x0d\x0a", 5)) {
        this->error_message = "SAP Identifier not found!\n";
        return false;
    }

    file.getChar((char *) &c);
    while (c != 0xff) {
        header += c;
        file.getChar((char *) &c);
        if (file.atEnd()) {
            this->error_message = "Premature end of file!\n";
            return false;
        }
    }

    qDebug() << header;

    QRegularExpression re;
    QRegularExpressionMatch match;

    re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    re.setPattern("INIT[ ]*([0-9A-Fa-f]{1,4})");
    match = re.match(header);
    if (match.hasMatch() == false) {
        this->error_message = "No INIT address found\n";
        return false;
    }
    init = match.captured(1).toInt(nullptr, 16);

    re.setPattern("PLAYER[ ]*([0-9A-Fa-f]{1,4})");
    match = re.match(header);
    if (match.hasMatch() == false) {
        this->error_message = "No PLAYER address found\n";
        return false;
    }
    play = match.captured(1).toInt(nullptr, 16);

    re.setPattern("TYPE[ ]*([BCDRS])");
    match = re.match(header);
    if (match.hasMatch() == false) {
        this->error_message = "No PLAYER address found\n";
        return false;
    }
    type = match.captured(1).toInt(nullptr, 16);

    file.ungetChar(c);

    LoaderAtari8bitBinary la8b;
    if (la8b.Load(file) == false)
        return false;

    userLabels.insert(init, "init");
    userLabels.insert(play, "play");

    return true;
}

// ----------------------------------------------------------------------------
// COMMODORE C64

bool LoaderC64Binary::Load(QFile& file) {
    quint8 tmp[2];
    quint64 start, end, size;

    file.read((char *) tmp, 2);
    start = LE16(tmp);
    size = file.size() - 2;
    end = start + size - 1;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    genericComment(file, &segment);

    // 2-byte pointer to the next line of BASIC code
    // 2-byte line number
    // Byte code for the SYS command is $9e
    // Zero terminated string
    // Pointer to next line or zero

    if (segment.data[4] == 0x9e) {
        segment.datatypes[0] = segment.datatypes[1] = DT_WORDLE;
        segment.datatypes[2] = segment.datatypes[3] = DT_WORDLE;
        segment.datatypes[4] = DT_BYTES;
        int i;
        for (i=5; segment.data[i]; i++)
            segment.datatypes[i] = DT_PETSCII;
        segment.datatypes[i] = DT_PETSCII;

        int sys = QString((const char *)&segment.data[5]).toInt();
        QString hex = QString("SYS $%1").arg(sys, 4, 16, QLatin1Char('0'));
        segment.comments.insert(start+4, hex);
    }

    segments.append(segment);

    return true;
}

bool LoaderC64PSID::Load(QFile &file) {
    quint8 tmp[4];
    quint64 start = 0, end, size, init, play, version;

    file.read((char*) tmp, 4);
    if ((tmp[0] != 'P' && tmp[0] != 'R') || tmp[1] != 'S' || tmp[2] != 'I' || tmp[3] != 'D') {
        this->error_message = "SID Identifier not found!\n";
        return false;
    }

    size = file.size();

    file.read((char*) tmp, 2);      // version
    version = BE16(tmp);

    file.read((char*) tmp, 2);      // data offset
    size -= BE16(tmp);

    file.read((char*) tmp, 2);      // load address
    start = BE16(tmp);
    file.read((char*) tmp, 2);      // init address
    init = BE16(tmp);
    file.read((char*) tmp, 2);      // play address
    play = BE16(tmp);

    file.read((char*) tmp, 2);      // number of songs
    file.read((char*) tmp, 2);      // start song

    file.seek(file.pos() + 4);      // skip speed
    file.seek(file.pos() + 32*3);   // skip name, author, copyright
    if (version >= 2)
        file.seek(file.pos() + 6);  // skip flags/reserved
    if (!start) {
        file.read((char*) tmp, 2);
        start = LE16(tmp);
        size -= 2;
    }

    end = start + size - 1;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    genericComment(file, &segment);
    segments.append(segment);

    userLabels.insert(init, "init");
    userLabels.insert(play, "play");

    return true;
}

// ----------------------------------------------------------------------------
// ATARI 2600

bool LoaderAtari2600ROM2K4K::Load(QFile &file) {
    quint64 size;
    quint16 start, end, init;

    size = file.size();

    if (size != 2048 && size != 4096) {
        this->error_message = "File size if not 2KB or 4KB.\n";
        return false;
    }

    start = 0xf000;
    end   = start + size - 1;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    init = segment.data[size-4] + segment.data[size-3] * 256;

    if (size == 2048 && init >= 0xf800) {
        segment.start += 0x800;
        segment.end += 0x800;
    }

    genericComment(file, &segment);
    segments.append(segment);

    userLabels.insert(init, "init");

    return true;
}

// ----------------------------------------------------------------------------
// ORIC 1 / ATMOS

bool LoaderOricTap::Load(QFile &file) {
    char c;
    quint8 data[9];
    quint64 start, end, size;

    // Synchronisation bytes: 0x16

    file.getChar(&c);
    if (c != 0x16) {
        this->error_message = "Synchronisation bytes not found!\n";
        return false;
    }

    while (c == 0x16) {
        file.getChar(&c);
    }

    // End of synchronisation: $24

    if (c != 0x24) {
        this->error_message = "Proper synchronistion end not found!\n";
        return false;
    }

    file.read((char *)data, 9);

    start = (data[6] << 8) + data[7];   // big endian!
    end   = (data[4] << 8) + data[5];   // big endian!

    size = end - start + 1;

    while (c != 0x00) {
        file.getChar(&c);   // skip zero terminated name
    }

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    genericComment(file, &segment);
    segments.append(segment);

    userLabels.insert(start, "start");

    return true;
}

// ----------------------------------------------------------------------------
// APPLE ][

bool LoaderApple2DOS33::Load(QFile &file) {
    quint8 tmp[4];
    quint16 start, end, size;

    file.read((char*) tmp, 4);
    start = LE16(tmp);
    size  = LE16(tmp+2);

    end   = start + size - 1;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    genericComment(file, &segment);
    segments.append(segment);

    userLabels.insert(start, "start");

    return true;
};

bool LoaderApple2AppleSingle::Load(QFile &file) {
    quint8 tmp[12];
    quint16 numentries, start, end, size;
    quint32 magic, version, entry_id, offset, length;
    quint32 prodos_offset, data_fork_offset;

    file.read((char*) tmp, 8);
    magic   = BE32(tmp);
    version = BE32(tmp+4);

    if (magic != 0x00051600 || version != 0x00020000) {
        this->error_message = "Magic Number not found!\n";
        return false;       // fix if anything other than 0x00020000 exists
    }

    file.seek(file.pos() + 16);     // skip filler

    file.read((char*) tmp, 2);
    numentries = BE16(tmp);

    // find ProDOS file info and data fork

    prodos_offset = data_fork_offset = 0xffffffff;
    size = 0;

    for (int i=0; i<numentries; i++) {
        file.read((char*) tmp, 12);

        entry_id = BE32(tmp);
        offset   = BE32(tmp+4);
        length   = BE32(tmp+8);

        if (entry_id == 11) {   // ProDOS File Info
            prodos_offset = offset;
        }
        if (entry_id == 1) {    // remember Data fork
            data_fork_offset = offset;
            size = length;
        }
    }

    if (size == 0) {
        this->error_message = "Size of data fork is zero!\n";
        return false;
    }
    if (data_fork_offset == 0xffffffff) {
        this->error_message = "Data fork not found!\n";
        return false;
    }
    if (prodos_offset == 0xffffffff) {
        this->error_message = "ProDOS File Info not found!\n";
        return false;
    }

    file.seek(prodos_offset);
    file.read((char*) tmp, 8);
    start = BE16(tmp+6);

    end = start + size - 1;

    file.seek(data_fork_offset);
    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    genericComment(file, &segment);
    segments.append(segment);

    userLabels.insert(start, "start");

    return true;
};

// ----------------------------------------------------------------------------
// NINTENDO NES

bool LoaderNESSongFile::Load(QFile &file) {
    quint8 tmp[8];
    quint64 start, end, size, init, play;
    int x = 0;

    file.read((char*) tmp, 5);
    if (strncmp((char*) tmp, "NESM\x1a", 5)) {
        this->error_message = "Magic Number not found!\n";
        return false;
    }

    file.seek(file.pos() + 3);      // skip version, songs, starting song
    file.read((char *) tmp, 6);

    start = LE16(tmp);
    init  = LE16(tmp+2);
    play  = LE16(tmp+4);

    file.seek(file.pos() + 3*32 + 2);   // skip name, artist, copyright, NTSC speed

    file.read((char*) tmp, 8);
    for (int i=0; i<8; i++)
        x += tmp[i];
    if (x) {
        this->error_message = "Bankswitching NSF file not supported!\n";
        return false;
    }

    file.seek(file.pos() + 8);          // skip PAL speed, flags, reserved

    size = file.size() - file.pos();

    end = start + size - 1;

    if (end > 0xffff)
        return false;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size) {
        this->error_message = "Premature end of file!\n";
        return false;
    }

    genericComment(file, &segment);
    segments.append(segment);

    userLabels.insert(init, "init");
    userLabels.insert(play, "play");

    return true;
}
