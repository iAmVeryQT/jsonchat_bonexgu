#include "traymenu.h"
#include "ui_traymenu.h"
#include <QDebug>

traymenu::traymenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::traymenu)
{
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ui->setupUi(this);

    mServerOn = false;

    on_gui_clicked();
    on_onoff_clicked();
}

traymenu::~traymenu()
{
    delete ui;
}

void traymenu::popup()
{
    show();
    activateWindow();
    setFocus();
}

void traymenu::focusOutEvent(QFocusEvent*)
{
    hide();
}

void traymenu::on_exit_clicked()
{
    QApplication::exit(0);
}

void traymenu::on_gui_clicked()
{
    mWindow.show();
}

void traymenu::on_onoff_clicked()
{
    mServerOn = !mServerOn;
    if(mServerOn)
    {
        mWindow.ServerOn();
        ui->onoff->setText("Server OFF");
    }
    else
    {
        mWindow.ServerOff();
        ui->onoff->setText("Server ON");
    }
}
