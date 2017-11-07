#-------------------------------------------------
#
# Project created by QtCreator 2013-07-30T14:24:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Project
TEMPLATE = app


SOURCES += main.cpp \
    codeeditor.cpp \
    operatingwindow.cpp \
    codeeditorwindow.cpp \
    treemodel.cpp \
    machine.cpp \
    parser.cpp \
    translator.cpp

HEADERS  += \
    codeeditor.h \
    operatingwindow.h \
    codeeditorwindow.h \
    treemodel.h \
    parser.h \
    machine.h \
    translator.h

FORMS    +=

OTHER_FILES +=

RESOURCES += \
    icons.qrc \
    machineTemplate.qrc
