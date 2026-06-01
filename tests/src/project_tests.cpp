
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

    /// std::string round-trip: harness sanity check that a plain
    /// value keeps its byte length and compares equal to its source.
    void testSampleString_shouldMatchExpectedLengthAndContent() {
        std::string encoded = STR_SAMPLE;

        qExpect(encoded.size())->toEqual((size_t) STR_SAMPLE_LEN);
        int difference = encoded.compare(0, encoded.size(), STR_SAMPLE, STR_SAMPLE_LEN);
        qExpect(difference)->toEqual(0);
    }

    /// Disabled sample: a deliberately failing assertion, kept as a
    /// template for how a failing check reads. Do not enable.
    void testFailureSample_shouldStayDisabled() {
        //QVERIFY2(false, "This always fails, just ignore it ;-)");
    }
};

Q_DECLARE_TEST(GeneralTest)
#include <project_tests.moc>
