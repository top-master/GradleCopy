QT       += core gui
#CONFIG += qt_static

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GradleCopy
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    copythread.cpp

HEADERS  += mainwindow.h \
    copythread.h

FORMS    += mainwindow.ui
