
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
        Q_ASSERT(pom.open(QFile::WriteOnly));
        QTextStream(&pom)
            << "<project><packaging>" << packaging << "</packaging></project>\n";
        pom.close();
        for (int i = 0; i < siblingExts.size(); ++i) {
            QFile sibling(base + QLatin1Char('.') + siblingExts.at(i));
            Q_ASSERT(sibling.open(QFile::WriteOnly));
            sibling.write("dummy");
            sibling.close();
        }
        return pomPath;
    }

private slots:
    void initTestCase() {
        m_tmp = new QTemporaryDir();
        QVERIFY(m_tmp->isValid());
    }

    void cleanupTestCase() {
        delete m_tmp;
        m_tmp = Q_NULLPTR;
    }

    // -- Construction.
    void testConstructor_shouldRememberInputPath() {
        QString pomPath = m_tmp->path() + QLatin1Literal("/ctor-1.0.pom");
        ProjectInfo info(pomPath);
        QCOMPARE(info.inputPomPath(), pomPath);
    }

    // -- Parsing.
    void testParse_shouldReturnFalseWhenPomMissing() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/does-not-exist-1.0.pom"));
        QVERIFY( ! info.parse());
        QVERIFY(info.isUnknown());
    }

    void testParse_shouldDetectJarPackaging() {
        ProjectInfo info(seedPom(QLL("parsejar"), QLL("jar")));
        QVERIFY(info.parse());
        QCOMPARE((int) info.type(), (int) ProjectInfo::Jar);
    }

    void testReparse_shouldRerunDetectionFromScratch() {
        ProjectInfo info(seedPom(QLL("reparse"), QLL("jar")));
        info.parse();
        QVERIFY(info.reparse());
        QCOMPARE((int) info.type(), (int) ProjectInfo::Jar);
    }

    // -- Verbose flag round-trip.
    void testIsVerbose_shouldStartTrueByDefault() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/verbose-1.0.pom"));
        QVERIFY(info.isVerbose());
    }

    void testSetVerbose_shouldFlipState() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/verbose-1.0.pom"));
        info.setVerbose(false);
        QVERIFY( ! info.isVerbose());
    }

    // -- Type predicates.
    void testType_shouldBeUnknownBeforeParse() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/type-1.0.pom"));
        QCOMPARE((int) info.type(), (int) ProjectInfo::UnknownType);
    }

    void testIsParent_shouldOnlyHoldForPomOnlyPackaging() {
        ProjectInfo info(seedPom(QLL("parent"), QLL("pom")));
        info.parse();
        QVERIFY(info.isParent());
        QVERIFY( ! info.isJar());
    }

    void testIsJar_shouldHoldForJarAndBundle() {
        ProjectInfo jar(seedPom(QLL("isjar"), QLL("jar")));
        jar.parse();
        QVERIFY(jar.isJar());

        ProjectInfo bundle(seedPom(QLL("isbundle-asjar"), QLL("bundle")));
        bundle.parse();
        QVERIFY(bundle.isJar());
    }

    void testIsAar_shouldHoldForAarPackagingOnly() {
        ProjectInfo info(seedPom(QLL("isaar"), QLL("aar")));
        info.parse();
        QVERIFY(info.isAar());
        QVERIFY( ! info.isJar());
    }

    void testIsApk_shouldHoldForApkPackagingOnly() {
        ProjectInfo info(seedPom(QLL("isapk"), QLL("apk")));
        info.parse();
        QVERIFY(info.isApk());
        QVERIFY( ! info.isJar());
    }

    void testIsBundle_shouldHoldForBundlePackagingOnly() {
        ProjectInfo info(seedPom(QLL("isbundle"), QLL("bundle")));
        info.parse();
        QVERIFY(info.isBundle());
        QVERIFY( ! info.isAar());
    }

    void testIsUnknown_shouldHoldForUnrecognisedPackaging() {
        ProjectInfo info(seedPom(QLL("unk"), QLL("zip")));
        info.parse();
        QVERIFY(info.isUnknown());
    }

    void testIsBackup_shouldHoldForPomBackupExtension() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/bkp-1.0.pom.backup"));
        QVERIFY(info.isBackup());
    }

    // -- Downloaded / complete checks.
    void testIsDownloaded_shouldHoldOncePackageFileExists() {
        QStringList sibling; sibling << QLL("jar");
        ProjectInfo info(seedPom(QLL("dl"), QLL("jar"), sibling));
        info.parse();
        QVERIFY(info.isDownloaded());
    }

    void testIsComplete_shouldHoldWhenPomAndPackageBothExist() {
        QStringList sibling; sibling << QLL("jar");
        ProjectInfo info(seedPom(QLL("complete"), QLL("jar"), sibling));
        info.parse();
        QVERIFY(info.isComplete());
    }

    // -- Path getters.
    void testInputPomPath_shouldEchoConstructorArg() {
        QString pomPath = m_tmp->path() + QLatin1Literal("/echo-1.0.pom");
        ProjectInfo info(pomPath);
        QCOMPARE(info.inputPomPath(), pomPath);
    }

    void testPomPath_shouldAlwaysEndWithDotPom() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom.backup"));
        QVERIFY(info.pomPath().endsWith(QLatin1Literal(".pom")));
        QVERIFY( ! info.pomPath().endsWith(QLatin1Literal(".pom.backup")));
    }

    void testPomBackupPath_shouldEndWithBackupExtension() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QVERIFY(info.pomBackupPath().endsWith(QLatin1Literal(".pom.backup")));
    }

    void testJarPath_shouldEndWithDotJar() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QVERIFY(info.jarPath().endsWith(QLatin1Literal(".jar")));
    }

    void testAarPath_shouldEndWithDotAar() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QVERIFY(info.aarPath().endsWith(QLatin1Literal(".aar")));
    }

    void testApkPath_shouldEndWithDotApk() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QVERIFY(info.apkPath().endsWith(QLatin1Literal(".apk")));
    }

    void testPackagePath_shouldPickJarForJarPackaging() {
        ProjectInfo info(seedPom(QLL("pkgjar"), QLL("jar")));
        info.parse();
        QCOMPARE(info.packagePath(), info.jarPath());
    }

    void testPackagePath_shouldPickAarForAarPackaging() {
        ProjectInfo info(seedPom(QLL("pkgaar"), QLL("aar")));
        info.parse();
        QCOMPARE(info.packagePath(), info.aarPath());
    }

    void testPackagePath_shouldPickApkForApkPackaging() {
        ProjectInfo info(seedPom(QLL("pkgapk"), QLL("apk")));
        info.parse();
        QCOMPARE(info.packagePath(), info.apkPath());
    }

    void testJarPathForPlatform_shouldEmbedPlatformSuffix() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QString platformJar = info.jarPathForPlatform();
        QVERIFY(platformJar.endsWith(QLatin1Literal(".jar")));
        QVERIFY(platformJar.contains(QLatin1Char('-'))); // -osx., -linux., -windows.
    }

    void testAarPathForPlatform_shouldEmbedPlatformSuffix() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QString platformAar = info.aarPathForPlatform();
        QVERIFY(platformAar.endsWith(QLatin1Literal(".aar")));
    }

    void testApkPathForPlatform_shouldEmbedPlatformSuffix() {
        ProjectInfo info(m_tmp->path() + QLatin1Literal("/p-1.0.pom"));
        QString platformApk = info.apkPathForPlatform();
        QVERIFY(platformApk.endsWith(QLatin1Literal(".apk")));
    }

    void testBasePath_shouldStripPomExtension() {
        QString pomPath = m_tmp->path() + QLatin1Literal("/base-1.0.pom");
        ProjectInfo info(pomPath);
        QVERIFY( ! info.basePath().endsWith(QLatin1Literal(".pom")));
        QVERIFY(pomPath.startsWith(info.basePath()));
    }

    // -- Restore + binary comparisons.
    void testRestoreIncomplete_shouldRenameBackupBackToPom() {
        QString base = m_tmp->path() + QLatin1Literal("/restore-1.0");
        QString pomPath = base + QLatin1Literal(".pom");
        QString backupPath = base + QLatin1Literal(".pom.backup");
        QFile backup(backupPath);
        QVERIFY(backup.open(QFile::WriteOnly));
        backup.write("<project><packaging>jar</packaging></project>");
        backup.close();

        ProjectInfo info(pomPath);
        QVERIFY(info.restoreIncomplete());
        QVERIFY(QFile::exists(pomPath));
        QVERIFY( ! QFile::exists(backupPath));
    }

    void testBinarySameTo_shouldDetectIdenticalContents() {
        QString a = m_tmp->path() + QLatin1Literal("/same-a.pom");
        QString b = m_tmp->path() + QLatin1Literal("/same-b.pom");
        QFile fa(a); QVERIFY(fa.open(QFile::WriteOnly));
        fa.write("identical"); fa.close();
        QFile fb(b); QVERIFY(fb.open(QFile::WriteOnly));
        fb.write("identical"); fb.close();

        ProjectInfo info(a);
        QCOMPARE((int) info.binarySameTo(b), (int) ThreeState::True);
    }

    // -- Static helpers.
    void testGetPomBackupExtension_shouldReturnDotPomBackup() {
        QCOMPARE(QString(ProjectInfo::getPomBackupExtension()),
                 QString(QLatin1Literal(".pom.backup")));
    }

    void testPlatformSuffix_shouldEndWithGivenExtension() {
        QString suffix = ProjectInfo::platformSuffix(QLL("jar"));
        QVERIFY(suffix.endsWith(QLatin1Literal(".jar")));
        QVERIFY(suffix.startsWith(QLatin1Char('-')));
    }

    void testRestoreIncompleteLib_shouldRecoverFromBackup() {
        QString base = m_tmp->path() + QLatin1Literal("/lib-static-1.0");
        QString pomPath = base + QLatin1Literal(".pom");
        QString backupPath = base + QLatin1Literal(".pom.backup");
        QFile backup(backupPath);
        QVERIFY(backup.open(QFile::WriteOnly));
        backup.write("<project><packaging>jar</packaging></project>");
        backup.close();

        // Static helper takes the `.jar` path -- strips 3 chars, appends
        // `pom` to reach the POM file.
        QVERIFY(ProjectInfo::restoreIncompleteLib(base + QLatin1Literal(".jar")));
        QVERIFY(QFile::exists(pomPath));
    }

    void testBinarySame_shouldReturnFalseForDifferingContents() {
        QString a = m_tmp->path() + QLatin1Literal("/diff-a.pom");
        QString b = m_tmp->path() + QLatin1Literal("/diff-b.pom");
        QFile fa(a); QVERIFY(fa.open(QFile::WriteOnly));
        fa.write("alpha"); fa.close();
        QFile fb(b); QVERIFY(fb.open(QFile::WriteOnly));
        fb.write("beta"); fb.close();

        QCOMPARE((int) ProjectInfo::binarySame(a, b),
                 (int) ThreeState::False);
    }

private:
    QTemporaryDir *m_tmp;
};

Q_DECLARE_TEST(ProjectInfoTest)
#include <project_info_test.moc>
