#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QtCore/qthread.h>
#include <QtCore/qdir.h>
#include <QDirIterator>

class CopyThread : public QThread {
    Q_OBJECT
public:
    inline CopyThread()
        : dryRun(false)
        , operation(Nothing)
    {}
    ~CopyThread() Q_DEL_OVERRIDE;

    enum OperationType {
        Nothing,
        CopyLibraries,
        GetLinkList
    };

    inline void prepare(const QDir &source_,
                      const QString &target_,
                      bool dryRun_ = false) {
        emit statusChanged("preparing");

        source = source_;
        m_target = target_;
        dryRun = dryRun_;
    }

    inline void setOperation(OperationType ot) { operation = ot; }

    inline QString target() const { return m_target; }

signals:
    void domainChanged(const QString &status);
    void statusChanged(const QString &status);
    void listReady(const QStringList &links, const QStringList &localFiles);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QDir source;
    QString m_target;
    bool dryRun;
    OperationType operation;
};

#endif // COPYTHREAD_H
