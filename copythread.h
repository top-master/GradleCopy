#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QtCore/qthread.h>
#include <QtCore/qdir.h>
#include <QDirIterator>

class CopyThread : public QThread {
    Q_OBJECT
public:
    inline CopyThread() : dryRun(false) {}
    ~CopyThread() Q_DEL_OVERRIDE;

    inline void start(const QDir &source_,
                      const QString &target_,
                      bool dryRun_ = false,
                      Priority p = InheritPriority) {
        emit statusChanged("preparing");

        source = source_;
        target = target_;
        dryRun = dryRun_;

        QThread::start(p);
    }

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void domainChanged(const QString &status);
    void statusChanged(const QString &status);

private:
    QDir source;
    QString target;
    bool dryRun;
};

#endif // COPYTHREAD_H
