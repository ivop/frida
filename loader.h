#ifndef LOADER_H
#define LOADER_H

#include <QFile>

class Loader {
public:
    virtual bool Load(QFile& file) = 0;
    static struct segment createEmptySegment(quint64 start, quint64 end);
    void genericComment(QFile& file, struct segment *segment);
};

class LoaderRaw : public Loader {
public:
    bool Load(QFile& file) override;
};

class LoaderAtari8bitBinary : public Loader {
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

#endif // LOADER_H
