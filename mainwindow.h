#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QMutex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void log(const QString &);
private slots:
    void on_browseButton_clicked();

    void on_copyButton_clicked();

    void on_selectTargetButton_clicked();

private:
    Ui::MainWindow *ui;

    static QBasicMutex mutex;
    static MainWindow *instance;
};

#endif // MAINWINDOW_H
