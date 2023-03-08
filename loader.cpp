#include <QDebug>
#include <QFileInfo>
#include "frida.h"
#include "loader.h"
#include "disassembler.h"

#define LE16(x) (x[0] | x[1]<<8)
#define BE16(x) (x[0]<<8 | x[1])

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

bool LoaderAtari8bitBinary::Load(QFile& file) {
    quint8 tmp[2];
    quint64 ffff, start, end, size;

    file.read((char *)tmp, 2);
    ffff = LE16(tmp);
    if (ffff != 0xffff) {
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
        if ((quint64)file.read((char*)segment.data, size) != size)
            return false;

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

bool LoaderC64Binary::Load(QFile& file) {
    quint8 tmp[2];
    quint64 start, end, size;

    file.read((char *) tmp, 2);
    start = LE16(tmp);
    size = file.size() - 2;
    end = start + size - 1;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size)
        return false;

    genericComment(file, &segment);
    segments.append(segment);

    return true;
}

bool LoaderC64PSID::Load(QFile &file) {
    quint8 tmp[4];
    quint64 start = 0, end, size, init, play, version;

    file.read((char*) tmp, 4);
    if (tmp[0] != 'P' || tmp[1] != 'S' || tmp[2] != 'I' || tmp[3] != 'D')
        return false;

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
    if (version == 2)
        file.seek(file.pos() + 6);  // skip flags/reserved
    if (!start) {
        file.read((char*) tmp, 2);
        start = LE16(tmp);
        size -= 2;
    }

    end = start + size - 1;

    struct segment segment = createEmptySegment(start, end);
    if ((quint64)file.read((char*)segment.data, size) != size)
        return false;

    genericComment(file, &segment);
    segments.append(segment);

//    QString *curcom = &segments[currentSegment].comments[segment.start];
//    curcom->append(QString("\nInit: %1").arg(init, 4, 16, (QChar)'0'));
//    curcom->append(QString("\nPlay: %1").arg(play, 4, 16, (QChar)'0'));

    userLabels.insert(init, "init");
    userLabels.insert(play, "play");

    return true;
}
