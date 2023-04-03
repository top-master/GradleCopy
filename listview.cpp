#include "downloadthread.h"
#include "listview.h"
#include "ui_listview.h"

#include "project-info.h"

#include <QDesktopServices>
#include <QDir>
#include <qurl.h>
#include <QStringListModel>
#include <qmenu.h>
#include <QClipboard>
#include <QRegularExpression>
#include <QMessageBox>
#include <QDesktopWidget>

ListView::ListView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ListView)
    , downloader(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->progressGroup->setVisible(false);
    ui->usageInfo->setVisible(false);
    ui->usageInfo->setText(QApplication::translate("ListView",
        "<b>Next steps:</b> Move <a href=\"link\">downloads folder</a>'s content to your <a href=\"target\">m2repository folder</a>"
        " (and replace existing).\n", 0, QApplication::UnicodeUTF8));

#ifndef Q_OS_WIN
    // MacOS is complicated.
    ui->usageInfo->setText(ui->usageInfo->text() + QApplication::translate("ListView",
        "<p>WARNING: <b>MacOS</b> users who only see <b>Stop</b> and <b>Replace</b> buttons in the Copy dialog should click Stop;\n"
        "<br>Then they should instead use <b>Merge</b> feature, which is available since MacOS 10.7 Lion, like:\n"
        "<ul>\n"
        "<li>First, open <a href=\"link\">downloads folder</a> and select all of it's content,</li>\n"
        "<li>Hold-down the <b>Option</b> key (Alt),</li>\n"
        "<li>Then at the same time, drag-and-drop said content into m2repository folder,</li>\n"
        "<li>Above should result to seeing <b>Merge</b> button in the Copy dialog, therein simply click <b>Merge</b> button.</li>\n"
        "<li>All done! but click <b>Replace All</b> if a separate dialog asks again.</li>\n"
        "</ul>", 0, QApplication::UnicodeUTF8);
#endif

    // Sub-menu for download button.
    QMenu *menu = new QMenu();
    menu->addAction(QLatin1Literal("open Downloads Dir"), this, SLOT(showDownloads()));
    menu->addAction(QLatin1Literal("Disable incomplete lib (rename .pom with no .jar beside it)"), this, SLOT(disableIncompleteLibraries()));
    menu->addAction(QLatin1Literal("Restore incomplete lib (rename .pom.backup to .pom)"), this, SLOT(restoreIncompleteLibraries()));
    ui->downloadButton->setMenu(menu);

    connect(ui->copyButton, &QPushButton::clicked, this, &ListView::copyList);

    downloadsDir = QDir::currentPath() + QLatin1Literal("/downloads");

    // Ensures window takes smallest size possible.
    this->resize(this->width(), 100);
    // Show at center.
    QRect screen = QApplication::desktop()->availableGeometry(this);
    this->move(screen.width()/2 - this->width()/2, screen.height() * 0.25);
}

ListView::~ListView()
{
    delete ui;
}

void ListView::setList(const QStringList &remoteLinks, const QStringList &localFiles)
{
    QStringListModel *m = new QStringListModel(localFiles, ui->listView);
    ui->listView->setModel(m);

    this->remoteLinks = remoteLinks;
    this->localFiles = localFiles;
}

QStringList ListView::providerLinks() const
{
    QString text = ui->providerStieList->toPlainText();
    QStringList list = text.split(QRegularExpression("\\n", QRegularExpression::DotMatchesEverythingOption), QString::SkipEmptyParts);

    return list;
}

bool ListView::maybeAbort()
{
    if (this->downloader) {
        int r = QMessageBox::question(this, tr("Confirm"), tr("Are you sure to cancel current download?"));
        if (r == QMessageBox::Yes && this->downloader) {
            const_cast<DownloadThread *>(this->downloader)->abort();
        }
        return true;
    }
    return false;
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
    this->showFolder( downloadsDir );
}

void ListView::showFolder(const QString &path)
{
    QDesktopServices::openUrl( QUrl::fromLocalFile( path ) );
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
            ProjectInfo info(pomFile);
            if (info.parse() && info.isComplete()) {
                continue;
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
        ProjectInfo::restoreIncompleteLib(jarPath);
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
    if (this->maybeAbort()) {
        return;
    }
    QStringList list = getRemotelinks();
    if (list.count() >= 1) {
        // Excludes .pom.backup files from list (if user wishes).
        const bool isExcludeding = QMessageBox::Yes != QMessageBox::question(this, tr("Filter"),
                tr("Include disabled packages in the download list?"
                   "\n\n(Maybe to speed up download,"
                   "\nexclude packages that only have .pom.backup file, by clicking No.)"));
        if (isExcludeding) {
            // Prepare.
            const QString backupExt = QLatin1Literal(".backup");
            QString path;
            path.reserve(m_mavenFolder.size() + 100);
            path += m_mavenFolder;

            // Finally, filters.
            QStringList::Iterator i = list.begin();
            while (i != list.end()) {
                path.resize(m_mavenFolder.size());
                path += *i;
                path.chop(3);
                path += QLL("pom");
                if(!QFile::exists(path)) {
                    path += backupExt;
                    if(QFile::exists(path)) {
                        i = list.erase(i);
                        continue;
                    }
                }
                ++i;
            }
            if (list.count() <= 0) {
                QMessageBox::information(this, QString(), tr("Nothing to do (all packages had .pom.backup files)."));
                return;
            }
        }

        // Update view.
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(list.count());
        ui->progressGroup->setVisible(true);
        ui->statusLeftView->clear();
        ui->statusView->clear();
        ui->downloadButton->setText(tr("&Cancel"));

        // Launch download thread.
        DownloadThread *t = new DownloadThread();
        this->downloader = t;
        connect(t, &QThread::finished, this, &ListView::onFinish);
        connect(t, &DownloadThread::byDownloadBegin, this, &ListView::onDownloadBegin);
        connect(t, &DownloadThread::byDownload, this, &ListView::onDownloadFinish);
        t->prepare(list, downloadsDir, this->providerLinks());
        t->start();
    }
}

void ListView::on_usageInfo_linkActivated(const QString &link)
{
    if (link == QLL("target")) {
        this->showFolder( m_mavenFolder );
    } else {
        this->showDownloads();
    }
}


void ListView::onDownloadBegin(int index, const QString &link)
{
    // Inform.
    QString state;
    state.reserve(100);
    state += QString::number(index + 1);
    state += QLL(" of ");
    state += QString::number(this->remoteLinks.count());
    ui->statusLeftView->setText(state);
    ui->statusView->setText(link);

    // Update progress.
    ui->progressBar->setValue(index);

    // Debug.
    DownloadThread *thread = qobject_cast<DownloadThread *>(sender());
    Q_UNUSED(thread)
}

void ListView::onDownloadFinish(int index, const QString &link)
{
    Q_UNUSED(index) Q_UNUSED(link)
    // Nothing to do,
    // because changing progress only on success is wrong.
}

void ListView::onFinish()
{
    ui->progressGroup->setVisible(false);
    ui->usageInfo->setVisible(true);
    ui->downloadButton->setText(tr("&Download"));
    if (this->downloader) {
        const_cast<DownloadThread *>(this->downloader)->deleteLater();
        this->downloader = Q_NULLPTR;
    }
}

void ListView::closeEvent(QCloseEvent *event)
{
    if (this->maybeAbort()) {
        event->ignore();
    } else {
        super::closeEvent(event);
    }
}
