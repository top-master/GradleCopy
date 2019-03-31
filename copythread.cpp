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

    if(operation == CopyLibraries) {
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
//                                if(!target.exists()) {
                                    QFile::copy(sourcePath, targetPath);
//                                } else {
//                                    qWarning() << "file already exists:" << targetPath;
//                                }
                            } else
                                qDebug() << "copy:" << sourcePath << "to:" << targetPath;
                        }

                    }
                }
            }
        }
    } else if(operation == GetLinkList) {
        QStringList remoteLinks;
        QStringList localFiles;
        QLatin1Literal providerSite("https://repo1.maven.org/maven2/");

        //search for any library that is not ready for download
        QDirIterator it(target, QStringList() << "*.pom" << "*.pom.backup", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString pomPath = it.next();
            const QString basePath = baseFromPom(pomPath);
            const QString jarPath = basePath + QLatin1Literal(".jar");
            QFileInfo jarInfo(jarPath);
            const QString aarPath = basePath + QLatin1Literal(".aar");
            QFileInfo aarInfo(aarPath);
            if(jarInfo.exists() == false && aarInfo.exists() == false) {
                QString link;

                link.reserve(providerSite.size() + (jarPath.length() - target.length()));
                link += providerSite;
                link += jarPath.right(jarPath.length() - target.length());

                localFiles += jarPath;
                remoteLinks += link;
            }
        }
        emit listReady(providerSite, remoteLinks, localFiles);
    }

    emit statusChanged("finished");
}

QString CopyThread::baseFromPom(const QString &pomPath) const
{
    const QLatin1Literal pomBackupExt = getPomBackupExtension();
    if(pomPath.endsWith(pomBackupExt))
        return pomPath.left(pomPath.size() - pomBackupExt.size());
    return pomPath.left(pomPath.size() - 4);
}
