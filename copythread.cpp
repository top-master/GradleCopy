#include "copythread.h"

#include <QDebug>


CopyThread::~CopyThread()
{
    qWarning() << "CopyThread deleted";
}

void CopyThread::run() {
    emit statusChanged("running");

    //prepare arguments
    if(!target.endsWith(QLatin1Char('/')))
        target.append(QLatin1Char('/'));
    source.setFilter(source.filter() | QDir::NoDotAndDotDot);

    QDirIterator it(source);
    while(it.hasNext()) {
        QDir domainDir = QDir(it.next());
        domainDir.setFilter(domainDir.filter() | QDir::NoDotAndDotDot);

        const QString domainName = domainDir.dirName().replace(QLatin1Char('.'), QLatin1Char('/'));
        const QString domainPath = target
                + domainName + QLatin1Char('/');

        emit domainChanged(domainName);

        QDirIterator it(domainDir);
        while(it.hasNext()) {
            QDir libraryDir = QDir(it.next());
            libraryDir.setFilter(libraryDir.filter() | QDir::NoDotAndDotDot);

            const QString libraryPath = domainPath + libraryDir.dirName() + QLatin1Char('/');
            QDirIterator it(libraryDir);
            while(it.hasNext()) {
                QDir versionDir = QDir(it.next());
                versionDir.setFilter(versionDir.filter() | QDir::NoDotAndDotDot);
                const QString versionPath = libraryPath + versionDir.dirName() + QLatin1Char('/');

                if(!dryRun && !versionDir.mkpath(versionPath)) {
                    qWarning() << "failed to make path:" << versionPath;
                }

                QDirIterator it(versionDir);
                while(it.hasNext()) {
                    QDir idDir = QDir(it.next());
                    idDir.setFilter(idDir.filter() | QDir::NoDotAndDotDot
                                    | QDir::Files);
                    QDirIterator it(idDir);
                    while(it.hasNext()) {
                        const QString sourcePath = it.next();

                        QFileInfo fileInfo(sourcePath);
                        const QString targetPath = versionPath + fileInfo.fileName();

                        if(!dryRun) {
                            QFileInfo target(targetPath);
                            if(!target.exists()) {
                                QFile::copy(sourcePath, targetPath);
                            } else {
                                qWarning() << "file already exists:" << targetPath;
                            }
                        } else
                            qDebug() << "copy:" << sourcePath << "to:" << targetPath;
                    }

                }
            }
        }
    }
    emit statusChanged("finished");
}
