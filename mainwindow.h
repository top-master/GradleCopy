#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QMutex>

#include "copythread.h"

class CopyThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    typedef QMainWindow super;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void log(const QString &);
public slots:
    inline void generateDownloadList() { startOperation(CopyThread::GetLinkList); }
    void showList(const QStringList &remoteLinks, const QStringList &localFiles);
    void showListIfAny(const QStringList &remoteLinks, const QStringList &localFiles);
private slots:
    void on_browseButton_clicked();

    inline void on_copyButton_clicked() { startOperation(CopyThread::CopyLibraries); }

    void on_selectTargetButton_clicked();

    void postCopyOperation();

private:
    bool ensureSource(QDir *);
    CopyThread *ensureThread(CopyThread::OperationType ot);
    void startOperation(CopyThread::OperationType ot, bool silent = false);

private:
    Ui::MainWindow *ui;

    static QBasicMutex mutex;
    static MainWindow *instance;
};

#endif // MAINWINDOW_H
