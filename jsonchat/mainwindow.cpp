#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>
#include <QFileDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QObjectUserData>
#include <QStandardPaths>
#include <QMessageBox>
//QMessageBox(QMessageBox::Information, "Debug", Text).exec();

class MyData : public QObjectUserData
{
public:
    QString mSender;
    QString mFilePath;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&mSocket, SIGNAL(readyRead()), this, SLOT(readyPeer()));

    mAddress = "boss2d.com";
    ui->AddressEdit->setText(mAddress);

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
            auto NewSubType = NewJson["subtype"].toString();
            auto NewName = NewJson["name"].toString("???");
            auto NewText = NewJson["text"].toString("...");

            if(NewSubType == "fileshare")
            {
                // 데이터
                auto NewFilePath = NewJson["filepath"].toString();
                MyData* NewData = new MyData();
                NewData->mSender = NewName;
                NewData->mFilePath = NewFilePath;

                // 버튼
                QPushButton* NewButton = new QPushButton("파일받기");
                NewButton->setUserData(0, NewData);
                connect(NewButton, SIGNAL(pressed()), this, SLOT(on_DownloadBtn_pressed()));

                // 위젯
                QHBoxLayout* NewLayout = new QHBoxLayout();
                NewLayout->addWidget(new QLabel(NewText));
                NewLayout->addWidget(NewButton);
                QWidget* NewWidget = new QWidget();
                NewWidget->setLayout(NewLayout);

                // 아이템
                QListWidgetItem* NewItem = new QListWidgetItem();
                NewItem->setBackgroundColor(QColor(192, 224, 255));
                NewItem->setSizeHint(QSize(0, 40));
                ui->TalkList->addItem(NewItem);

                ui->TalkList->setItemWidget(NewItem, NewWidget);
            }
            else if(NewSubType == "getfile")
            {
                QListWidgetItem* NewItem = new QListWidgetItem("<" + NewName + "> " + NewText);
                NewItem->setBackgroundColor(QColor(255, 224, 192));
                ui->TalkList->addItem(NewItem);
            }
            else
            {
                const int Pos1 = NewText.indexOf("[");
                const int Pos2 = NewText.indexOf("]");
                if(Pos1 != -1 && Pos1 < Pos2)
                {
                    QString FileTitle = NewText.mid(Pos1 + 1, Pos2 - Pos1 - 1);
                    QIcon NewIcon("../assets/image/" + FileTitle + ".png");
                    QListWidgetItem* NewItem = new QListWidgetItem(NewIcon,
                        "[" + NewName + "] " + NewText.remove("[" + FileTitle + "]"));
                    ui->TalkList->addItem(NewItem);
                }
                else ui->TalkList->addItem("[" + NewName + "] " + NewText);
            }
            ui->TalkList->scrollToBottom();
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
        ui->FileBtn->setEnabled(true);
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
    if(auto Peer = qobject_cast<QTcpSocket*>(sender()))
    {
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
}

void MainWindow::on_FileBtn_pressed()
{
    QFileDialog Dialog(nullptr, "전송할 파일을 선택하세요");
    if(Dialog.exec())
    {
        const QString FilePath = Dialog.selectedFiles()[0];
        const int SlashPos = FilePath.lastIndexOf("/");
        const QString ShortName = FilePath.right(FilePath.length() - SlashPos - 1);
        const int64_t FileSize = QFileInfo(FilePath).size();

        QString Msg = "#json begin {";
        Msg += "'type':'chat',";
        Msg += "'room':'" + mRoomName + "',";
        Msg += "'name':'" + mUserName + "',";
        Msg += "'text':'" + mUserName + "님의 파일공유("
            + ShortName + ", " + QString::number(FileSize) + "byte)',";
        Msg += "'subtype':'fileshare',";
        Msg += "'filepath':'" + FilePath + "',";
        Msg += "'filesize':'" + QString::number(FileSize) + "'";
        Msg += "} #json end";
        mSocket.write(Msg.toUtf8().constData());
    }
}

void MainWindow::on_DownloadBtn_pressed()
{
    if(auto Button = qobject_cast<QPushButton*>(sender()))
    {
        QFileDialog Dialog(nullptr, "전송받을 폴더를 선택하세요");
        Dialog.setFileMode(QFileDialog::Directory);
        if(Dialog.exec())
        {
            const QString DirPath = Dialog.selectedFiles()[0];
            auto Data = (MyData*) Button->userData(0);

            QString Msg = "#json begin {";
            Msg += "'type':'chat',";
            Msg += "'room':'" + mRoomName + "',";
            Msg += "'name':'" + mUserName + "',";
            Msg += "'text':'파일받기를 시작합니다',";
            Msg += "'subtype':'getfile',";
            Msg += "'sender':'" + Data->mSender + "',";
            Msg += "'filepath':'" + Data->mFilePath + "'";
            Msg += "} #json end";
            mSocket.write(Msg.toUtf8().constData());
        }
    }
}
