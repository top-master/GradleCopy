#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QThread>
#include <QStringList>
#include <QUrl>

class DownloadThread : public QThread
{
    Q_OBJECT
public:
    inline DownloadThread() {}

    inline void prepare(const QStringList &links_,
                        const QString &target_,
                        const QString &excludeFromPath_) {
        emit statusChanged("preparing");

        links = links_;
        target = target_;
        excludeFromPath = excludeFromPath_;
    }

protected:
    void run() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void statusChanged(const QString &);
    void onDownloadBegin(int index, const QString &link, const QString &localPath);
    void onDownload(int index, const QString &link, const QString &localPath);

private slots:
    void saveData(const QUrl &url);

private:
    QStringList links;
    QString target;
    QString excludeFromPath;
};

#endif // DOWNLOADTHREAD_H
