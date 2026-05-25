
#include <src/test-helpers.h>


class MyClass {
public:
    inline MyClass() {}
};

class ExampleTest : public QObject {
    Q_OBJECT
public:
    ExampleTest() {
    }

private slots:
    void initTestCase() {
        // Anything needed to run once, and that before all test-slot.
    }

    void cleanupTestCase() {
        // Anything needed to run once, and that after all test-slot.
    }

    void init() {
        // Anything needed to run before each test-slot, and you can even do
        // assertions here, but assert-failure skips the actual test-slot.
    }

    void cleanup() {
        // Anything needed to run after each test-slot, and you can even do
        // assertions here, but assert-failure marks the test-slot as failed.
    }

    void testMyMethodName_shouldConstructWithoutIssue() {
        // Dummy.
        MyClass *obj = new MyClass();

        // Actual test.
        QVERIFY(obj != Q_NULLPTR);

        delete obj;
    }
};

Q_DECLARE_TEST(ExampleTest)
#include <example-test.moc>
