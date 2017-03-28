#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <vector>

class Process;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *e);

public slots:
    void killProcess();
    void stopProcess();
    void continueProcess();

private:
    Ui::MainWindow *ui;
    QStandardItemModel* model;
    std::vector<Process> ps;
};

#endif // MAINWINDOW_H
