#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/types.h>
#include <dirent.h>
#include <QStandardItemModel>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

class Process
{
public:
    Process(string pid)
    {
        this->pid= pid;
    }
    void setName(string n)
    {
        name= n;
    }
    void setStatus(string s)
    {
        status= s;
    }
    string getPid()
    {
        return pid;
    }
    string getName()
    {
        return name;
    }
    string getStatus()
    {
        return status;
    }
private:
    string pid, name, status;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    DIR *dp;
    dirent *dptr;
    dp= opendir("/proc/");
    vector<Process> ps;

    if(dp != NULL)
    while((dptr= readdir(dp)) != NULL)
    {
        string sys_[]= {".", "..", "fs", "bus", "irq", "sys", "tty", "acpi", "scsi", "asound", "driver", "sysvipc"};
        bool ok= true;
        for(auto s: sys_)
            if(dptr->d_name == s)
            {
                ok= false;
                break;
            }
        if((int)dptr->d_type == 4 &&  ok)
            ps.push_back(Process(dptr->d_name));
    }
    closedir(dp);

    for(auto& p: ps)
    {
        string path= "/proc/"+p.getPid()+"/status";
        ifstream abre(path.c_str());
        string buffer;
        while(!abre.eof())
        {
            getline(abre, buffer);
            if(buffer.substr(0,5) == "Name:")
            {
                buffer= buffer.substr(6, buffer.size()-6);
                p.setName(buffer);
            }
            if(buffer.substr(0,4) == "Pid:")
            {
                buffer= buffer.substr(5, buffer.size()-5);
                p.setStatus(buffer);
            }
        }
        //cout << endl;
    }

    QStandardItemModel* model= new QStandardItemModel(ps.size(),2,this);
    model->setHeaderData(0, Qt::Horizontal, QVariant("PID"));
    model->setHeaderData(1, Qt::Horizontal, QVariant("NAME"));
    ui->tableView->setModel(model);
    for(int i=0; i<ps.size(); i++)
    {
        QStandardItem* item= new QStandardItem;
        item->setText(QString::fromStdString(ps[i].getPid()));
        model->setItem(i, 0, item);
        item= new QStandardItem;
        item->setText(QString::fromStdString(ps[i].getName()));
        model->setItem(i, 1, item);

        //QListWidgetItem *qitem= new QListWidgetItem;
        //qitem->setText(QString::fromStdString(p.getName()));
        //ui->listWidget->addItem(qitem);
    }

    //ui->listView->add;
}

MainWindow::~MainWindow()
{
    delete ui;
}
