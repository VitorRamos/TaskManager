#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/types.h>
#include <dirent.h>
#include <QStandardItemModel>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <QAction>

#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

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

typedef struct statstruct_proc {
  int           pid;                      /** The process id. **/
  char          exName [_POSIX_PATH_MAX]; /** The filename of the executable **/
  char          state; /** 1 **/          /** R is running, S is sleeping,
               D is sleeping in an uninterruptible wait,
               Z is zombie, T is traced or stopped **/
  unsigned      euid,                      /** effective user id **/
                egid;                      /** effective group id */
  int           ppid;                     /** The pid of the parent. **/
  int           pgrp;                     /** The pgrp of the process. **/
  int           session;                  /** The session id of the process. **/
  int           tty;                      /** The tty the process uses **/
  int           tpgid;                    /** (too long) **/
  unsigned int	flags;                    /** The flags of the process. **/
  unsigned int	minflt;                   /** The number of minor faults **/
  unsigned int	cminflt;                  /** The number of minor faults with childs **/
  unsigned int	majflt;                   /** The number of major faults **/
  unsigned int  cmajflt;                  /** The number of major faults with childs **/
  int           utime;                    /** user mode jiffies **/
  int           stime;                    /** kernel mode jiffies **/
  int		cutime;                   /** user mode jiffies with childs **/
  int           cstime;                   /** kernel mode jiffies with childs **/
  int           counter;                  /** process's next timeslice **/
  int           priority;                 /** the standard nice value, plus fifteen **/
  unsigned int  timeout;                  /** The time in jiffies of the next timeout **/
  unsigned int  itrealvalue;              /** The time before the next SIGALRM is sent to the process **/
  int           starttime; /** 20 **/     /** Time the process started after system boot **/
  unsigned int  vsize;                    /** Virtual memory size **/
  unsigned int  rss;                      /** Resident Set Size **/
  unsigned int  rlim;                     /** Current limit in bytes on the rss **/
  unsigned int  startcode;                /** The address above which program text can run **/
  unsigned int	endcode;                  /** The address below which program text can run **/
  unsigned int  startstack;               /** The address of the start of the stack **/
  unsigned int  kstkesp;                  /** The current value of ESP **/
  unsigned int  kstkeip;                 /** The current value of EIP **/
  int		signal;                   /** The bitmap of pending signals **/
  int           blocked; /** 30 **/       /** The bitmap of blocked signals **/
  int           sigignore;                /** The bitmap of ignored signals **/
  int           sigcatch;                 /** The bitmap of catched signals **/
  unsigned int  wchan;  /** 33 **/        /** (too long) **/
  int		sched, 		  /** scheduler **/
                sched_priority;		  /** scheduler priority **/

} procinfo;

