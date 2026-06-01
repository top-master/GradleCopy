
#include "test-helpers.h"

#include <project-info.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>


class ProjectInfoTest : public QObject {
    Q_OBJECT

    /// Drops `<m_tmp>/<name>-1.0.pom` (with the given packaging) plus
    /// any sibling artifact files, and returns the absolute pom path.
    QString seedPom(const QString &name,
                    const QString &packaging,
                    const QStringList &siblingExts = QStringList()) {
        QString base = m_tmp->path() + QLatin1Char('/') + name + QLatin1Literal("-1.0");
        QString pomPath = base + QLatin1Literal(".pom");
        QFile pom(pomPath);
        const bool pomOpened = pom.open(QFile::WriteOnly);
        Q_ASSERT(pomOpened);
        Q_UNUSED(pomOpened);
        QTextStream(&pom)
            << "<project><packaging>" << packaging << "</packaging></project>\n";
        pom.close();
        for (int i = 0; i < siblingExts.size(); ++i) {
            QFile sibling(base + QLatin1Char('.') + siblingExts.at(i));
            const bool siblingOpened = sibling.open(QFile::WriteOnly);
            Q_ASSERT(siblingOpened);
            Q_UNUSED(siblingOpened);
            sibling.write("dummy");
            sibling.close();
        }
        return pomPath;
    }

private slots:
    void initTestCase() {
        m_tmp = new QTemporaryDir();
        qExpect(m_tmp->isValid())->toBeTruthy();
    }

    void cleanupTestCase() {
        delete m_tmp;
        m_tmp = Q_NULLPTR;
    }

    /// Construction.
    void testConstructor_shouldRememberInputPath() {
        QString pomPath = m_tmp->path() + QLatin1Literal("/ctor-1.0.pom");
        ProjectInfo info(pomPath);
        qExpect(info.inputPomPath())->toEqual(pomPath);
    }

