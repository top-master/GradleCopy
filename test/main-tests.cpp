#include "src/test-helpers.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    // QApplication is needed because some test classes instantiate
    // QWidget subclasses (MainWindow, ListView) which require it.
    QApplication app(argc, argv);

    return QTestRunner::run(argc, argv);
}
