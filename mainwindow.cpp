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
#include <signal.h>

#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

using namespace std;

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
  unsigned long nswap;/** 35 **/
  unsigned long cnswap;/** 36 **/
  int exit_signal;/** 37 **/
  int processor;/** 38 **/
  unsigned rt_priority;/** 39 **/
  unsigned rt_policy;/** 40 **/
  unsigned long long delayacct_blkio_ticks;/** 41 **/
  unsigned long guest_time;/** 42 **/
  unsigned int cguest_time;/** 43 **/

} procinfo;

int get_proc_info(pid_t pid, procinfo * pinfo)
{
  char szFileName [_POSIX_PATH_MAX],
    szStatStr [2048],
    *s, *t;
  FILE *fp;
  struct stat st;

  if (nullptr == pinfo) {
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


  if ((fp = fopen (szFileName, "r")) == nullptr) {
    return (pinfo->pid = -1);
  } /** IF_NULL **/

  if ((s = fgets (szStatStr, 2048, fp)) == nullptr) {
    fclose (fp);
    return (pinfo->pid = -1);
  }

  /** pid **/
  sscanf (szStatStr, "%u", &(pinfo->pid));
  s = strchr (szStatStr, '(') + 1;
  t = strchr (szStatStr, ')');
  strncpy (pinfo->exName, s, t - s);
  pinfo->exName [t - s] = '\0';

  sscanf (t + 2, "%c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u %lu %lu %d %d %u %u %llu %lu %ld",
      /*       1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43*/
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
      &(pinfo->wchan),
      &(pinfo->cnswap),
      &(pinfo->exit_signal),
      &(pinfo->processor),
      &(pinfo->cnswap),
      &(pinfo->exit_signal),
      &(pinfo->processor),
      &(pinfo->rt_priority),
      &(pinfo->rt_policy),
      &(pinfo->delayacct_blkio_ticks),
      &(pinfo->guest_time),
      &(pinfo->cguest_time));
  fclose (fp);
  return 0;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->customPlot->addGraph();
    ui->customPlot->xAxis->setLabel("TIME");
    ui->customPlot->yAxis->setLabel("%CPU");
    ui->customPlot->yAxis->setAutoTickStep(false);
    ui->customPlot->yAxis->setRange(0, 100);
    ui->customPlot->yAxis->setTickStep(10);


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

    ui->tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* killpid = new QAction("kill");
    QAction* stoppid = new QAction("stop");
    QAction* continuepid = new QAction("continue");
    connect(killpid, SIGNAL(triggered()), this, SLOT(killProcess()));
    connect(stoppid, SIGNAL(triggered()), this, SLOT(stopProcess()));
    connect(continuepid, SIGNAL(triggered()), this, SLOT(continueProcess()));
    ui->tableView->addAction(killpid);
    ui->tableView->addAction(stoppid);
    ui->tableView->addAction(continuepid);

    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    DIR *dp;
    dirent *dptr;
    dp= opendir("/proc/");

    if(dp != nullptr)
    while((dptr= readdir(dp)) != nullptr)
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
        {
            AddNewProcess(dptr->d_name);
            ps.push_back(dptr->d_name);
        }
    }
    closedir(dp);

    startTimer(500);
}

void MainWindow::AddNewProcess(string pidName)
{
    struct sysinfo s_info;
    sysinfo(&s_info);

    pid_t pid=  QString::fromStdString(pidName).toFloat();
    procinfo info;
    get_proc_info(pid, &info);
    QList<QStandardItem*> itens;

    QStandardItem* pid_= new QStandardItem;
    pid_->setData(info.pid, Qt::DisplayRole);

    QStandardItem* exName= new QStandardItem;
    exName->setText(QString::fromStdString(info.exName));

    float total_time= info.stime+info.utime;
    float hz= sysconf(_SC_CLK_TCK);
    float sec= s_info.uptime-(info.starttime/hz);
    float cpu_use= 100*((total_time/hz)/sec);

    QStandardItem* cpu_use_= new QStandardItem;
    cpu_use_->setData(cpu_use, Qt::DisplayRole);

    QStandardItem* state= new QStandardItem;
    QString estado(info.state);

    if(estado == "R")
        state->setText("Running");
    else if(estado == "S" || estado == "D")
        state->setText("Sleeping");
    else if(estado == "Z")
        state->setText("Zombie");
    else if(estado == "T")
        state->setText("Stopped");

    //state->setText(estado);

    QStandardItem* ppid= new QStandardItem;
    ppid->setData(info.ppid, Qt::DisplayRole);

    QStandardItem* vsize= new QStandardItem;
    vsize->setData(info.vsize/10E6, Qt::DisplayRole);
    itens << pid_ << exName << cpu_use_ << state << ppid << vsize;

    model->appendRow(itens);
}

