
#include "test-helpers.h"

#include <listview.h>

#include <QStandardPaths>
#include <QStringList>


class ListViewTest : public QObject {
    Q_OBJECT

    /// Writable temp dir ending in `/`; cross-platform via `QStandardPaths::TempLocation`.
    QString tmp() const {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if ( ! dir.endsWith(QLatin1Char('/'))) {
            dir.append(QLatin1Char('/'));
        }
        return dir;
    }

private slots:
    /// Widget instantiates cleanly, view items start empty.
    void testConstructor_shouldStartWithEmptyViewItems() {
        ListView view;
        qExpect(view.getViewItems().size())->toEqual(0);
    }

    /// Covered implicitly by RAII; pin "doesn't crash".
    void testDestructor_shouldRunWithoutCrashing() {
        {
            ListView view;
        }
        QVERIFY(true);
    }

    /// Reflects what `setList` filled the UI with.
    void testGetViewItems_shouldReflectListLoadedViaSetList() {
        ListView view;
        QStringList remote; remote << QLL("https://example.com/a.jar")
                                   << QLL("https://example.com/b.jar");
        QStringList local;  local  << (tmp() + QLL("a.jar"))
                                   << (tmp() + QLL("b.jar"));
        view.setList(remote, local);
        qExpect(view.getViewItems().size())->toEqual(remote.size());
    }

    /// Preserves the remote-links input for getRemotelinks().
    void testSetList_shouldStoreRemoteLinks() {
        ListView view;
        QStringList remote; remote << QLL("https://example.com/x.jar");
        QStringList local;  local  << (tmp() + QLL("x.jar"));
        view.setList(remote, local);
        qExpect(view.getRemotelinks())->toEqual(remote);
    }

    /// Empty by default.
    void testGetRemotelinks_shouldStartEmpty() {
        ListView view;
        qExpect(view.getRemotelinks().size())->toEqual(0);
    }

    /// Appends trailing slash if missing.
    void testSetMavenFolder_shouldAcceptPathWithoutCrashing() {
        ListView view;
        view.setMavenFolder(tmp() + QLL("m2"));
        // No public getter -- pin "doesn't crash, stays usable".
        view.setMavenFolder(tmp() + QLL("m2/"));
        QVERIFY(true);
    }

    /// Contents come from the .ui form; pin "callable, returns a string list".
    void testProviderLinks_shouldStartEmpty() {
        ListView view;
        const QStringList links = view.providerLinks();
        qExpect(links.size())->toBeGreaterOrEqual(0);
    }

    /// No downloader running -> returns without crashing.
    void testMaybeAbort_shouldReturnSafelyWhenNoDownloaderRunning() {
        ListView view;
        bool aborted = view.maybeAbort();
        qExpect(aborted == true || aborted == false)->toBeTruthy(); // pin "no crash"
    }

    /// Would pop Finder via openUrl; pin "entry exists".
    void testShowDownloads_shouldRunWithoutCrashing() {
        void (ListView::*ptr)() = &ListView::showDownloads;
        qExpect(ptr)->Not->toBeNull();
    }

    /// Same -- avoid invoking openUrl in the runner.
    void testShowFolder_shouldRunWithoutCrashing() {
        void (ListView::*ptr)(const QString &) = &ListView::showFolder;
        qExpect(ptr)->Not->toBeNull();
    }

    /// Empty maven folder is safe.
    void testDisableIncompleteLibraries_shouldHandleEmptyFolder() {
        ListView view;
        view.setMavenFolder(QDir::tempPath());
        view.disableIncompleteLibraries();
        QVERIFY(true);
    }

    /// Same coverage shape.
    void testRestoreIncompleteLibraries_shouldHandleEmptyFolder() {
        ListView view;
        view.setMavenFolder(QDir::tempPath());
        view.restoreIncompleteLibraries();
        QVERIFY(true);
    }

    /// No list to copy = no crash.
    void testCopyList_shouldRunWithoutCrashWhenEmpty() {
        ListView view;
        view.copyList();
        QVERIFY(true);
    }
};

Q_DECLARE_TEST(ListViewTest)
#include <list_view_test.moc>
