#-------------------------------------------------
#
# Project created by QtCreator 2025-09-11T11:15:06
#
#-------------------------------------------------

QT       += core gui network quick  charts quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogCollector
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

DISTFILES += \
    dashboard_1.qml
