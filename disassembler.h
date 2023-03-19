#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <QDebug>

extern class Disassembler *Disassembler;

class Disassembler {
public:
    void generateDisassembly(void);
    virtual void trace(quint64 address) = 0;

    QString hexPrefix, hexSuffix;
    quint64 cputype;
    bool toUpper;

protected:
    virtual void initTables(void) = 0;
    virtual int getInstructionSizeAt(quint64 address) = 0;
    virtual void createOperandLabels(quint64 address) = 0;
    virtual void disassembleInstructionAt(quint64 address,
                                          struct disassembly &dis, int&n) = 0;
};

class Disassembler6502 : public Disassembler {
public:
    void trace(quint64 address) override;

protected:
    void initTables(void) override;
    int getInstructionSizeAt(quint64 address) override;
    void createOperandLabels(quint64 address) override;
    void disassembleInstructionAt(quint64 address,
                                  struct disassembly &dis, int &n) override;
};

class Disassembler8080 : public Disassembler {
public:
    void trace(quint64 address) override;

protected:
    void initTables(void) override;
    int getInstructionSizeAt(quint64 address) override;
    void createOperandLabels(quint64 address) override;
    void disassembleInstructionAt(quint64 address,
                                  struct disassembly &dis, int &n) override;
};

#endif // DISASSEMBLER_H
