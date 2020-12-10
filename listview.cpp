#include "downloadthread.h"
#include "listview.h"
#include "ui_listview.h"

#include <QDesktopServices>
#include <QDir>
#include <qurl.h>
#include <QStringListModel>
#include <qmenu.h>
#include <QClipboard>

ListView::ListView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ListView)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);

    QMenu *menu = new QMenu();
    menu->addAction(QLatin1Literal("open Downloads Dir"), this, SLOT(showDownloads()));
    menu->addAction(QLatin1Literal("Disable incomplete lib (rename .pom with no .jar beside it)"), this, SLOT(disableIncompleteLibraries()));
    menu->addAction(QLatin1Literal("Restore incomplete lib (rename .pom.backup to .pom)"), this, SLOT(restoreIncompleteLibraries()));
    ui->downloadButton->setMenu(menu);

    connect(ui->copyButton, &QPushButton::clicked, this, &ListView::copyList);

    downloadsDir = QDir::currentPath() + QLatin1Literal("/downloads");
}

ListView::~ListView()
{
    delete ui;
}

void ListView::setList(const QString &providerLink, const QStringList &remoteLinks, const QStringList &localFiles)
{
    this->providerLink = providerLink;
    QStringListModel *m = new QStringListModel(localFiles, ui->listView);
    ui->listView->setModel(m);

    this->remoteLinks = remoteLinks;
    this->localFiles = localFiles;
}
QStringList ListView::getViewItems() const
{
    QStringListModel *m = qobject_cast<QStringListModel *>(ui->listView->model());
    if(m)
        return m->stringList();
    return QStringList();
}

void ListView::showDownloads()
{
    QDesktopServices::openUrl( QUrl::fromLocalFile( downloadsDir ) );
}

void ListView::disableIncompleteLibraries()
{
    const QStringList &list = this->localFiles;
    foreach (const QString &jarPath, list) {
        if(!QFile::exists(jarPath)) {
            QString pomFile = jarPath;
            pomFile.chop(3);
            pomFile += QLatin1Literal("pom");
            QString pomFileBackup = pomFile + QLatin1Literal(".backup");
            if(QFile::exists(pomFileBackup) && QFile::exists(pomFile)) {
                if(!QFile::remove(pomFileBackup)) {
                    qWarning("can not delete: %s", qPrintable(pomFileBackup));
                    continue;
                }
            }
            if(!QFile::rename(pomFile, pomFileBackup))
                qWarning("can not rename: %s", qPrintable(pomFile));
        }
    }
}

void ListView::restoreIncompleteLibraries()
{
    const QStringList &list = this->localFiles;
    foreach (const QString &jarPath, list) {
        if(!QFile::exists(jarPath)) {
            QString pomFile = jarPath;
            pomFile.chop(3);
            pomFile += QLatin1Literal("pom");
            QString pomFileBackup = pomFile + QLatin1Literal(".backup");

            if(QFile::exists(pomFileBackup)) {
                if(QFile::exists(pomFile)) {
                    if(!QFile::remove(pomFile)) {
                        qWarning("can not delete: %s", qPrintable(pomFile));
                        continue;
                    }
                }
                if(!QFile::rename(pomFileBackup, pomFile))
                    qWarning("can not rename: %s", qPrintable(pomFileBackup));
                else
                    qWarning("restored: %s", qPrintable(pomFileBackup));
            }
        }
    }
}

void ListView::copyList()
{
    const QStringList &list = getRemotelinks();
    QClipboard *clipboard = qApp->clipboard();
    clipboard->clear();
    clipboard->setText(list.join(QLatin1Literal("\n")));
}

void ListView::on_downloadButton_clicked()
{
    const QStringList &list = getRemotelinks();
    if(list.count() >= 1) {
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(list.count());
        ui->progressBar->setVisible(true);

        DownloadThread *t = new DownloadThread();
        connect(t, &QThread::finished, this, &ListView::onFinish);
        connect(t, &QThread::finished, t, &QThread::deleteLater);
        connect(t, &DownloadThread::onDownloadBegin, this, &ListView::onDownloadBegin);
        connect(t, &DownloadThread::onDownload, this, &ListView::onDownloadFinish);
        t->moveToThread(t);
        t->prepare(list, downloadsDir, providerLink);
        t->start();
    }
}

void ListView::onDownloadBegin(int index, const QString &link, const QString &localPath)
{
    ui->statusView->setText(link);
}

void ListView::onDownloadFinish(int index, const QString &link, const QString &localPath)
{
    ui->progressBar->setValue(index);
}

void ListView::onFinish()
{
    ui->progressBar->setVisible(false);
}