    /// Parsing.
    void testParse_shouldReturnFalseWhenPomMissing() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/does-not-exist-1.0.pom"));
        qExpect(info.parse())->toBeFalsy();
        qExpect(info.isUnknown())->toBeTruthy();
    }

    void testParse_shouldDetectJarPackaging() {
        ProjectInfo info(seedPom(QLL("parsejar"), QLL("jar")));
        qExpect(info.parse())->toBeTruthy();
        qExpect((int) info.type())->toEqual((int) ProjectInfo::Jar);
    }

    void testReparse_shouldRerunDetectionFromScratch() {
        ProjectInfo info(seedPom(QLL("reparse"), QLL("jar")));
        info.parse();
        qExpect(info.reparse())->toBeTruthy();
        qExpect((int) info.type())->toEqual((int) ProjectInfo::Jar);
    }

    /// Verbose flag round-trip.
    void testIsVerbose_shouldStartTrueByDefault() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/verbose-1.0.pom"));
        qExpect(info.isVerbose())->toBeTruthy();
    }

    void testSetVerbose_shouldFlipState() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/verbose-1.0.pom"));
        info.setVerbose(false);
        qExpect(info.isVerbose())->toBeFalsy();
    }

    /// Type predicates.
    void testType_shouldBeUnknownBeforeParse() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/type-1.0.pom"));
        qExpect((int) info.type())->toEqual((int) ProjectInfo::UnknownType);
    }

    void testIsParent_shouldOnlyHoldForPomOnlyPackaging() {
        ProjectInfo info(seedPom(QLL("parent"), QLL("pom")));
        info.parse();
        qExpect(info.isParent())->toBeTruthy();
        qExpect(info.isJar())->toBeFalsy();
    }

    void testIsJar_shouldHoldForJarAndBundle() {
        ProjectInfo jar(seedPom(QLL("isjar"), QLL("jar")));
        jar.parse();
        qExpect(jar.isJar())->toBeTruthy();

        ProjectInfo bundle(seedPom(QLL("isbundle-asjar"), QLL("bundle")));
        bundle.parse();
        qExpect(bundle.isJar())->toBeTruthy();
    }

    void testIsAar_shouldHoldForAarPackagingOnly() {
        ProjectInfo info(seedPom(QLL("isaar"), QLL("aar")));
        info.parse();
        qExpect(info.isAar())->toBeTruthy();
        qExpect(info.isJar())->toBeFalsy();
    }

    void testIsApk_shouldHoldForApkPackagingOnly() {
        ProjectInfo info(seedPom(QLL("isapk"), QLL("apk")));
        info.parse();
        qExpect(info.isApk())->toBeTruthy();
        qExpect(info.isJar())->toBeFalsy();
    }

    void testIsBundle_shouldHoldForBundlePackagingOnly() {
        ProjectInfo info(seedPom(QLL("isbundle"), QLL("bundle")));
        info.parse();
        qExpect(info.isBundle())->toBeTruthy();
        qExpect(info.isAar())->toBeFalsy();
    }

    void testIsUnknown_shouldHoldForUnrecognisedPackaging() {
        ProjectInfo info(seedPom(QLL("unk"), QLL("zip")));
        info.parse();
        qExpect(info.isUnknown())->toBeTruthy();
    }

    void testIsBackup_shouldHoldForPomBackupExtension() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/bkp-1.0.pom.backup"));
        qExpect(info.isBackup())->toBeTruthy();
    }

    /// Downloaded / complete checks.
    void testIsDownloaded_shouldHoldOncePackageFileExists() {
        QStringList sibling; sibling << QLL("jar");
        ProjectInfo info(seedPom(QLL("dl"), QLL("jar"), sibling));
        info.parse();
        qExpect(info.isDownloaded())->toBeTruthy();
    }

    void testIsComplete_shouldHoldWhenPomAndPackageBothExist() {
        QStringList sibling; sibling << QLL("jar");
        ProjectInfo info(seedPom(QLL("complete"), QLL("jar"), sibling));
        info.parse();
        qExpect(info.isComplete())->toBeTruthy();
    }

    /// Path getters.
    void testInputPomPath_shouldEchoConstructorArg() {
        QString pomPath = m_tmp->path() + QLatin1Literal("/echo-1.0.pom");
        ProjectInfo info(pomPath);
        qExpect(info.inputPomPath())->toEqual(pomPath);
    }

    void testPomPath_shouldAlwaysEndWithDotPom() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom.backup"));
        qExpect(info.pomPath())->toEndWith(QLatin1Literal(".pom"));
        qExpect(info.pomPath())->Not->toEndWith(QLatin1Literal(".pom.backup"));
    }

    void testPomBackupPath_shouldEndWithBackupExtension() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        qExpect(info.pomBackupPath())->toEndWith(QLatin1Literal(".pom.backup"));
    }

    void testJarPath_shouldEndWithDotJar() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        qExpect(info.jarPath())->toEndWith(QLatin1Literal(".jar"));
    }

    void testAarPath_shouldEndWithDotAar() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        qExpect(info.aarPath())->toEndWith(QLatin1Literal(".aar"));
    }

    void testApkPath_shouldEndWithDotApk() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        qExpect(info.apkPath())->toEndWith(QLatin1Literal(".apk"));
    }

    void testPackagePath_shouldPickJarForJarPackaging() {
        ProjectInfo info(seedPom(QLL("pkgjar"), QLL("jar")));
        info.parse();
        qExpect(info.packagePath())->toEqual(info.jarPath());
    }

    void testPackagePath_shouldPickAarForAarPackaging() {
        ProjectInfo info(seedPom(QLL("pkgaar"), QLL("aar")));
        info.parse();
        qExpect(info.packagePath())->toEqual(info.aarPath());
    }

    void testPackagePath_shouldPickApkForApkPackaging() {
        ProjectInfo info(seedPom(QLL("pkgapk"), QLL("apk")));
        info.parse();
        qExpect(info.packagePath())->toEqual(info.apkPath());
    }

    void testJarPathForPlatform_shouldEmbedPlatformSuffix() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QString platformJar = info.jarPathForPlatform();
        qExpect(platformJar)->toEndWith(QLatin1Literal(".jar"));
        qExpect(platformJar.contains(QLatin1Char('-')))->toBeTruthy(); // -osx., -linux., -windows.
    }

    void testAarPathForPlatform_shouldEmbedPlatformSuffix() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QString platformAar = info.aarPathForPlatform();
        qExpect(platformAar)->toEndWith(QLatin1Literal(".aar"));
    }

    void testApkPathForPlatform_shouldEmbedPlatformSuffix() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QString platformApk = info.apkPathForPlatform();
        qExpect(platformApk)->toEndWith(QLatin1Literal(".apk"));
    }

    void testBasePath_shouldStripPomExtension() {
        QString pomPath = m_tmp->path() + QLatin1Literal("/base-1.0.pom");
        ProjectInfo info(pomPath);
        qExpect(info.basePath())->Not->toEndWith(QLatin1Literal(".pom"));
        qExpect(pomPath)->toStartWith(info.basePath());
    }

    /// Restore + binary comparisons.
    void testRestoreIncomplete_shouldRenameBackupBackToPom() {
        QString base = m_tmp->path() + QLatin1Literal("/restore-1.0");
        QString pomPath = base + QLatin1Literal(".pom");
        QString backupPath = base + QLatin1Literal(".pom.backup");
        QFile backup(backupPath);
        qExpect(backup.open(QFile::WriteOnly))->toBeTruthy();
        backup.write("<project><packaging>jar</packaging></project>");
        backup.close();

        ProjectInfo info(pomPath);
        qExpect(info.restoreIncomplete())->toBeTruthy();
        qExpect(QFile::exists(pomPath))->toBeTruthy();
        qExpect(QFile::exists(backupPath))->toBeFalsy();
    }

    void testBinarySameTo_shouldDetectIdenticalContents() {
        QString a = m_tmp->path() + QLatin1Literal("/same-a.pom");
        QString b = m_tmp->path() + QLatin1Literal("/same-b.pom");
        QFile fa(a); qExpect(fa.open(QFile::WriteOnly))->toBeTruthy();
        fa.write("identical"); fa.close();
        QFile fb(b); qExpect(fb.open(QFile::WriteOnly))->toBeTruthy();
        fb.write("identical"); fb.close();

        ProjectInfo info(a);
        qExpect((int) info.binarySameTo(b))->toEqual((int) ThreeState::True);
    }

    /// Static helpers.
    void testGetPomBackupExtension_shouldReturnDotPomBackup() {
        qExpect(QString(ProjectInfo::getPomBackupExtension()))
            ->toEqual(QString(QLatin1Literal(".pom.backup")));
    }

    void testPlatformSuffix_shouldEndWithGivenExtension() {
        QString suffix = ProjectInfo::platformSuffix(QLL("jar"));
        qExpect(suffix)->toEndWith(QLatin1Literal(".jar"));
        qExpect(suffix)->toStartWith(QLatin1Literal("-"));
    }

    void testRestoreIncompleteLib_shouldRecoverFromBackup() {
        QString base = m_tmp->path() + QLatin1Literal("/lib-static-1.0");
        QString pomPath = base + QLatin1Literal(".pom");
        QString backupPath = base + QLatin1Literal(".pom.backup");
        QFile backup(backupPath);
        qExpect(backup.open(QFile::WriteOnly))->toBeTruthy();
        backup.write("<project><packaging>jar</packaging></project>");
        backup.close();

        // Static helper takes the `.jar` path -- strips 3 chars, appends
        // `pom` to reach the POM file.
        qExpect(ProjectInfo::restoreIncompleteLib(base + QLatin1Literal(".jar")))->toBeTruthy();
        qExpect(QFile::exists(pomPath))->toBeTruthy();
    }

    void testBinarySame_shouldReturnFalseForDifferingContents() {
        QString a = m_tmp->path() + QLatin1Literal("/diff-a.pom");
        QString b = m_tmp->path() + QLatin1Literal("/diff-b.pom");
        QFile fa(a); qExpect(fa.open(QFile::WriteOnly))->toBeTruthy();
        fa.write("alpha"); fa.close();
        QFile fb(b); qExpect(fb.open(QFile::WriteOnly))->toBeTruthy();
        fb.write("beta"); fb.close();

        qExpect((int) ProjectInfo::binarySame(a, b))->toEqual((int) ThreeState::False);
    }

private:
    QTemporaryDir *m_tmp;
};

Q_DECLARE_TEST(ProjectInfoTest)
#include <project_info_test.moc>
