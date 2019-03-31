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
signals:
    void downloaded(const QUrl &url);

private slots:
    void fileDownloaded(QNetworkReply* pReply);

private:
    QNetworkAccessManager m_WebCtrl;
    QNetworkReply *netReply;
    QByteArray m_DownloadedData;
};

#endif // FILEDOWNLOADER_H
