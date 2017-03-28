#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <vector>

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
    void AddNewProcess(std::string pid);

public slots:
    void killProcess();
    void stopProcess();
    void continueProcess();

private slots:
    void on_pushButtonKill_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonCont_clicked();


private:
    Ui::MainWindow *ui;
    QStandardItemModel* model;
    std::vector<std::string> ps;
};

#endif // MAINWINDOW_H
