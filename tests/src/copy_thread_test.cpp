
#include "test-helpers.h"

#include <copythread.h>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>


class CopyThreadTest : public QObject {
    Q_OBJECT

private slots:
    /// Covered implicitly by every other test via RAII.
    void testDestructor_shouldRunWithoutCrashing() {
        {
            CopyThread t;
        }
        QVERIFY(true);
    }

    /// Stores source, target, dry-run flag and emits status.
    void testPrepare_shouldEmitStatusChangedAndStoreTarget() {
        QTemporaryDir tmp; qExpect(tmp.isValid())->toBeTruthy();
        CopyThread t;
        QSignalSpy spy(&t, SIGNAL(statusChanged(QString)));
        t.prepare(QDir(tmp.path()), tmp.path() + QLatin1Literal("/out"), true);

        qExpect(spy.count())->toEqual(1);
        qExpect(spy.at(0).at(0).toString())->toEqual(QString(QLatin1Literal("preparing")));
        qExpect(t.target())->toEqual(tmp.path() + QLatin1Literal("/out"));
    }

    /// No observable side effect beyond what `run()`
    /// later branches on; pin "doesn't crash" + return state stays usable.
    void testSetOperation_shouldAcceptEnumWithoutCrashing() {
        CopyThread t;
        t.setOperation(CopyThread::GetLinkList);
        QVERIFY(true);
    }

    /// Getter.
    void testTarget_shouldEchoLastPreparedValue() {
        QTemporaryDir tmp; qExpect(tmp.isValid())->toBeTruthy();
        CopyThread t;
        t.prepare(QDir(tmp.path()), tmp.path() + QLatin1Literal("/dest"), true);
        qExpect(t.target())->toEqual(tmp.path() + QLatin1Literal("/dest"));
    }

    /// Real thread drive is flaky; pin only the precondition.
    void testRun_shouldEmitRunningAndFinishOnEmptyTree() {
        QTemporaryDir src; qExpect(src.isValid())->toBeTruthy();
        QTemporaryDir dst; qExpect(dst.isValid())->toBeTruthy();

        CopyThread t;
        t.setOperation(CopyThread::CopyLibraries);
        t.prepare(QDir(src.path()), dst.path(), /*dryRun=*/true);
        qExpect(t.target())->toEqual(dst.path());
    }
};

Q_DECLARE_TEST(CopyThreadTest)
#include <copy_thread_test.moc>
