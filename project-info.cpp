#include "project-info.h"

#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtXml/QDomDocument>
#include <QFileInfo>


ProjectInfo::ProjectInfo(const QString &pomOrBackupFilePath)
    : m_isVerbose(true)
    , m_isParsed(ThreeState::Unknown)
    , m_type(Self::UnknownType)
{
    m_path = pomOrBackupFilePath;
    m_basePath = this->baseFromPom(pomOrBackupFilePath);
}

bool ProjectInfo::parse()
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (m_isParsed != ThreeState::Unknown) {
        return m_isParsed;
    }
    m_isParsed = ThreeState::False;

    // Parse.
    QFile pomFile(this->inputPomPath());
    if ( ! pomFile.exists()) {
        return false;
    }
    QDomDocument doc;
    if ( ! doc.setContent(&pomFile, true, &errorStr, &errorLine, &errorColumn)) {
        qWarning() << "Failed to parse: " << inputPomPath()
                   << "at line" << errorLine << "column" << errorColumn
                   << "message:" << errorStr;
        return false;
    }

    // Detects type.
    QDomElement root = doc.documentElement();
    const QString &packaging = root.firstChildElement("packaging").text();
    const QString &typeName = packaging.trimmed().toLower();
    if (typeName == QLL("pom")) {
        this->m_type = Self::PomOnly;
    } else if (typeName == QLL("jar") || typeName.isEmpty()) {
        this->m_type = Self::Jar;
    } else if (typeName == QLL("aar")) {
        this->m_type = Self::Aar;
    } else if (typeName == QLL("apk")) {
        this->m_type = Self::Apk;
    } else {
        this->m_type = Self::UnknownType;
    }

    m_isParsed = ThreeState::True;
    return true;
}

bool ProjectInfo::isDownloaded() const
{
    if (this->isParent()) {
        return true;
    }
    if (this->isJar() || this->isUnknown()) {
        QFileInfo jarInfo(this->jarPath());
        QFileInfo jarInfoForPlatform(this->jarPathForPlatform());
        if (jarInfo.exists() || jarInfoForPlatform.exists()) {
            return true;
        }
    }
    if (this->isAar() || this->isUnknown()) {
        QFileInfo aarInfo(this->aarPath());
        QFileInfo aarInfoForPlatform(this->aarPathForPlatform());
        if (aarInfo.exists() || aarInfoForPlatform.exists()) {
            return true;
        }
    }
    if (this->isApk()) {
        QFileInfo apkInfo(this->apkPath());
        QFileInfo apkInfoForPlatform(this->apkPathForPlatform());
        if (apkInfo.exists() || apkInfoForPlatform.exists()) {
            return true;
        }
    }

    return false;
}

bool ProjectInfo::isComplete() const
{
    QFileInfo fileInfo(this->pomPath());
    if (fileInfo.exists()) {
        return this->isDownloaded();
    }

    return false;
}

bool ProjectInfo::restoreIncompleteLib(const QString &path)
{
    QString pomFile = path;
    pomFile.chop(3);
    pomFile += QLatin1Literal("pom");

    ProjectInfo info(pomFile);

    return info.restoreIncomplete();
}

ThreeState::Type ProjectInfo::binarySame(const QString &firstFile, const QString &otherFile)
{
    QFile left(firstFile);
    left.open(QFile::ReadOnly);
    QFile right(otherFile);
    right.open(QFile::ReadOnly);
    // Prevent out of memory (just in case).
    if (left.size() > 100 * Self::MB) {
        return ThreeState::Unknown;
    }
    return (left.size() == right.size()
        && left.readAll() == right.readAll())
            ? ThreeState::True
            : ThreeState::False;
}

bool ProjectInfo::restoreIncomplete()
{
    // Don't restore backup if already has needed files.
    QString pomPath = this->pomPath();
    ProjectInfo info(pomPath);
    if (info.parse()) {
        // If backup is same or invalid, no need to keep it.
        if (this->isBackup()
            && (this->binarySameTo(pomPath) || ! this->parse())
        ) {
            return QFile::remove(this->inputPomPath());
        }

        if (info.isComplete()) {
            return false;
        }
    }

    // Restore POM file from backup.
    QString pomBackupPath = info.pomBackupPath();
    if (QFile::exists(pomBackupPath)) {
        if (QFile::exists(pomPath)) {
            if ( ! QFile::remove(pomPath)) {
                qWarning("can not delete: %s", qPrintable(pomPath));

                return false;
            }
        }
        if (QFile::rename(pomBackupPath, pomPath)) {
            if (m_isVerbose) {
                qDebug("restored: %s", qPrintable(pomBackupPath));
            }

            return true;
        } else {
            qWarning("can not rename: %s", qPrintable(pomBackupPath));
        }
    }

    return false;
}

ThreeState::Type ProjectInfo::binarySameTo(const QString &otherFile) const
{
    return Self::binarySame(this->inputPomPath(), otherFile);
}

QString ProjectInfo::baseFromPom(const QString &pomOrBackupPath) const
{
    const QLatin1Literal pomBackupExt = getPomBackupExtension();
    if(pomOrBackupPath.endsWith(pomBackupExt, Qt::CaseInsensitive))
        return pomOrBackupPath.left(pomOrBackupPath.size() - pomBackupExt.size());
    return pomOrBackupPath.left(pomOrBackupPath.size() - 4);
}
