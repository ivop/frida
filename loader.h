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

#ifndef LOADER_H
#define LOADER_H

#include <QFile>

class Loader {
public:
    virtual bool Load(QFile& file) = 0;
    static struct segment createEmptySegment(quint64 start, quint64 end);
    void genericComment(QFile& file, struct segment *segment);
    QString error_message;
};

class LoaderRaw : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderAtari8bitBinary : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderAtari8bitSAP : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderC64Binary : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderC64PSID : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderAtari2600ROM2K4K : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderOricTap : public Loader {
public:
    bool Load(QFile &file) override;
};

class LoaderApple2DOS33 : public Loader {
public:
    bool Load(QFile &file) override;
};

class LoaderApple2AppleSingle : public Loader {
public:
    bool Load(QFile &file) override;
};

class LoaderNESSongFile : public Loader {
public:
    bool Load(QFile &file) override;
};

class LoaderCPMBinary : public Loader {
public:
    bool Load(QFile &file) override;
};

#endif // LOADER_H
