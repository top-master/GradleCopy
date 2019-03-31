#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QWidget>

namespace Ui {
class ListView;
}

class ListView : public QWidget
{
    Q_OBJECT

public:
    explicit ListView(QWidget *parent = 0);
    ~ListView();

    QStringList getViewItems() const;

    void setList(const QString &providerLink, const QStringList &remoteLinks, const QStringList &localFiles);
    inline QStringList getRemotelinks() const { return remoteLinks; }

public slots:
    void showDownloads();
    void disableIncompleteLibraries();
    void restoreIncompleteLibraries();
    void copyList();

private slots:
    void on_downloadButton_clicked();

    void onDownloadBegin(int index, const QString &link, const QString &localPath);
    void onDownloadFinish(int index, const QString &link, const QString &localPath);
private:
    Ui::ListView *ui;
    QString providerLink;
    QStringList remoteLinks;
    QStringList localFiles;

    QString downloadsDir;
};

#endif // LISTVIEW_H
