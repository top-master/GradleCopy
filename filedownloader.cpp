#include "filedownloader.h"

#include <QEventLoop>

FileDownloader::FileDownloader(QUrl url, QObject *parent)
    : QObject(parent), m_WebCtrl(this), netReply(Q_NULLPTR)
{
    connect(&m_WebCtrl, &QNetworkAccessManager::finished,
            this, &FileDownloader::fileDownloaded );

    QNetworkRequest request(url);
    netReply = m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
    m_DownloadedData = pReply->readAll();

    if(netReply == pReply)
        netReply = Q_NULLPTR;
    const QUrl &url = netReply->url();
    pReply->deleteLater();

    emit downloaded(url);
}

QByteArray FileDownloader::downloadedData() const {
    return m_DownloadedData;
}

void FileDownloader::wait()
{
    if(netReply) {
        QEventLoop loop;
        connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }
}
