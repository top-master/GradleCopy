#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "copythread.h"
#include "listview.h"

#include <QtGui/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QtGui/qmenu.h>
#include <QtGui/QMessageBox>

#include <QApplication>

QtMsgHandler oldMsgHandler = 0;
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
    oldMsgHandler(type, msg);
    //if(type == QtFatalMsg)
    //    abort();
}

QBasicMutex MainWindow::mutex;
MainWindow *MainWindow::instance = 0;

MainWindow::MainWindow(QWidget *parent)
    : super(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString gradlePath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    gradlePath += QLatin1Literal("/.gradle/caches/modules-2/files-2.1");
    ui->addressEdit->setText( QDir::toNativeSeparators(gradlePath) );

    QString androidPath = QString::fromLocal8Bit(qgetenv("ANDROID_HOME")).trimmed();
    if ( androidPath.isEmpty() ) {
        QMessageBox::warning(this, QLL("Missing SDK"), QLL(
            "Your ANDROID_HOME environment-variable is not set or empty!"
        ));
    } else {
        androidPath += QLatin1String("/extras/m2repository");
        ui->targetEdit->setText( QDir::toNativeSeparators(androidPath) );
    }
    // Adds Drop-down menu to the start-button
    QMenu *menu = new QMenu();
    menu->addAction("Generate Missing Jar List", this, SLOT(generateDownloadList()));
    ui->copyButton->setMenu(menu);

    QMutexLocker locker(&mutex);
    if (instance == Q_NULLPTR) {
        instance = this;
    }
    // Any log output will wait until this returns
    oldMsgHandler = qInstallMsgHandler(&LogMessageOutput);
}

MainWindow::~MainWindow()
{
    QMutexLocker locker(&mutex);
    delete ui;
    if (instance == this) {
        instance = 0;
    }
}

void MainWindow::log(const QString &msg)
{
    QMutexLocker locker(&mutex);
    if (instance) {
        QMetaObject::invokeMethod(instance->ui->logView, "appendPlainText"
                                  , Qt::QueuedConnection, Q_ARG(QString, msg));
    }
}

void MainWindow::showList(const QStringList &remoteLinks, const QStringList &localFiles)
{
    ListView *lv = new ListView();
    lv->setList(remoteLinks, localFiles);
    CopyThread *thread = qobject_cast<CopyThread *>(sender());
    if (thread) {
        lv->setMavenFolder(thread->target());
    }
    lv->showNormal();
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

bool MainWindow::ensureSource(QDir *source)
{
    *source = QDir(ui->addressEdit->text());
    if (source->exists()) {
        return true;
    }
    QApplication::beep();
    ui->addressEdit->setFocus();
    ui->addressEdit->selectAll();
    return false;
}

CopyThread *MainWindow::ensureThread(CopyThread::OperationType ot)
{
    CopyThread *thread = 0;
    QDir source;
    if ( ot == CopyThread::GetLinkList || ensureSource(&source) ) {
        QString target = ui->targetEdit->text();
        thread = new CopyThread();
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        connect(thread, &CopyThread::statusChanged, this->ui->statusView, &QLabel::setText, Qt::QueuedConnection);
        connect(thread, &CopyThread::domainChanged, this->ui->statusView, &QLabel::setText, Qt::QueuedConnection);
        thread->prepare(source, target, ui->testRun->isChecked());
    }
    return thread;
}

void MainWindow::startOperation(CopyThread::OperationType ot)
{
    CopyThread *t = ensureThread(ot);
    if (t) {
        t->setOperation(ot);
        if (ot == CopyThread::GetLinkList) {
            connect(t, &CopyThread::listReady, this, &MainWindow::showList);
        }
        t->start();
    }
}


