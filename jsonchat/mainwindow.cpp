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

    mUserName = QString::fromWCharArray(L"익명");
    ui->UserName->setText(mUserName);
}

MainWindow::~MainWindow()
{
    delete ui;
    foreach(auto CurFile, mFileWorks)
    {
        if(CurFile != nullptr)
        {
            CurFile->remove();
            delete CurFile;
        }
    }
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
            auto SubType = NewJson["subtype"].toString();
            auto Name = NewJson["name"].toString("???");
            auto Text = NewJson["text"].toString("...");

            if(SubType == "fileshare")
            {
                // 데이터
                auto FilePath = NewJson["filepath"].toString();
                auto NewData = new MyData();
                NewData->mSender = Name;
                NewData->mFilePath = FilePath;

                // 버튼
                auto NewButton = new QPushButton(QString::fromWCharArray(L"파일받기"));
                NewButton->setUserData(0, NewData);
                connect(NewButton, SIGNAL(pressed()), this, SLOT(on_DownloadBtn_pressed()));

                // 위젯
                auto NewLayout = new QHBoxLayout();
                NewLayout->addWidget(new QLabel(Text));
                NewLayout->addWidget(NewButton);
                auto NewWidget = new QWidget();
                NewWidget->setLayout(NewLayout);

                // 아이템
                auto NewItem = new QListWidgetItem();
                NewItem->setBackgroundColor(QColor(192, 224, 255));
                NewItem->setSizeHint(QSize(0, 40));
                ui->TalkList->addItem(NewItem);
                ui->TalkList->setItemWidget(NewItem, NewWidget);
            }
            else if(SubType == "getfile")
            {
                auto NewItem = new QListWidgetItem("<" + Name + "> " + Text);
                NewItem->setBackgroundColor(QColor(255, 224, 192));
                ui->TalkList->addItem(NewItem);

                // 송신자가 나인지 확인
                auto Sender = NewJson["sender"].toString();
                if(Sender == mUserName)
                {
                    auto FilePath = NewJson["filepath"].toString();
                    auto FileID = NewJson["fileid"].toString();

                    QFile ReadFile(FilePath);
                    if(ReadFile.open(QFileDevice::ReadOnly))
                    {
                        auto FileSize = ReadFile.size();
                        if(FileSize < 1024 * 100)
                        {
                            QByteArray NewBuffer = ReadFile.read(FileSize);
                            QByteArray NewBase64 = NewBuffer.toBase64();
                            ReadFile.close();

                            QString Msg = "#json begin {";
                            Msg += "'type':'chat',";
                            Msg += "'room':'" + mRoomName + "',";
                            Msg += "'name':'" + mUserName + "',";
                            Msg += "'to':'" + Name + "',";
                            Msg += QString::fromWCharArray(L"'text':'송신완료!',");
                            Msg += "'subtype':'setfile',";
                            Msg += "'fileid':'" + FileID + "',";
                            Msg += "'done':'1',";
                            Msg += QString("'base64':'") + NewBase64.constData() + "'";
                            Msg += "} #json end";
                            mSocket.write(Msg.toUtf8().constData());
                        }
                        else
                        {
                        }
                    }
                }
            }
            else if(SubType == "setfile")
            {
                auto NewItem = new QListWidgetItem("<" + Name + "> " + Text);
                NewItem->setBackgroundColor(QColor(224, 255, 192));
                ui->TalkList->addItem(NewItem);

                auto FileID = NewJson["fileid"].toString().toInt();
                auto Done = NewJson["done"].toString().toInt();
                auto Base64 = NewJson["base64"].toString();

                if(auto CurFile = mFileWorks.at(FileID))
                {
                    QByteArray NewBase64(Base64.toUtf8().constData());
                    QByteArray NewBuffer = QByteArray::fromBase64(NewBase64);
                    CurFile->write(NewBuffer);
                    if(Done == 1)
                    {
                        CurFile->close();
                        auto OldFileName = CurFile->fileName();
                        CurFile->rename(OldFileName.left(OldFileName.length() - 9));
                        delete CurFile;
                        mFileWorks.replace(FileID, nullptr);
                    }
                }
            }
            else
            {
                const int Pos1 = Text.indexOf("[");
                const int Pos2 = Text.indexOf("]");
                if(Pos1 != -1 && Pos1 < Pos2)
                {
                    QString FileTitle = Text.mid(Pos1 + 1, Pos2 - Pos1 - 1);
                    QIcon NewIcon("../assets/image/" + FileTitle + ".png");
                    auto NewItem = new QListWidgetItem(NewIcon,
                        "[" + Name + "] " + Text.remove("[" + FileTitle + "]"));
                    ui->TalkList->addItem(NewItem);
                }
                else ui->TalkList->addItem("[" + Name + "] " + Text);
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
    QFileDialog Dialog(nullptr, QString::fromWCharArray(L"전송할 파일을 선택하세요"));
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
        Msg += "'text':'" + mUserName + QString::fromWCharArray(L"님의 파일공유(")
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
        QFileDialog Dialog(nullptr, QString::fromWCharArray(L"전송받을 폴더를 선택하세요"));
        Dialog.setFileMode(QFileDialog::Directory);
        if(Dialog.exec())
        {
            const QString DirPath = Dialog.selectedFiles()[0];
            auto Data = (MyData*) Button->userData(0);

            QString ShortName;
            int SlashPos = Data->mFilePath.lastIndexOf('/');
            if(SlashPos != -1)
                ShortName = Data->mFilePath.right(Data->mFilePath.length() - SlashPos - 1);
            else ShortName = Data->mFilePath;

            // 파일구성
            QFile* WriteFile = new QFile(DirPath + "/" + ShortName + ".download");
            if(WriteFile->open(QFileDevice::WriteOnly))
            {
                mFileWorks.append(WriteFile);

                QString Msg = "#json begin {";
                Msg += "'type':'chat',";
                Msg += "'room':'" + mRoomName + "',";
                Msg += "'name':'" + mUserName + "',";
                Msg += QString::fromWCharArray(L"'text':'파일받기를 시작합니다',");
                Msg += "'subtype':'getfile',";
                Msg += "'sender':'" + Data->mSender + "',";
                Msg += "'filepath':'" + Data->mFilePath + "',";
                Msg += "'fileid':'" + QString::number(mFileWorks.length() - 1) + "'";
                Msg += "} #json end";
                mSocket.write(Msg.toUtf8().constData());
            }
        }
    }
}
