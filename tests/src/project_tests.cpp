
#include "test-helpers.h"


static const char STR_SAMPLE[] = "Some text";
enum {
    STR_SAMPLE_LEN = sizeof(STR_SAMPLE) / sizeof(STR_SAMPLE[0]) - 1
};


class GeneralTest : public QObject {
    Q_OBJECT
public:
    GeneralTest() {
    }

private slots:
    void initTestCase() {
    }

    void cleanupTestCase() {
    }

    void test_simpleUnitTest() {
        std::string encoded = STR_SAMPLE;

        QCOMPARE(encoded.size(), (size_t) STR_SAMPLE_LEN);
        int difference = encoded.compare(0, encoded.size(), STR_SAMPLE, STR_SAMPLE_LEN);
        QCOMPARE(difference, 0);
    }

    void test_simpleFail() {
        //QVERIFY2(false, "This always fails, just ignore it ;-)");
    }
};

Q_DECLARE_TEST(GeneralTest)
#include <project_tests.moc>
