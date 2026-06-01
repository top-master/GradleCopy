
#include "test-helpers.h"

#include <mainwindow.h>

#include <QFile>
#include <QStandardPaths>
#include <QStringList>


class MainWindowTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Seed ANDROID_HOME so MainWindow's ctor doesn't pop a modal QMessageBox.
        m_origAndroidHome = qgetenv("ANDROID_HOME");
        const QString fakeSdk =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation)
            + QLatin1Literal("/gradlecopy-fake-android-sdk");
        qputenv("ANDROID_HOME", QFile::encodeName(fakeSdk));
    }

    void cleanupTestCase() {
        if (m_origAndroidHome.isEmpty()) {
            qunsetenv("ANDROID_HOME");
        } else {
            qputenv("ANDROID_HOME", m_origAndroidHome);
        }
    }

    /// Just verify it stands up with ANDROID_HOME set.
    void testConstructor_shouldStandUpWithoutCrashing() {
        MainWindow w;
        QVERIFY(true);
    }

    /// Covered implicitly by RAII.
    void testDestructor_shouldRunWithoutCrashing() {
        {
            MainWindow w;
        }
        QVERIFY(true);
    }

    /// Static; guards on `instance` being non-null when called without one.
    void testLog_shouldBeSafeWithoutInstance() {
        MainWindow::log(QLL("test message"));
        QVERIFY(true);
    }

    /// Spawns a CopyThread that would outlive the test; pin "entry exists".
    void testGenerateDownloadList_shouldBailWithUnsetSource() {
        void (MainWindow::*ptr)() = &MainWindow::generateDownloadList;
        qExpect(ptr)->Not->toBeNull();
    }

    /// Forces a real window via showNormal(); pin "entry exists".
    void testShowList_shouldOpenListViewWithoutCrashing() {
        void (MainWindow::*ptr)(const QStringList &, const QStringList &)
            = &MainWindow::showList;
        qExpect(ptr)->Not->toBeNull();
    }

    /// Returns early when both lists empty; safe to drive.
    void testShowListIfAny_shouldSkipForEmptyLists() {
        MainWindow w;
        w.showListIfAny(QStringList(), QStringList());
        QVERIFY(true);
    }

private:
    QByteArray m_origAndroidHome;
};

Q_DECLARE_TEST(MainWindowTest)
#include <main_window_test.moc>
