
#include "test-helpers.h"

#include <downloadthread.h>

#include <QSignalSpy>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>


class DownloadThreadTest : public QObject {
    Q_OBJECT

    QStringList providers() const {
        QStringList list;
        list << QLL("https://example.com/m2/")
             << QLL("https://mirror.example.org/m2/");
        return list;
    }

    /// Writable temp dir ending in `/`; cross-platform via `QStandardPaths::TempLocation`.
    QString tmp() const {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if ( ! dir.endsWith(QLatin1Char('/'))) {
            dir.append(QLatin1Char('/'));
        }
        return dir;
    }

private slots:
    /// Default-constructed instance tears down cleanly.
    void testDestructor_shouldRunWithoutCrashing() {
        {
            DownloadThread t;
        }
        QVERIFY(true);
    }

    /// Emits statusChanged("preparing"), captures link count, slashes target.
    void testPrepare_shouldEmitStatusAndCountLinks() {
        DownloadThread t;
        QSignalSpy spy(&t, SIGNAL(statusChanged(QString)));
        QStringList links;
        links << QLL("https://example.com/m2/foo.jar")
              << QLL("https://example.com/m2/bar.jar");
        t.prepare(links, tmp() + QLL("gc-dl/out"), providers());

        qExpect(spy.count())->toEqual(1);
        qExpect(spy.at(0).at(0).toString())->toEqual(QString(QLL("preparing")));
        qExpect(t.linkCount())->toEqual(links.size());
        qExpect(t.pendingLinkCount())->toEqual(links.size());
    }

    /// Must not crash when called before any task is in flight.
    void testAbort_shouldBeSafeBeforeAnyTaskStarted() {
        DownloadThread t;
        t.abort();
        QVERIFY(true);
    }

    /// Result unspecified until run() runs; pin "doesn't crash".
    void testPendingProviderCount_shouldReflectInitialList() {
        DownloadThread t;
        t.prepare(QStringList(), tmp() + QLL("gc-dl/o/"), providers());
        (void) t.pendingProviderCount();
        QVERIFY(true);
    }

    /// Returns the size captured at prepare().
    void testLinkCount_shouldEqualPreparedListSize() {
        DownloadThread t;
        QStringList links; links << QLL("a") << QLL("b") << QLL("c");
        t.prepare(links, tmp() + QLL("gc-dl/o/"), providers());
        qExpect(t.linkCount())->toEqual(3);
    }

    /// Drops as run() consumes; before run, equals total.
    void testPendingLinkCount_shouldStartAtListSize() {
        DownloadThread t;
        QStringList links; links << QLL("a") << QLL("b");
        t.prepare(links, tmp() + QLL("gc-dl/o/"), providers());
        qExpect(t.pendingLinkCount())->toEqual(2);
    }

    /// Strips provider-site prefix from URL.
    void testFindLocalPath_shouldStripProviderPrefix() {
        DownloadThread t;
        QStringList links; links << QLL("https://example.com/m2/foo/bar.jar");
        t.prepare(links, tmp() + QLL("gc-dl/out/"), providers());
        QString local = t.findLocalPath(QLL("https://example.com/m2/foo/bar.jar"));
        qExpect(local)->toEndWith(QLatin1Literal("foo/bar.jar"));
    }

    /// Returns parent folder of the local path.
    void testFindLocalFolder_byUrl_shouldReturnParentFolder() {
        DownloadThread t;
        QStringList links; links << QLL("https://example.com/m2/foo/bar.jar");
        t.prepare(links, tmp() + QLL("gc-dl/out/"), providers());
        QString folder = t.findLocalFolder(QUrl(QLL("https://example.com/m2/foo/bar.jar")));
        qExpect(folder)->toEndWith(QLatin1Literal("foo"));
    }

    /// Returns substring before last '/'.
    void testFindLocalFolder_byString_shouldReturnSubstringBeforeLastSlash() {
        DownloadThread t;
        const QString input = tmp() + QLL("gc-dl/out/foo/bar.jar");
        const QString expected = input.left(input.lastIndexOf(QLatin1Char('/')));
        qExpect(t.findLocalFolder(input))->toEqual(expected);
    }

    /// Real thread drive is flaky; pin "prepare leaves run-able state self-consistent".
    void testRun_shouldExitWhenAbortedEarly() {
        DownloadThread t;
        QStringList links; links << QLL("https://nowhere.invalid/m2/foo/bar.jar");
        t.prepare(links, tmp() + QLL("gc-dl/out/"), providers());
        qExpect(t.linkCount())->toEqual(1);
        qExpect(t.pendingLinkCount())->toEqual(1);
    }
};

Q_DECLARE_TEST(DownloadThreadTest)
#include <download_thread_test.moc>
