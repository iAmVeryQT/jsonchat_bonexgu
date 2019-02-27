#include "emojipopup.h"
#include "ui_emojipopup.h"

#include <QScrollBar>
#include "mainwindow.h"

EmojiPopup::EmojiPopup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmojiPopup)
{
    setWindowFlags(Qt::Popup | Qt::WindowStaysOnTopHint);
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->hide();
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->tableWidget->setColumnCount(4);
    for(int x = 0; x < 4; ++x)
    {
        int CurX = ui->tableWidget->width() * x / 4;
        int NextX = ui->tableWidget->width() * (x + 1) / 4;
        ui->tableWidget->setColumnWidth(x, NextX - CurX);
    }

    ui->tableWidget->setRowCount(6);
    for(int y = 0; y < 6; ++y)
    {
        int CurY = ui->tableWidget->height() * y / 6;
        int NextY = ui->tableWidget->height() * (y + 1) / 6;
        ui->tableWidget->setRowHeight(y, NextY - CurY);
    }

    for(int y = 0; y < 6; ++y)
    for(int x = 0; x < 4; ++x)
    {
        QIcon NewIcon("../assets/image/crying.png");
        QTableWidgetItem* NewItem = new QTableWidgetItem();
        NewItem->setIcon(NewIcon);
        NewItem->setData(0, QString("[crying]"));
        ui->tableWidget->setItem(y, x, NewItem);
    }
    connect(ui->tableWidget, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(on_itemPressed(QTableWidgetItem*)));
}

EmojiPopup::~EmojiPopup()
{
    delete ui;
}

void EmojiPopup::focusOutEvent(QFocusEvent*)
{
    hide();
}

void EmojiPopup::on_itemPressed(QTableWidgetItem* item)
{
    QString Name = item->data(0).toString();
    ((MainWindow*) parent())->SendEmoji(Name);
}
