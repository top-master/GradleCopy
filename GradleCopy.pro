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
CONFIG -= debug_and_release

win32 {
    RC_FILE = $$PWD/assets/main.rc
    DESTDIR = GradleCopy-win-x86
}

# macOS Finder icon -- regenerated from `assets/main.ico` by `gen-icns.sh`.
mac {
    ICON = $$PWD/assets/main.icns
    icns_gen.target   = $$PWD/assets/main.icns
    icns_gen.depends  = $$PWD/assets/main.ico $$PWD/assets/gen-icns.sh
    icns_gen.commands = sh $$PWD/assets/gen-icns.sh $$PWD/assets/main.ico $$PWD/assets/main.icns
    QMAKE_EXTRA_TARGETS += icns_gen
    PRE_TARGETDEPS      += $$PWD/assets/main.icns
}

# Absolute path to the dir the built executable lives in.
mac:   exeDir = $$shadowed($$DESTDIR)/$${TARGET}.app/Contents/MacOS
else:  exeDir = $$shadowed($$DESTDIR)

# MARK: Copy Redist.

# OpenSSL is dynamically linked even under qt_static.
isXD: copyOpenSSL($$exeDir)

isXD : !qt_static {
    CONFIG(debug, debug|release): copyModuleList($$exeDir)
    mac {
        copyPlatformDriver(cocoa, $$exeDir/platforms)
        CONFIG(debug, debug|release): copyModule(PrintSupport, $$exeDir)
    }
    win32:  copyPlatformDriver(windows, $$exeDir/platforms)
    unix:!mac: copyPlatformDriver(xcb,  $$exeDir/platforms)
}

SOURCES += \
    $$PWD/main.cpp \
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
