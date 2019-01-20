#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>
#include <QMessageBox>
//QMessageBox(QMessageBox::Information, "Debug", Text).exec();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&mSocket, SIGNAL(readyRead()), this, SLOT(readyPeer()));

    mRoomName = "123";
    ui->RoomName->setText(mRoomName);

    mUserName = "익명";
    ui->UserName->setText(mUserName);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnMessage(const char* text)
{
    mRecvText += text;
    int BeginPos = 0, EndPos = 0;
    while((EndPos = mRecvText.indexOf("#json end")) != -1)
    {
        if((BeginPos = mRecvText.indexOf("#json begin")) != -1)
        {
            BeginPos += 11;
            auto NewJson = QJsonDocument::fromJson(mRecvText.mid(BeginPos, EndPos - BeginPos).toUtf8());
            auto NewName = NewJson["name"].toString("???");
            auto NewText = NewJson["text"].toString("...");

            if(NewText.indexOf("[in-love]") != -1)
            {
                QIcon NewIcon("../image/in-love.png");
                QListWidgetItem* NewItem = new QListWidgetItem(NewIcon,
                    "[" + NewName + "] " + NewText.remove("[in-love]"));
                ui->TalkList->addItem(NewItem);
            }
            else ui->TalkList->addItem("[" + NewName + "] " + NewText);
        }
        mRecvText.remove(0, EndPos + 9);
    }
}

void MainWindow::on_ConnectBtn_clicked()
{
    mSocket.connectToHost(mAddress, 10125);
    if(mSocket.waitForConnected(5000))
    {
        ui->AddressEdit->setEnabled(false);
        ui->RoomName->setEnabled(false);
        ui->UserName->setEnabled(false);
        ui->TalkList->setEnabled(true);
        ui->TalkEdit->setEnabled(true);
    }
}

void MainWindow::on_AddressEdit_textChanged(const QString &arg1)
{
    mAddress = arg1;
}

void MainWindow::on_RoomName_textEdited(const QString &arg1)
{
    mRoomName = arg1;
}

void MainWindow::on_UserName_textEdited(const QString &arg1)
{
    mUserName = arg1;
}

void MainWindow::on_TalkEdit_textEdited(const QString &arg1)
{
    mUserText = arg1;
}

void MainWindow::on_TalkEdit_returnPressed()
{
    QString Msg = "#json begin {";
    Msg += "'type':'chat',";
    Msg += "'room':'" + mRoomName + "',";
    Msg += "'name':'" + mUserName + "',";
    Msg += "'text':'" + mUserText + "'";
    Msg += "} #json end";
    mSocket.write(Msg.toUtf8().constData());
    ui->TalkEdit->setText("");
}

void MainWindow::readyPeer()
{
    QTcpSocket* Peer = (QTcpSocket*) sender();
    qint64 PacketSize = Peer->bytesAvailable();

    if(0 < PacketSize)
    {
        char* PacketBuffer = new char[PacketSize + 1];
        Peer->read(PacketBuffer, PacketSize);
        PacketBuffer[PacketSize] = '\0';
        OnMessage(PacketBuffer);
        delete[] PacketBuffer;
    }
}
