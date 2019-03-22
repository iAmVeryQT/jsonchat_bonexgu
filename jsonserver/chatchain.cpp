#include "chatchain.h"
#include "chatserver.h"

#define KOREAN(STR) QString::fromWCharArray(L##STR)

ChatChain::ChatChain()
{
    listen(QHostAddress::Any, 10126);
    mRefServer = nullptr;
    mRefEditor = nullptr;
    mLogWidget = nullptr;

    connect(this, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

ChatChain::~ChatChain()
{
    close();
}

void ChatChain::BroadcastMessage(const QByteArray& bytes)
{
    // 나에게 접속한 다른 서버들에게 전달
    for(auto iSocket : mChainSockets)
    {
        iSocket->write(bytes);
        mLogWidget->addItem(KOREAN("자식체인에게 전달"));
    }

    // 내가 접속한 서버에게도 전달
    if(mMyConnector.state() == QAbstractSocket::ConnectedState)
    {
        mMyConnector.write(bytes);
        mLogWidget->addItem(KOREAN("부모체인에게 전달"));
    }
}

void ChatChain::SetServer(ChatServer* server)
{
    mRefServer = server;
}

void ChatChain::Connect(QString address, QString nick)
{
    auto NewData = new ChainData(false);
    mMyConnector.setUserData(0, NewData);
    mMyConnector.connectToHost(address, 10126);
    if(mMyConnector.waitForConnected(5000))
    {
        QString Msg = "#json begin {";
        Msg += "\"nick\":\"" + nick + "\"";
        Msg += "} #json end";
        mMyConnector.write(Msg.toUtf8().constData());
        connect(&mMyConnector, SIGNAL(readyRead()), this, SLOT(readyPeer()));
    }
}

void ChatChain::SetChainEditor(QLineEdit* editor, QListWidget* log)
{
    mRefEditor = editor;
    mLogWidget = log;
}

void ChatChain::acceptPeer()
{
    QTcpSocket* NewPeer = nextPendingConnection();
    auto NewData = new ChainData(true);
    NewPeer->setUserData(0, NewData);

    mChainSockets.insert(NewData->mID, NewPeer);

    connect(NewPeer, SIGNAL(readyRead()), this, SLOT(readyPeer()));
    connect(NewPeer, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(errorPeer(QAbstractSocket::SocketError)));
}

void ChatChain::readyPeer()
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    JsonParser(CurPeer);
}

void ChatChain::errorPeer(QAbstractSocket::SocketError)
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    auto CurData = (ChainData*) CurPeer->userData(0);

    mChainSockets.remove(CurData->mID);

    mLogWidget->addItem(KOREAN("자식체인제거 : ") + CurData->mNick);
    // 자식체인제거
    mRefEditor->setText(mRefEditor->text().remove(", " + CurData->mNick));
    mRefEditor->setText(mRefEditor->text().remove(CurData->mNick + ", "));
    mRefEditor->setText(mRefEditor->text().remove(CurData->mNick));
}

void ChatChain::JsonParser(QTcpSocket* sock)
{
    auto CurData = (ChainData*) sock->userData(0);
    auto PacketSize = sock->bytesAvailable();
    if(0 < PacketSize)
    {
        QByteArray NewPacket = sock->read(PacketSize);
        CurData->mStringDump += NewPacket;

        int BeginPos = 0, EndPos = 0;
        while((EndPos = CurData->mStringDump.indexOf("#json end")) != -1)
        {
            BeginPos = CurData->mStringDump.indexOf("#json begin");
            if(BeginPos != -1 && BeginPos < EndPos)
            {
                BeginPos += 11;
                OnJson(sock, CurData->mStringDump.mid(BeginPos, EndPos - BeginPos).toUtf8());
            }
            CurData->mStringDump.remove(0, EndPos + 9);
        }
    }
}

void ChatChain::OnJson(QTcpSocket* sock, const QByteArray& bytes)
{
    auto CurData = (ChainData*) sock->userData(0);
    if(CurData->mFirstTime)
    {
        auto NewJson = QJsonDocument::fromJson(bytes);
        CurData->mFirstTime = false;
        CurData->mNick = NewJson["nick"].toString("Noname");

        mLogWidget->addItem(KOREAN("자식체인등록 : ") + CurData->mNick);
        // 자식체인등록
        if(mRefEditor->text().length() == 0)
            mRefEditor->setText(CurData->mNick);
        else mRefEditor->setText(mRefEditor->text() + ", " + CurData->mNick);
        return;
    }

    // 나에게 접속한 다른 서버들에게 전달
    for(auto iSocket : mChainSockets)
    {
        if(iSocket != sock)
            iSocket->write(bytes);
    }

    // 내가 접속한 서버에게도 전달
    if(&mMyConnector != sock)
    if(mMyConnector.state() == QAbstractSocket::ConnectedState)
        mMyConnector.write(bytes);

    // 내 서버에 이관
    mRefServer->RelayMessage(bytes);
    mLogWidget->addItem(KOREAN("체인데이터 수신"));
}
