#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include "filedownloader.h"

#include <QThread>
#include <QStringList>
#include <QUrl>
#include <QMutex>

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

class FileDownloader;

class DownloadThread : public QThread
{
    Q_OBJECT
public:
    inline DownloadThread()
        : m_linkCount(0)
    {}
    ~DownloadThread();

    inline void prepare(const QStringList &links_,
                        const QString &target_,
                        const QStringList &providerSiteList_) {
        // Prevents our signals/slots from causing crashes,
        // which happen if another thread handles them.
        this->moveToThread(this);

        emit statusChanged("preparing");

        m = Data();
        links = links_;
        m_linkCount = links_.size();
        target = target_;
        if (!target.endsWith(QLatin1Char('/'))) {
            target.append(QLatin1Char('/'));
        }
        providerSiteList = providerSiteList_;
    }

    inline void abort() {
        m.isAborted = true;
        QMutexLocker _(&taskLock);
        if (m.task) {
            m.task->abort();
        }
    }

    int pendingProviderCount() const { return providerSiteList.size() - providerIndex; }
    int linkCount() const { return m_linkCount; }
    int pendingLinkCount() const { return links.size(); }

    /// Excludes provider-site's prefix from URL, to get local-file's path.
    QString findLocalPath(const QString &url) const;
    QString findLocalFolder(const QUrl &url) const;
    QString findLocalFolder(const QString &localPath) const;

protected:
    void run() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void statusChanged(const QString &);
    void byDownloadBegin(int index, const QString &link);
    void byDownload(int index, const QString &link);

private slots:
    void catchResult(QNetworkReply *reply, int errorCode);
    void saveData(const QUrl &url, const QByteArray &data);
    void logError(int type, const QString &message);

private:
    QStringList links;
    int m_linkCount;
    QString target;
    QStringList providerSiteList;
    int providerIndex;

    QMutex taskLock;

    struct Data {
        inline Data()
            : isAborted(false)
            , providerIndex(0)
            , task(Q_NULLPTR)
        {
            notFoundCount = 0;
        }

        volatile bool isAborted;
        int providerIndex;
        QBasicAtomicInt notFoundCount;
        FileDownloader *task;
    } m;
};

#endif // DOWNLOADTHREAD_H
