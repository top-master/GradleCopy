#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <qtest.h>
#include <qvector.h>

template <typename T>
struct QTestRegisterHelper {
    enum { Size = sizeof(T) };

    static void *Construct(void *where)
    {
        return new (where) T();
    }
};

class QTestRunner {
public:
    QTestRunner();

    typedef void *(*Constructor)(void *);

    static void registerTest(const char *typeName, Constructor constructor, int size);
    template<typename T>
    static Q_INLINE_TEMPLATE const char *registerTest(const char *typeName) {
        registerTest(typeName, QTestRegisterHelper<T>::Construct, QTestRegisterHelper<T>::Size);
        return typeName;
    }

    static int run(int argc, char **argv);

    struct Entry {
        /// Silences compiler "POD constructed ..." warning.
        inline Entry() {}

        const char *typeName;
        int size;
        QTestRunner::Constructor constructor;
    };
};

#define Q_DECLARE_TEST(classType) static const char * const qOneTestPerFile = QTestRunner::registerTest<classType>(#classType);

namespace QTest {

QString qLastError();

} // namespace QTest

#define QFAIL_LASTERROR() QFAIL(qPrintable(QTest::qLastError()))

#define ASSERT_EQUAL(actual, expected) \
do {\
    if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__))\
        throw std::runtime_error("QTest: ASSERT_EQUAL failed.");\
} while (0)

#endif // TEST_HELPERS_H
