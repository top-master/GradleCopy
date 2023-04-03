TEMPLATE = app
TARGET = GradleCopy
DESTDIR = bin

QT += core gui network xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release) {
    CONFIG += qt_static
    #CONFIG += static_runtime
    CONFIG += force_debug_info
}

win32 {
    RC_FILE = $$PWD/assets/main.rc
    DESTDIR = GradleCopy-win-x86
}

isXD {
    copyOpenSSL()
    CONFIG(debug, debug|release): !qt_static: copyModules()
}

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    copythread.cpp \
    listview.cpp \
    filedownloader.cpp \
    downloadthread.cpp \
    project-info.cpp

HEADERS += \
    mainwindow.h \
    copythread.h \
    listview.h \
    filedownloader.h \
    downloadthread.h \
    project-info.h

FORMS += \
    mainwindow.ui \
    listview.ui

RESOURCES += \
    assets/assets.qrc
