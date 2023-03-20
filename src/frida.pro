#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T18:16:08
#
#-------------------------------------------------

QT += core gui widgets

QMAKE_CXXFLAGS += -std=c++17

TARGET = frida
TEMPLATE = app

SOURCES += \
    disassembler8080.cpp \
    exportassembly.cpp \
    exportassemblywindow.cpp \
    jumptowindow.cpp \
    main.cpp\
    loadsaveproject.cpp \
    mainwindow.cpp \
    loader.cpp \
    filetypes.cpp \
    cputypes.cpp \
    disassembler6502.cpp \
    disassembler.cpp \
    commentwindow.cpp \
    labelswindow.cpp \
    addlabelwindow.cpp \
    changesegmentwindow.cpp \
    lowhighbytewindow.cpp \
    startdialog.cpp

HEADERS += \
    exportassembly.h \
    exportassemblywindow.h \
    jumptowindow.h \
    mainwindow.h \
    frida.h \
    loader.h \
    disassembler.h \
    commentwindow.h \
    labelswindow.h \
    addlabelwindow.h \
    changesegmentwindow.h \
    loadsaveproject.h \
    lowhighbytewindow.h \
    startdialog.h

FORMS += \
    exportassemblywindow.ui \
    jumptowindow.ui \
    mainwindow.ui \
    commentwindow.ui \
    labelswindow.ui \
    addlabelwindow.ui \
    changesegmentwindow.ui \
    lowhighbytewindow.ui \
    startdialog.ui

RESOURCES += \
    frida.qrc

OTHER_FILES += \
    readme.txt \
    labels/atari-hardware.labels
