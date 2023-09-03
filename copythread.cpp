#include "copythread.h"
#include "project-info.h"

#include <QDebug>


CopyThread::~CopyThread()
{
    qDebug("%s operation completed.", this->operation == CopyLibraries
            ? "Copy"
            : "Scan-for-missing");
}

void CopyThread::run() {
    emit statusChanged("running");

    //prepare arguments
    if(!m_target.endsWith(QLatin1Char('/')))
        m_target.append(QLatin1Char('/'));
    source.setFilter(source.filter() | QDir::NoDotAndDotDot);

    if(operation == CopyLibraries) {
        QDirIterator it(source);
        while(it.hasNext()) {
            QDir domainDir = QDir(it.next());
            domainDir.setFilter(domainDir.filter() | QDir::NoDotAndDotDot);

            const QString domainName = domainDir.dirName().replace(QLatin1Char('.'), QLatin1Char('/'));
            const QString domainPath = m_target
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

                            // Don't even log binary-same gradle-caches
                            // (God knows why Gradle did re-download them).
                            QFileInfo target(targetPath);
                            if (target.exists()
                                && ProjectInfo::binarySame(targetPath, sourcePath)
                            ) {
                                continue;
                            }

                            if(!dryRun) {
                                QFile::copy(sourcePath, targetPath);
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

        QStringList filters;
        filters.reserve(2);
        filters << "*.pom";
        filters << "*.pom.backup";

        //search for any library that is not ready for download
        QDirIterator it(m_target, filters, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString pomPath = it.next();

            // Parse POM.
            ProjectInfo info(pomPath);

            // Excludes pom-only packages.
            if (info.parse()) {
                // Undo old GradleCopy version's mistakes.
                if (info.isBackup() && info.isDownloaded()) {
                    info.setVerbose(false);
                    info.restoreIncomplete();
                    info.setVerbose(true);
                    // Rename may fail, else could do:
                    // ```
                    // continue;
                    // ```
                }

                if (info.isParent()) {
                    // TODO: include dependencies of pom-only packages
                    // (into `remoteLinks`, if they don't exist locally).
                    continue;
                }
            }

            if ( ! info.isComplete()) {
                const QString &path = info.packagePath();

                // Skip duplicates.
#ifdef Q_OS_WIN
                const Qt::CaseSensitivity fileCasing = Qt::CaseInsensitive;
#else
                const Qt::CaseSensitivity fileCasing = Qt::CaseSensitive;
#endif
                if (localFiles.contains(path, fileCasing)) {
                    continue;
                }


                QString link;

                link.reserve(path.length() - m_target.length());
                link += path.right(path.length() - m_target.length());

                localFiles += path;
                remoteLinks += link;
            }
        }
        emit listReady(remoteLinks, localFiles);
    }

    emit statusChanged("finished");
}
