#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H


#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject {
    Q_OBJECT
public:
    explicit FileDownloader(QUrl url, QObject *parent = 0);
    virtual ~FileDownloader() Q_DEL_OVERRIDE;
    QByteArray downloadedData() const;

    void wait();
    void abort() { emit byAbort(); }

    QNetworkReply::NetworkError lastError() { return m_lastError; }
    QString lastErrorString() const { return m_lastErrorString; }

    inline QString link() const { return m_link; }

    enum {
        Second = 1000,
        Timeout = 30 * Second
    };

signals:
    void downloaded(const QUrl &url);
    void byResult(QNetworkReply *, QNetworkReply::NetworkError);
    void byAbort();

private slots:
    void fileDownloaded(QNetworkReply* pReply);
#ifndef QT_NO_OPENSSL
    void catchSslError(QNetworkReply*, const QList<QSslError> &);
#endif
    void catchError(QNetworkReply::NetworkError);
    void onTimeout();

private:
    friend class DownloadThread;

    QNetworkAccessManager m_WebCtrl;
    /// Ownership is external.
    QNetworkReply *netReply;
    QByteArray m_DownloadedData;

    QNetworkReply::NetworkError m_lastError;
    QString m_lastErrorString;

    QString m_link;
};

#endif // FILEDOWNLOADER_H
