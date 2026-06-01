TEMPLATE = app
TARGET = GradleCopy-test-runner
QT *= testlib
CONFIG *= testcase
lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG *= qtestlib
}
DEFINES *= IS_TEST_RUN
