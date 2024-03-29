#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T18:16:08
#
#-------------------------------------------------

CONFIG += precompile_header
PRECOMPILED_HEADER = pch.h

QT += core gui widgets

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

LIBS += -lz

TARGET = frida

win32* {
    RC_ICONS = ../deploy/win64/default.ico
}

macx* {
    ICON = ../deploy/macos/default.icns
}

SOURCES += \
    addconstantsgroupwindow.cpp \
    addconstanttogroupwindow.cpp \
    constantsmanager.cpp \
    disassembler8080.cpp \
    disassemblerZ80.cpp \
    exportassembly.cpp \
    exportassemblywindow.cpp \
    jumptowindow.cpp \
    loaderatari8bitcar.cpp \
    loaders.cpp \
    lowandhighbytepairswindow.cpp \
    main.cpp\
    loadsaveproject.cpp \
    mainwindow.cpp \
    filetypes.cpp \
    cputypes.cpp \
    disassembler6502.cpp \
    disassembler.cpp \
    commentwindow.cpp \
    labelswindow.cpp \
    addlabelwindow.cpp \
    changesegmentwindow.cpp \
    lowhighbytewindow.cpp \
    selectcartridgewindow.cpp \
    selectconstantsgoupwindow.cpp \
    startdialog.cpp

HEADERS += \
    addconstantsgroupwindow.h \
    addconstanttogroupwindow.h \
    architecture.h \
    compiler.h \
    constantsmanager.h \
    exportassembly.h \
    exportassemblywindow.h \
    jumptowindow.h \
    loaderatari8bitcar.h \
    loaders.h \
    lowandhighbytepairswindow.h \
    mainwindow.h \
    frida.h \
    disassembler.h \
    commentwindow.h \
    labelswindow.h \
    addlabelwindow.h \
    changesegmentwindow.h \
    loadsaveproject.h \
    lowhighbytewindow.h \
    platform.h \
    selectcartridgewindow.h \
    selectconstantsgoupwindow.h \
    startdialog.h

FORMS += \
    addconstantsgroupwindow.ui \
    addconstanttogroupwindow.ui \
    constantsmanager.ui \
    exportassemblywindow.ui \
    jumptowindow.ui \
    lowandhighbytepairswindow.ui \
    mainwindow.ui \
    commentwindow.ui \
    labelswindow.ui \
    addlabelwindow.ui \
    changesegmentwindow.ui \
    lowhighbytewindow.ui \
    selectcartridgewindow.ui \
    selectconstantsgoupwindow.ui \
    startdialog.ui

RESOURCES += \
    frida.qrc

