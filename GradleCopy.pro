TEMPLATE = app
TARGET = GradleCopy

QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release) {
    CONFIG += qt_static
    #CONFIG += static_runtime
}

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    copythread.cpp \
    listview.cpp \
    filedownloader.cpp \
    downloadthread.cpp

HEADERS += \
    mainwindow.h \
    copythread.h \
    listview.h \
    filedownloader.h \
    downloadthread.h

FORMS += \
    mainwindow.ui \
    listview.ui
