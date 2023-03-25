#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T18:16:08
#
#-------------------------------------------------

CONFIG += precompile_header
PRECOMPILED_HEADER = pch.h

QT += core gui widgets

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

QMAKE_CXXFLAGS += -std=c++17

TARGET = frida
TEMPLATE = app

SOURCES += \
    disassembler8080.cpp \
    exportassembly.cpp \
    exportassemblywindow.cpp \
    jumptowindow.cpp \
    lowandhighbytepairswindow.cpp \
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
    lowandhighbytepairswindow.h \
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
    lowandhighbytepairswindow.ui \
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
