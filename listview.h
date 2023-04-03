#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QWidget>

class DownloadThread;

namespace Ui {
class ListView;
}

class ListView : public QWidget
{
    typedef QWidget super;

    Q_OBJECT

public:
    explicit ListView(QWidget *parent = 0);
    ~ListView();

    QStringList getViewItems() const;

    void setList(const QStringList &remoteLinks, const QStringList &localFiles);
    inline QStringList getRemotelinks() const { return remoteLinks; }

    inline void setMavenFolder(const QString &path)
    {
        m_mavenFolder = path;
        if (!m_mavenFolder.endsWith(QLatin1Char('/'))) {
            m_mavenFolder.append(QLatin1Char('/'));
        }
    }

    QStringList providerLinks() const;

    bool maybeAbort();

public slots:
    void showDownloads();
    void showFolder(const QString &path);
    void disableIncompleteLibraries();
    void restoreIncompleteLibraries();
    void copyList();

private slots:
    void on_downloadButton_clicked();
    void on_usageInfo_linkActivated(const QString &link);

    void onDownloadBegin(int index, const QString &link);
    void onDownloadFinish(int index, const QString &link);
    void onFinish();

private:
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

private:
    Ui::ListView *ui;
    QStringList remoteLinks;
    QStringList localFiles;
    QString m_mavenFolder;

    QString downloadsDir;
    volatile DownloadThread *downloader;
};

#endif // LISTVIEW_H
