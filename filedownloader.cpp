#include "filedownloader.h"

#include <QEventLoop>
#include <QTimer>

FileDownloader::FileDownloader(QUrl url, QObject *parent)
    : QObject(parent)
    , m_WebCtrl(this)
    , netReply(Q_NULLPTR)
    , m_lastError(QNetworkReply::NoError)
{
    connect(&m_WebCtrl, &QNetworkAccessManager::finished,
            this, &FileDownloader::fileDownloaded );
#ifndef QT_NO_OPENSSL
    connect(&m_WebCtrl, &QNetworkAccessManager::sslErrors,
            this, &FileDownloader::catchSslError);
#endif

    QNetworkRequest request(url);
    netReply = m_WebCtrl.get(request);
    if (netReply) {
        connect(netReply, static_cast<void (QNetworkReply:: *)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, &FileDownloader::catchError);
        connect(this, SIGNAL(byAbort()), netReply, SLOT(abort()));
    }
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* reply) {
    m_DownloadedData = reply->readAll();
    if(netReply == reply) {
        netReply = Q_NULLPTR;
    }
    const QUrl &url = reply->url();
    reply->deleteLater();

    emit downloaded(url);
    emit byResult(reply, QNetworkReply::NoError);
}

void FileDownloader::catchError(QNetworkReply::NetworkError v)
{
    m_lastError = v;
    if (this->netReply) {
        m_lastErrorString = this->netReply->errorString();
    } else {
        m_lastErrorString = QLL("Unknown error.");
    }
    emit byResult(this->netReply, v);
}

void FileDownloader::onTimeout()
{
    // Cancel download if got nothing yet.
    if (m_DownloadedData.isEmpty()) {
        this->abort();
    }
}

#ifndef QT_NO_OPENSSL
void FileDownloader::catchSslError(QNetworkReply *reply, const QList<QSslError> &errorList)
{
    QString msg = tr("Unexpected SSL error: ");
    QLL separator(";\n");
    foreach (const QSslError &entry, errorList) {
        msg += entry.errorString();
        msg += separator;
    }
    msg.chop(separator.size());
    qWarning() << msg;

    reply->ignoreSslErrors();
}
#endif QT_NO_OPENSSL

QByteArray FileDownloader::downloadedData() const {
    return m_DownloadedData;
}

void FileDownloader::wait()
{
    if(netReply) {
        QEventLoop loop;
        connect(netReply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
        connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
        QTimer::singleShot(FileDownloader::Timeout, this, SLOT(onTimeout()));
        loop.exec();
    }
}
