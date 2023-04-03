#include "downloadthread.h"
#include "filedownloader.h"
#include "project-info.h"
#include "xd-compat.h"

#include <QApplication>
#include <QDir>
#include <qpixmap.h>


DownloadThread::~DownloadThread()
{
    // Nothing to do (but required).
}

QString DownloadThread::findLocalPath(const QString &urlArg) const
{
    QStringRef url = QStringRef(&urlArg);
    foreach (const QString &provider, providerSiteList) {
        if (url.startsWith(provider, Qt::CaseInsensitive)) {
            url = url.right(url.size() - provider.size());
            break;
        }
    }

    QString localPath;
    localPath.reserve(target.size() + url.size());
    localPath += target;
    localPath += url;

    return localPath;
}

QString DownloadThread::findLocalFolder(const QUrl &url) const
{
    QString path = findLocalPath(url.toString());
    return findLocalFolder(path);
}

QString DownloadThread::findLocalFolder(const QString &localPath) const
{
    int pos = localPath.lastIndexOf(QLatin1Char('/'));
    return localPath.leftRef(pos);
}

void DownloadThread::run()
{
    // Ensures Thread's destruct event is handled on separate thread.
    QT_FINALLY([&] {
        if (this->thread() == this) {
            this->moveToThread(qApp->thread());
        }
    });

    if(QDir().mkpath(target) == false) {
        qWarning("failed to make path: %s", qPrintable(target));
        return;
    }

    for (m.providerIndex = 0; m.providerIndex < providerSiteList.size(); ++m.providerIndex) {
        const QString providerSite = this->providerSiteList.at(m.providerIndex);
        bool isSlashNeeded = !providerSite.endsWith(QLatin1Char('/'));
        qDebug().noquote() << "Trying provider:" << providerSite;

        QMutexLocker taskLocker(&taskLock);
        QStringList::Iterator it = this->links.begin();
        while (it < this->links.end()) {
            if (m.isAborted) {
                break;
            }

            QString link = *it;
            link.reserve(providerSite.size() + 1 + link.size());
            if (isSlashNeeded) {
                link.prepend(QLatin1Char('/'));
            }
            link.prepend(providerSite);

            bool didRetry = false;
posSendRequest:
            QUrl url(link);
            FileDownloader *fd = new FileDownloader(url);
            fd->setParent(this);
            fd->m_link = link;
            connect(fd, &FileDownloader::byResult, this, &DownloadThread::catchResult);

            // Notify.
            int index = this->linkCount() - (this->links.end() - it);
            emit byDownloadBegin(index, link);

            // Cleanup.
            m.task = fd;
            taskLocker.unlock();
            fd->wait();
            bool isSuccess = fd->lastError() == QNetworkReply::NoError;
            taskLocker.relock();
            m.task = Q_NULLPTR;
            delete fd;

            // Notify success or retry.
            if (isSuccess) {
                emit byDownload(index, link);
                it = this->links.erase(it);
            } else if ( ! didRetry) {
                didRetry = true;
                QString suffix = ProjectInfo::platformSuffix(link.right(3));
                link.chop(4);
                link += suffix;
                goto posSendRequest;
            } else {
                ++it;
            }
        }
    }

    // Log result report.
    const int successCount = (this->linkCount() - this->pendingLinkCount());
    QDebug dbg = qDebug();
    dbg << "\nFinished downloading successfully for:"
        << successCount
        << "package(s) out of:" << this->linkCount();
    if (m.notFoundCount) {
        dbg << "package(s)\nIgnored 404 (Not-found) error(s) which we got for:"
            << m.notFoundCount << "links";
    }
}

void DownloadThread::saveData(const QUrl &url, const QByteArray &data)
{
    QString localPath = this->findLocalPath(url.toString());
    QString localFolder = this->findLocalFolder(localPath);

    if(QDir().mkpath(localFolder) == false) {
        qWarning("failed to make path: %s", qPrintable(localPath));
        return;
    }

    QFile file(localPath);
    if(file.open(QFile::Unbuffered | QFile::ReadWrite)) {
        file.write(data);
        file.close();
    }
}

void DownloadThread::catchResult(QNetworkReply *reply, int errorCode)
{
    QString errorMessage;

    if (errorCode == QNetworkReply::NoError) {
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                .toInt();

        if (httpStatus >= 200 && httpStatus <= 299) {
            FileDownloader *fd = qobject_cast<FileDownloader *>(sender());
            if(fd) {
                this->saveData(reply->url(), fd->downloadedData());
                fd->deleteLater();
            }
            return;
        } else if (httpStatus == 404) {
            errorCode = QNetworkReply::ContentNotFoundError;
            errorMessage = QLL("Not found 404");
        } else {
            errorCode = QNetworkReply::UnknownServerError;
            errorMessage = QString(QLL("Invalid HTTP status: %1"))
                    .arg(httpStatus);
        }
    }

    // Ignore errors unless is last provider.
    if (m.providerIndex + 1 < providerSiteList.size()) {
        return;
    }

    FileDownloader *fd = qobject_cast<FileDownloader *>(sender());

    switch (errorCode) {
    case QNetworkReply::ContentNotFoundError:
        if (fd) {
            // TODO: Maybe prevent repeated counting of same `fd->link()`.
            m.notFoundCount.ref();
        }
        break;

    default:
        if (reply && errorCode == reply->error()) {
            errorMessage = reply->errorString();
        }
        this->logError(errorCode, errorMessage);
    }
}

void DownloadThread::logError(int type, const QString &message)
{
    if ( ! message.isEmpty()) {
        qDebug("Error %d: %s", type, qPrintable(message));
    } else {
        qDebug("Unknown error: %d", type);
    }
}
