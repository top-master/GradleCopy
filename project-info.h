#ifndef POM_INFO_H
#define POM_INFO_H

#include <QtCore/QStringList>

namespace ThreeState {
enum Type {
    Unknown = -1,
    False = 0,
    True = 1,
};
} // namespace ThreeState

/**
 * Common info and tools for handling POM (Maven's "Project Object Model").
 */
class ProjectInfo
{
    typedef ProjectInfo Self;
public:
    ProjectInfo(const QString &pomOrBackupFilePath);

    bool parse();
    inline bool reparse() { m_isParsed = ThreeState::Unknown; return parse(); }

    inline bool isVerbose() const { return m_isVerbose; }
    inline void setVerbose(bool enabled) { m_isVerbose = enabled; }

    enum Type {
        UnknownType,
        PomOnly,
        Jar,
        Aar
    };
    inline Type type() const { return m_type; }

    /**
     * @return \c true for pom-only packages (without any .jar or .aar files).
     *
     * @see parse()
     */
    inline bool isParent() const { return m_type == Self::PomOnly; }
    inline bool isJar() const { return m_type == Self::Jar; }
    inline bool isAar() const { return m_type == Self::Aar; }
    inline bool isUnknown() const { return m_type == Self::UnknownType; }

    /**
     * Whether this instance does query a `*.pom.backup` file, or not.
     */
    inline bool isBackup() const { return inputPomPath().endsWith(getPomBackupExtension(), Qt::CaseInsensitive); }

    /**
     * Whether the related `.jar` or `.aar` is available, if required.
     *
     * Requires parse().
     */
    bool isDownloaded() const;

    /**
     * Same as isDownloaded(), but requires `.pom` file to exist.
     */
    bool isComplete() const;

    /**
     * Path to file queried by this instance.
     *
     * @see parse()
     */
    const QString &inputPomPath() const { return m_path; }

    /**
     * Path to file queried by Gradle.
     *
     * @see isBackup()
     */
    QString pomPath() const { return basePath() + QLatin1Literal(".pom"); }
    QString pomBackupPath() const { return basePath() + Self::getPomBackupExtension(); }
    QString jarPath() const { return basePath() + QLatin1Literal(".jar"); }
    QString aarPath() const { return basePath() + QLatin1Literal(".aar"); }

    /**
     * Platform-specific packages exist, which may be required by Android-Studio only.
     *
     * For example, see `com.android.tools.build:aapt2` package.
     */
    QString jarPathForPlatform() const {
        return basePath() + Self::platformSuffix(QLL("jar"));
    }

    /**
     * Same as {@link #jarPathForPlatform}, but for AAR.
     */
    QString aarPathForPlatform() const {
        return basePath() + Self::platformSuffix(QLL("aar"));
    }

    const QString &basePath() const { return m_basePath; }

    bool restoreIncomplete();
    ThreeState::Type binarySameTo(const QString &otherFile) const;

    enum {
        Byte = 1,
        KB = 1024 * Byte,
        MB = 1024 * KB
    };

public: // Globals.
    static inline QLatin1Literal getPomBackupExtension() { return QLatin1Literal(".pom.backup"); }

    static inline QString platformSuffix(const QString &extension)
    {
#if defined(Q_OS_MAC)
        QLL platform("-osx.");
#elif defined(Q_OS_LINUX)
        QLL platform("-linux.");
#else
        QLL platform("-windows.");
#endif
        QString result;
        result.reserve(platform.size() + extension.size());
        result += platform;
        result += extension;

        return result;
    }

    /**
     * @param path Absolute-path to .jar or .aar file.
     */
    static bool restoreIncompleteLib(const QString &path);

    static ThreeState::Type binarySame(const QString &firstFile, const QString &otherFile);

private:
    QString baseFromPom(const QString &pomOrBackupPath) const;

private:
    bool m_isVerbose;
    ThreeState::Type m_isParsed;
    Type m_type;
    QString m_path;
    QString m_basePath;

public:
    QStringList dependencies;
};

#endif // POM_INFO_H
