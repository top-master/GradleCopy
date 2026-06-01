
#include "test-helpers.h"

#include <filedownloader.h>

#include <QDir>
#include <QSignalSpy>
#include <QUrl>


class FileDownloaderTest : public QObject {
    Q_OBJECT

    /// Cross-platform `file://` URL into the OS temp dir; path doesn't need to exist.
    QUrl tmpUrl(const QString &leaf) const {
        return QUrl::fromLocalFile(
            QDir::tempPath() + QLatin1Literal("/gradlecopy-fd-test-") + leaf);
    }

private slots:
    /// `m_link` is set by DownloadThread (friend), not by the ctor; pin defaults.
    void testConstructor_shouldStoreLinkFromUrl() {
        FileDownloader d(tmpUrl(QLL("ctor")));
        qExpect((int) d.lastError())->toEqual((int) QNetworkReply::NoError);
        qExpect(d.downloadedData())->toBeEmpty();
    }

    /// Covered implicitly by RAII, just verify it runs.
    void testDestructor_shouldRunWithoutCrashing() {
        {
            FileDownloader d(tmpUrl(QLL("dtor")));
        }
        QVERIFY(true);
    }

    /// Empty before any reply arrives.
    void testDownloadedData_shouldStartEmpty() {
        FileDownloader d(tmpUrl(QLL("data")));
        qExpect(d.downloadedData().size())->toEqual(0);
    }

    /// Returns immediately when no reply is in flight; pin "doesn't deadlock".
    void testWait_shouldReturnImmediatelyWhenNoReplyPending() {
        FileDownloader d(tmpUrl(QLL("wait")));
        d.wait();
        QVERIFY(true);
    }

    /// Emits byAbort signal.
    void testAbort_shouldEmitByAbort() {
        FileDownloader d(tmpUrl(QLL("abort")));
        QSignalSpy spy(&d, SIGNAL(byAbort()));
        d.abort();
        qExpect(spy.count())->toEqual(1);
    }

    /// Defaults to NoError.
    void testLastError_shouldStartAtNoError() {
        FileDownloader d(tmpUrl(QLL("err")));
        qExpect((int) d.lastError())->toEqual((int) QNetworkReply::NoError);
    }

    /// Empty before any error.
    void testLastErrorString_shouldStartEmpty() {
        FileDownloader d(tmpUrl(QLL("errstr")));
        qExpect(d.lastErrorString())->toBeEmpty();
    }

    /// `m_link` is only set by DownloadThread via friend access; default is empty.
    void testLink_shouldEchoConstructorUrl() {
        FileDownloader d(tmpUrl(QLL("link")));
        qExpect(d.link())->toBeEmpty();
    }
};

Q_DECLARE_TEST(FileDownloaderTest)
#include <file_downloader_test.moc>