void MainWindow::killProcess()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    foreach( QModelIndex index, selectedRows )
    {
        int row = index.row();
        kill(QVariant(model->item(row,0)->data(Qt::DisplayRole)).toInt(), SIGKILL);
    }
}

void MainWindow::stopProcess()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    foreach( QModelIndex index, selectedRows )
    {
        int row = index.row();
        cout << QVariant(model->item(row,0)->data(Qt::DisplayRole)).toInt() << endl;
        kill(QVariant(model->item(row,0)->data(Qt::DisplayRole)).toInt(), SIGSTOP);
    }
}

void MainWindow::continueProcess()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    foreach( QModelIndex index, selectedRows )
    {
        int row = index.row();
        kill(QVariant(model->item(row,0)->data(Qt::DisplayRole)).toInt(), SIGCONT);
    }
}

double tempo=0;

void MainWindow::timerEvent(QTimerEvent *e)
{
    tempo+=0.5;
    ui->customPlot->replot();
    ui->customPlot->graph(0)->removeDataBefore(tempo-12);
    ui->customPlot->xAxis->setRange(tempo + 0.25, 10, Qt::AlignRight);

    DIR *dp;
    dirent *dptr;
    dp= opendir("/proc/");

    vector<string> update_pid;

    if(dp != nullptr)
    while((dptr= readdir(dp)) != nullptr)
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
        {
            update_pid.push_back(dptr->d_name);
        }
    }
    closedir(dp);

    for(int j=0; j<ps.size(); j++)
    {
        bool aindaExiste= false;
        for(int i=0; i<update_pid.size(); i++)
        {
            if(update_pid[i]==ps[j])
                aindaExiste= true;
        }
        if(!aindaExiste)
        {
            QList<QStandardItem*> itens;
            itens= model->findItems(QString::fromStdString(ps[j]),Qt::MatchExactly, 0);
            for(int k=0; k<itens.size(); k++)
            {
                int r= itens[k]->row();
                model->removeRow(r);
            }
        }
    }

    for(int i=0; i<update_pid.size(); i++)
    {
        bool jaEsta= false;
        for(int j=0; j<ps.size(); j++)
        {
            if(update_pid[i]==ps[j])
                jaEsta= true;
        }
        if(!jaEsta)
        {
            AddNewProcess(update_pid[i]);
            ps.push_back(update_pid[i]);
        }
    }
    float totalCPU[2]={0,0};
    struct sysinfo s_info;
    sysinfo(&s_info);
    for(int i=0; i<model->rowCount(); i++)
    {
        QStandardItem* Ppid= model->item(i,0);
        pid_t pid= QVariant(Ppid->data(Qt::DisplayRole)).toInt();

        procinfo info;
        get_proc_info(pid, &info);

        QString nome= QString(info.exName);
        QString spid= QString::number(info.pid);

        if( !nome.contains(ui->lineEdit->text()) && ui->lineEdit->text() != ""
        ||  !spid.contains(ui->lineEdit_2->text()) && ui->lineEdit_2->text() != "" )
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
        totalCPU[info.processor]+=cpu_use;

        QStandardItem* cpu_use_;
        cpu_use_= model->item(i,2);
        cpu_use_->setData(cpu_use, Qt::DisplayRole);

        QStandardItem* vsize;
        vsize= model->item(i,5);
        vsize->setData(info.rss, Qt::DisplayRole);

        QStandardItem* state= model->item(i, 3);
        QString estado(info.state);
        if(estado == "R")
            state->setText("Running");
        else if(estado == "S" || estado == "D")
            state->setText("Sleeping");
        else if(estado == "Z")
            state->setText("Zombie");
        else if(estado == "T")
            state->setText("Stopped");
    }
    //cout << totalCPU[0]/4.0 << " " << totalCPU[1] << endl;
    ui->customPlot->graph(0)->addData(tempo, totalCPU[0]/4.0>100.0?100.0:totalCPU[0]/4.0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonKill_clicked()
{
    killProcess();
}

void MainWindow::on_pushButtonStop_clicked()
{
    stopProcess();
}

void MainWindow::on_pushButtonCont_clicked()
{
    continueProcess();
}
