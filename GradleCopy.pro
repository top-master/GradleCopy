QT       += core gui network

CONFIG(release, debug|release) {
    CONFIG += qt_static
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GradleCopy
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    copythread.cpp \
    listview.cpp \
    filedownloader.cpp \
    downloadthread.cpp

HEADERS  += mainwindow.h \
    copythread.h \
    listview.h \
    filedownloader.h \
    downloadthread.h

FORMS    += mainwindow.ui \
    listview.ui
