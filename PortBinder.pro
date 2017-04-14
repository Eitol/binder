#-------------------------------------------------
#
# Project created by QtCreator 2017-04-11T18:55:45
#
#-------------------------------------------------

QT       += core gui
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PortBinder
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    udevrule.cpp \
    device.cpp

HEADERS  += mainwindow.h \
    udevrule.h \
    device.h

FORMS    += mainwindow.ui

RESOURCES += \
    media.qrc
