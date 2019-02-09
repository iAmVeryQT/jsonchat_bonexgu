#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QSizePolicy>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QIcon icon("../assets/icon/server.png");
    setWindowIcon(icon);
    ui->setupUi(this);

    ui->user->setDisabled(true);
    ui->log->setDisabled(true);

#ifdef Q_OS_WIN
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
#else
    setFixedSize(size());
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ServerOn()
{
    if(!mServer.isListening())
    {
        mServer.listen(QHostAddress::Any, 10125);
        ui->user->setDisabled(false);
        ui->log->setDisabled(false);
        qDebug() << "Server : listen";
    }
}

void MainWindow::ServerOff()
{
    if(mServer.isListening())
    {
        mServer.close();
        ui->user->setDisabled(true);
        ui->log->setDisabled(true);
        qDebug() << "Server : close";
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}
