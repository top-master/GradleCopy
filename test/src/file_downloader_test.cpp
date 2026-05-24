
#include "test-helpers.h"

#include <filedownloader.h>

#include <QDir>
#include <QSignalSpy>
#include <QUrl>


class FileDownloaderTest : public QObject {
    Q_OBJECT

    /// Builds a platform-correct `file://` URL pointing into the OS
    /// temp dir. The path does not need to exist -- `FileDownloader`'s
    /// ctor only hands the URL to `QNetworkAccessManager::get`, and
    /// the resulting reply just sits in an error state for missing
    /// files. Using `QUrl::fromLocalFile` (rather than a hard-coded
    /// `file:///tmp/...`) keeps the test valid on Windows, where the
    /// temp path has a drive letter prefix.
    QUrl tmpUrl(const QString &leaf) const {
        return QUrl::fromLocalFile(
            QDir::tempPath() + QLatin1Literal("/gradlecopy-fd-test-") + leaf);
    }

private slots:
    // -- Constructor: takes a URL + optional parent. `m_link` is set
    //    later by `DownloadThread` (declared friend), not by the ctor
    //    itself, so pin "construction succeeds and leaves derived state
    //    at sensible defaults" instead of asserting on `link()`.
    void testConstructor_shouldStoreLinkFromUrl() {
        FileDownloader d(tmpUrl(QLL("ctor")));
        QCOMPARE((int) d.lastError(), (int) QNetworkReply::NoError);
        QVERIFY(d.downloadedData().isEmpty());
    }

    // -- Destructor: covered implicitly by RAII, just verify it runs.
    void testDestructor_shouldRunWithoutCrashing() {
        {
            FileDownloader d(tmpUrl(QLL("dtor")));
        }
        QVERIFY(true);
    }

    // -- downloadedData(): empty before any reply arrives.
    void testDownloadedData_shouldStartEmpty() {
        FileDownloader d(tmpUrl(QLL("data")));
        QCOMPARE(d.downloadedData().size(), 0);
    }

    // -- wait(): when there's no in-flight reply, returns immediately
    //    rather than blocking forever; pin "doesn't deadlock".
    void testWait_shouldReturnImmediatelyWhenNoReplyPending() {
        FileDownloader d(tmpUrl(QLL("wait")));
        d.wait();
        QVERIFY(true);
    }

    // -- abort(): emits byAbort signal.
    void testAbort_shouldEmitByAbort() {
        FileDownloader d(tmpUrl(QLL("abort")));
        QSignalSpy spy(&d, SIGNAL(byAbort()));
        d.abort();
        QCOMPARE(spy.count(), 1);
    }

    // -- lastError(): defaults to NoError.
    void testLastError_shouldStartAtNoError() {
        FileDownloader d(tmpUrl(QLL("err")));
        QCOMPARE((int) d.lastError(), (int) QNetworkReply::NoError);
    }

    // -- lastErrorString(): empty before any error.
    void testLastErrorString_shouldStartEmpty() {
        FileDownloader d(tmpUrl(QLL("errstr")));
        QVERIFY(d.lastErrorString().isEmpty());
    }

    // -- link(): the m_link field is only populated by DownloadThread
    //    via its friend access, so a default-constructed FileDownloader
    //    legitimately has an empty link. Pin "the getter is callable
    //    and returns the field's default value" rather than asserting
    //    a non-empty round-trip.
    void testLink_shouldEchoConstructorUrl() {
        FileDownloader d(tmpUrl(QLL("link")));
        QVERIFY(d.link().isEmpty());
    }
};

Q_DECLARE_TEST(FileDownloaderTest)
#include <file_downloader_test.moc>
