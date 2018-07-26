#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "copythread.h"

#include <QtGui/QFileDialog>
#include <QtGui/QDesktopServices>


#include <QApplication>

void LogMessageOutput(QtMsgType type, const char *msg)
{
    QString result;
    switch (type) {
    case QtDebugMsg:
        result = msg;
        break;
//    case QtInfoMsg:
//        result = QString::fromPrintf("Info: %s", msg);
//        break;
    case QtWarningMsg:
        result = QString::fromPrintf("Warning: %s", msg);
        break;
    case QtCriticalMsg:
        result = QString::fromPrintf("Critical: %s", msg);
        break;
    case QtFatalMsg:
        result = QString::fromPrintf("Fatal: %s", msg);
        break;
    }

    MainWindow::log(result);

    if(type == QtFatalMsg)
        abort();
}

QBasicMutex MainWindow::mutex;
MainWindow *MainWindow::instance = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString gradlePath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    gradlePath += QLatin1Literal("/.gradle/caches/modules-2/files-2.1");
    ui->addressEdit->setText( gradlePath );

    QMutexLocker locker(&mutex);
    if(instance == Q_NULLPTR)
        instance = this;

    //any log output will wait until this returns
    qInstallMsgHandler(&LogMessageOutput);
}

MainWindow::~MainWindow()
{
    QMutexLocker locker(&mutex);
    delete ui;
    if(instance == this)
        instance = 0;
}

void MainWindow::log(const QString &msg)
{
    QMutexLocker locker(&mutex);
    if(instance) {
        QMetaObject::invokeMethod(instance->ui->logView, "appendPlainText"
                                  , Qt::QueuedConnection, Q_ARG(QString, msg));
    }
}

void MainWindow::on_browseButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this);
    ui->addressEdit->setText(path);
}

void MainWindow::on_selectTargetButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this);
    ui->targetEdit->setText(path);
}

void MainWindow::on_copyButton_clicked()
{
    QDir source = QDir(ui->addressEdit->text());
    QString target = ui->targetEdit->text();
    if(source.exists()) {
        CopyThread *thread = new CopyThread();
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        connect(thread, &CopyThread::statusChanged, this->ui->statusView, &QLabel::setText, Qt::QueuedConnection);
        connect(thread, &CopyThread::domainChanged, this->ui->statusView, &QLabel::setText, Qt::QueuedConnection);
        thread->start(source, target, ui->testRun->isChecked());
    } else {
        QApplication::beep();
        ui->addressEdit->setFocus();
        ui->addressEdit->selectAll();
    }
}