int get_proc_info(pid_t pid, procinfo * pinfo)
{
  char szFileName [_POSIX_PATH_MAX],
    szStatStr [2048],
    *s, *t;
  FILE *fp;
  struct stat st;

  if (NULL == pinfo) {
    errno = EINVAL;
    return -1;
  }

  sprintf (szFileName, "/proc/%u/stat", (unsigned) pid);

  if (-1 == access (szFileName, R_OK)) {
    return (pinfo->pid = -1);
  } /** if **/

  if (-1 != stat (szFileName, &st)) {
    pinfo->euid = st.st_uid;
    pinfo->egid = st.st_gid;
  } else {
    pinfo->euid = pinfo->egid = -1;
  }


  if ((fp = fopen (szFileName, "r")) == NULL) {
    return (pinfo->pid = -1);
  } /** IF_NULL **/

  if ((s = fgets (szStatStr, 2048, fp)) == NULL) {
    fclose (fp);
    return (pinfo->pid = -1);
  }

  /** pid **/
  sscanf (szStatStr, "%u", &(pinfo->pid));
  s = strchr (szStatStr, '(') + 1;
  t = strchr (szStatStr, ')');
  strncpy (pinfo->exName, s, t - s);
  pinfo->exName [t - s] = '\0';

  sscanf (t + 2, "%c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
      /*       1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33*/
      &(pinfo->state),
      &(pinfo->ppid),
      &(pinfo->pgrp),
      &(pinfo->session),
      &(pinfo->tty),
      &(pinfo->tpgid),
      &(pinfo->flags),
      &(pinfo->minflt),
      &(pinfo->cminflt),
      &(pinfo->majflt),
      &(pinfo->cmajflt),
      &(pinfo->utime),
      &(pinfo->stime),
      &(pinfo->cutime),
      &(pinfo->cstime),
      &(pinfo->counter),
      &(pinfo->priority),
      &(pinfo->timeout),
      &(pinfo->itrealvalue),
      &(pinfo->starttime),
      &(pinfo->vsize),
      &(pinfo->rss),
      &(pinfo->rlim),
      &(pinfo->startcode),
      &(pinfo->endcode),
      &(pinfo->startstack),
      &(pinfo->kstkesp),
      &(pinfo->kstkeip),
      &(pinfo->signal),
      &(pinfo->blocked),
      &(pinfo->sigignore),
      &(pinfo->sigcatch),
      &(pinfo->wchan));
  fclose (fp);
  return 0;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    DIR *dp;
    dirent *dptr;
    dp= opendir("/proc/");

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

    /*
    for(auto& p: ps)
    {
        string path= "/proc/"+p.getPid()+"/status";
        string buffer;
        ifstream abre(path.c_str());
        //cout << info.exName << " " << cpu_use << endl;
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
    */

    model= new QStandardItemModel(ps.size(),6,this);
    model->setHeaderData(0, Qt::Horizontal, QVariant("PID"));
    model->setHeaderData(1, Qt::Horizontal, QVariant("NAME"));
    model->setHeaderData(2, Qt::Horizontal, QVariant("CPU%"));
    model->setHeaderData(3, Qt::Horizontal, QVariant("STATE"));
    model->setHeaderData(4, Qt::Horizontal, QVariant("PPID"));
    model->setHeaderData(5, Qt::Horizontal, QVariant("MEMORY"));
    ui->tableView->setModel(model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSortingEnabled(true);

    struct sysinfo s_info;
    sysinfo(&s_info);

    for(int i=0; i<ps.size(); i++)
    {
        pid_t pid=  QString::fromStdString(ps[i].getPid()).toFloat();
        procinfo info;
        get_proc_info(pid, &info);

        QStandardItem* pid_= new QStandardItem;
        //pid_->setText(QString::number((info.pid)));
        pid_->setData(info.pid, Qt::DisplayRole);
        model->setItem(i, 0, pid_);

        QStandardItem* exName= new QStandardItem;
        exName->setText(QString::fromStdString(info.exName));
        model->setItem(i, 1, exName);

        float total_time= info.stime+info.utime;
        float hz= sysconf(_SC_CLK_TCK);
        float sec= s_info.uptime-(info.starttime/hz);
        float cpu_use= 100*((total_time/hz)/sec);

        QStandardItem* cpu_use_= new QStandardItem;
        //cpu_use_->setText(QString::number(cpu_use));
        cpu_use_->setData(cpu_use, Qt::DisplayRole);
        model->setItem(i, 2, cpu_use_);

        QStandardItem* state= new QStandardItem;
        QString estado(info.state);
        state->setText(estado);
        model->setItem(i, 3, state);

        QStandardItem* ppid= new QStandardItem;
        //ppid->setText(QString::number(info.ppid));
        ppid->setData(info.ppid, Qt::DisplayRole);
        model->setItem(i, 4, ppid);

        QStandardItem* vsize= new QStandardItem;
        //vsize->setText(QString::number(info.vsize/10E6));
        vsize->setData(info.vsize/10E6, Qt::DisplayRole);
        model->setItem(i, 5, vsize);
    }
    ui->tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* killpid = new QAction("kill");
    QAction* stoppid = new QAction("stop");
    QAction* continuepid = new QAction("continue");
    connect(killpid, SIGNAL(triggered()), this, SLOT(killProcess()));
    connect(killpid, SIGNAL(triggered()), this, SLOT(stopProcess()));
    connect(killpid, SIGNAL(triggered()), this, SLOT(continueProcess()));
    ui->tableView->addAction(killpid);
    ui->tableView->addAction(stoppid);
    ui->tableView->addAction(continuepid);
    //ui->listView->add;
    startTimer(500);
}

void MainWindow::killProcess()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    foreach( QModelIndex index, selectedRows )
    {
        int row = index.row();
    }
}

void MainWindow::stopProcess()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    foreach( QModelIndex index, selectedRows )
    {
        int row = index.row();
    }
}

void MainWindow::continueProcess()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    foreach( QModelIndex index, selectedRows )
    {
        int row = index.row();
    }
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    struct sysinfo s_info;
    sysinfo(&s_info);
    for(int i=0; i<ps.size(); i++)
    {
        pid_t pid=  QString::fromStdString(ps[i].getPid()).toFloat();
        procinfo info;
        get_proc_info(pid, &info);

        QString nome= QString(info.exName);

        if( !nome.contains(ui->lineEdit->text()) && ui->lineEdit->text() != "")
        {
            ui->tableView->hideRow(i);
        }
        else
        {
            ui->tableView->showRow(i);
        }

        float total_time= info.stime+info.utime;
        float hz= sysconf(_SC_CLK_TCK);
        float sec= s_info.uptime-(info.starttime/hz);
        float cpu_use= 100*((total_time/hz)/sec);

        QStandardItem* cpu_use_;
        cpu_use_= model->item(i,2);
        cpu_use_->setData(cpu_use, Qt::DisplayRole);
        //cpu_use_->setText(QString::number(cpu_use));

        QStandardItem* vsize;
        vsize= model->item(i,5);
        vsize->setData(info.rss, Qt::DisplayRole);
        //vsize->setText(QString::number(info.rss*(size_t)sysconf( _SC_PAGESIZE)));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
