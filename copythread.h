#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QtCore/qthread.h>
#include <QtCore/qdir.h>
#include <QDirIterator>

class CopyThread : public QThread {
    Q_OBJECT
public:
    inline CopyThread() : operation(Nothing), dryRun(false) {}
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
        target = target_;
        dryRun = dryRun_;
    }

    inline void setOperation(OperationType ot) { operation = ot; }

    inline static QLatin1Literal getPomBackupExtension() { return QLatin1Literal(".pom.backup"); }

signals:
    void domainChanged(const QString &status);
    void statusChanged(const QString &status);
    void listReady(const QString &providerLink, const QStringList &links, const QStringList &localFiles);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString baseFromPom(const QString &pomPath) const;
private:
    QDir source;
    QString target;
    bool dryRun;
    OperationType operation;
};

#endif // COPYTHREAD_H
