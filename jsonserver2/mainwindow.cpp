#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString ResPath = QCoreApplication::applicationDirPath() + "/../../assets/";
    QIcon icon(ResPath + "icon/server.png");
    setWindowIcon(icon);
    ui->setupUi(this);

    mServer.SetWidgets(ui->room, ui->user, ui->log);

    ui->room->setDisabled(true);
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
        ui->room->setDisabled(false);
        ui->user->setDisabled(false);
        ui->log->setDisabled(false);
        ui->log->addItem("#Server : listen");
    }
}

void MainWindow::ServerOff()
{
    if(mServer.isListening())
    {
        mServer.close();
        ui->room->setDisabled(true);
        ui->user->setDisabled(true);
        ui->log->setDisabled(true);
        ui->log->addItem("#Server : close");
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

void MainWindow::on_admin_clicked()
{
}

void MainWindow::on_blacklist_clicked()
{
}

void MainWindow::on_normal_clicked()
{
}
