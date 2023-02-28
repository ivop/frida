#ifndef LOADER_H
#define LOADER_H

#include <QFile>

class Loader {
public:
    virtual bool Load(QFile& file) = 0;
    static struct segment createEmptySegment(quint64 start, quint64 end);
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

#endif // LOADER_H
