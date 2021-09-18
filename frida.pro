#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T18:16:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11

TARGET = frida
TEMPLATE = app

SOURCES += main.cpp\
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

HEADERS  += mainwindow.h \
    frida.h \
    loader.h \
    disassembler.h \
    commentwindow.h \
    labelswindow.h \
    addlabelwindow.h \
    changesegmentwindow.h \
    lowhighbytewindow.h \
    startdialog.h

FORMS    += mainwindow.ui \
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
