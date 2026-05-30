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

isXD {
    # On macOS, XD's bare-name dylibs must sit next to the binary inside the .app bundle.
    mac:   tmp_dest = $$shadowed($$DESTDIR)/$${TARGET}.app/Contents/MacOS
    else:  tmp_dest = $$shadowed($$DESTDIR)
    copyOpenSSL($$tmp_dest)
    CONFIG(debug, debug|release): !qt_static: copyModuleList($$tmp_dest)

    # Stage Qt's per-OS platform plugin under `<binary-dir>/platforms/` (skipped for static builds).
    !qt_static {
        mac {
            copyPlatformDriver(cocoa, $$tmp_dest/platforms)
            CONFIG(debug, debug|release): copyModule(PrintSupport, $$tmp_dest)  # cocoa's runtime dep
        }
        win32:  copyPlatformDriver(windows, $$tmp_dest/platforms)
        unix:!mac: copyPlatformDriver(xcb,  $$tmp_dest/platforms)
    }
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
