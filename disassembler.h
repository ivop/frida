#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <QDebug>

class Disassembler {
public:
    void generateDisassembly(void);
    virtual void trace(quint64 address) = 0;

protected:
    virtual void initTables(void) = 0;
    virtual int getInstructionSizeAt(quint64 address) = 0;
    virtual void createOperandLabel(quint64 address) = 0;
    virtual void disassembleInstructionAt(quint64 address,
                                          struct disassembly &dis, int&n) = 0;

    QString hexPrefix, hexSuffix;
};

class Disassembler6502 : public Disassembler {
public:
    void trace(quint64 address) override;

protected:
    void initTables(void) override;
    int getInstructionSizeAt(quint64 address) override;
    void createOperandLabel(quint64 address) override;
    void disassembleInstructionAt(quint64 address,
                                  struct disassembly &dis, int &n) override;
};

extern Disassembler *Disassembler;

#endif // DISASSEMBLER_H
