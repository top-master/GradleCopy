#include "src/test-helpers.h"

#include <QApplication>


int main(int argc, char *argv[])
{
#if defined(Q_OS_LINUX)
    // Headless CI / containers have no X or Wayland display, and no linux
    // framebuffer device, so XD's default "linuxfb" QPA aborts the process
    // before any test runs. Fall back to the "offscreen" platform, and
    // point the platform-plugin search at XD's own plugins dir: a debug
    // build's per-build-mode plugin staging skips XD's release-named
    // plugins, leaving the staged search path empty. An explicit
    // QT_QPA_PLATFORM (e.g. a developer on a real display) always wins.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")
            && qEnvironmentVariableIsEmpty("DISPLAY")
            && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
#ifdef XD_QT_PLUGINS
        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH",
                QByteArray(XD_QT_PLUGINS) + "/platforms");
#endif
    }
#endif

    // QApplication is needed because some test classes instantiate
    // QWidget subclasses (MainWindow, ListView) which require it.
    QApplication app(argc, argv);

    return QTestRunner::run(argc, argv);
}
