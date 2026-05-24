
#include "test-helpers.h"

#ifdef Q_OS_WIN
#  include <qt_windows.h>
#else
#  include <cerrno>
#  include <cstring>
#endif

Q_GLOBAL_STATIC(QVector<QTestRunner::Entry>, testTypes)

QTestRunner::QTestRunner()
{
    // Nothing to do (but required).
}

void QTestRunner::registerTest(const char *typeName, Constructor constructor, int size)
{
    Entry &entry = testTypes()->expand();
    entry.typeName = typeName;
    entry.constructor = constructor;
    entry.size = size;
}

#if defined(QT_HAS_XD) && QT_HAS_XD(5, 6, 5)
class MultiTestProvider : public QTest::TestProvider {
    QVector<QTestRunner::Entry> &list;
    int m_index;
    QObject *m_lastTest;
public:
    MultiTestProvider()
        : list(*testTypes())
        , m_index(0)
        , m_lastTest(Q_NULLPTR)
    {}

    ~MultiTestProvider()
    {
        cleanup();
    }

    QObject *next() Q_DECL_OVERRIDE
    {
        cleanup();
        if (m_index < list.size()) {
            const QTestRunner::Entry &info = *list.ptr(m_index);
            ++m_index;

            QObject *test = reinterpret_cast<QObject *>(
                        info.constructor( ::malloc(info.size) )
                    );
            test->setObjectName(QString::fromLocal8Bit(info.typeName));
            m_lastTest = test;
            return test;
        }
        return Q_NULLPTR;
    }

    void cleanup() {
        if (m_lastTest) {
            // Release resources.
            m_lastTest->~QObject();
            free(m_lastTest);
            m_lastTest = Q_NULLPTR;
        }
    }
};

int QTestRunner::run(int argc, char **argv)
{
    MultiTestProvider provider;
    return QTest::qExec(provider, argc, argv);
}
#else
int QTestRunner::run(int argc, char **argv)
{
    int exitCode = 0;

    foreach(const Entry &info, *testTypes()) {
        QObject *test = reinterpret_cast<QObject *>(
                    info.constructor( malloc(info.size) )
                );
        test->setObjectName(QString::fromLocal8Bit(info.typeName));
        int r = QTest::qExec(test, argc, argv);
        if (r && ! exitCode) {
            exitCode = r;
        }

        // Release resources.
        test->~QObject();
        free(test);
    }

    return exitCode;
}
#endif


QString QTest::qLastError()
{
#ifdef Q_OS_WIN
    return QString(QLatin1Literal("Error 0x%1: %2"))
            .arg(GetLastError(), 0, 16)
            .arg(qt_error_string());
#else
    return QString(QLatin1Literal("Error %1: %2"))
            .arg(errno)
            .arg(QString::fromLocal8Bit(strerror(errno)));
#endif
}
