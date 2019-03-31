#include "downloadthread.h"
#include "filedownloader.h"

#include <QDir>
#include <qpixmap.h>


void DownloadThread::run()
{
    if(QDir().mkpath(target) == false) {
        qWarning("failed to make path: %s", qPrintable(target));
        return;
    }

    foreach (const QString &link, this->links) {
        QUrl url(link);

        FileDownloader *fd = new FileDownloader(url);
        fd->setParent(this);
        connect(fd, &FileDownloader::downloaded, this, &DownloadThread::saveData);

        fd->wait();
    }
}

void DownloadThread::saveData(const QUrl &url)
{
    FileDownloader *fd = qobject_cast<FileDownloader *>(sender());
    if(fd) {
        const QByteArray &data = fd->downloadedData();
        fd->deleteLater();

        QString localPath = url.toString();
        localPath = localPath.right(localPath.size() - excludeFromPath.size());

        if(QDir().mkpath(localPath) == false) {
            qWarning("failed to make path: %s", qPrintable(localPath));
            return;
        }

        QFile file(localPath);
        if(file.open(QFile::Unbuffered | QFile::ReadWrite)) {
            file.write(data);
            file.close();
        }
    }
}
