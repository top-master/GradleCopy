
# First, ensures search-pattern result contains newly added source-files
# (because else IDE may not re-run qmake-script).
refreshChangeDate($$PWD/test.pro)

# Ensures right name is used
# (because some QMake parsers exit once name is found).
include($$PWD/config/test-mode.pri)

# Then import testee (which is only possible because,
# we exclude repeat-sensitive scripts from what needs to be tested).
VPATH *= $$clean_path($$PWD/../)
INCLUDEPATH *= $$clean_path($$PWD/../)
include($$PWD/../GradleCopy.pro)
SOURCES -= $$clean_path($$PWD/../main.cpp)

# Restores to test-mode (because maybe was overridden above).
include($$PWD/config/test-mode.pri)

#CONFIG += console

# Skip the macOS `.app` wrapper for the test runner and install it as a
# plain binary inside the main `GradleCopy.app` bundle, alongside the app
# binary. That keeps Qt modules, plugins, OpenSSL, and any other resources
# the main app already populated (Contents/MacOS, Contents/Frameworks,
# Contents/PlugIns, Contents/Resources) shared between the two binaries
# instead of duplicating an entire second `.app` directory.
mac: CONFIG -= app_bundle
mac {
    DESTDIR = ../bin/GradleCopy.app/Contents/MacOS
} else {
    DESTDIR = ../bin
}
DESTDIR = $$clean_path( $$shadowed( $$DESTDIR ) )

SOURCES += \
    main-tests.cpp \
    $$files($$PWD/src/*.cpp, true)
