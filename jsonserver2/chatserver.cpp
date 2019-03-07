#include "chatserver.h"
#include <QDebug>
#include <QJsonDocument>

#define KOREAN(STR) QString::fromWCharArray(L##STR)

ChatServer::ChatServer()
{
    mRoomWidget = nullptr;
    mPeerWidget = nullptr;
    mLogWidget = nullptr;
    connect(this, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

void ChatServer::acceptPeer()
{
    QTcpSocket* NewPeer = nextPendingConnection();
    auto NewData = new PeerData();
    NewPeer->setUserData(0, NewData);

    QString PeerID = "ID_" + QString::number(NewData->mID);
    mPeerWidget->addItem(PeerID);
    mPeerWidget->sortItems();
    AddLog(KOREAN("서버입장 : ") + PeerID);

    connect(NewPeer, SIGNAL(readyRead()), this, SLOT(readyPeer()));
    connect(NewPeer, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(errorPeer(QAbstractSocket::SocketError)));
}

void ChatServer::readyPeer()
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    auto CurData = (PeerData*) CurPeer->userData(0);
    auto PacketSize = CurPeer->bytesAvailable();

    if(0 < PacketSize)
    {
        QByteArray NewPacket = CurPeer->read(PacketSize);
        QString OnString = NewPacket;

        int BeginPos = 0, EndPos = 0;
        if((EndPos = OnString.indexOf("#json end")) != -1)
        {
            BeginPos = OnString.indexOf("#json begin");
            if(BeginPos != -1 && BeginPos < EndPos)
            {
                BeginPos += 11;
                auto NewJson = QJsonDocument::fromJson(
                    OnString.mid(BeginPos, EndPos - BeginPos).toUtf8());

                // 자신의 이름이 달라졌을 경우
                auto UserName = NewJson["name"].toString("noname");
                if(CurData->mLastUserName != UserName)
                    CurData->mLastUserName = UserName;

                // 자신의 룸이름이 달라졌을 경우
                auto RoomName = NewJson["room"].toString("global");
                if(CurData->mLastRoomName != RoomName)
                {
                    // 이전 룸이름이 존재한다면 방을 바꾸는 것이다!
                    if(0 < CurData->mLastRoomName.length())
                        ExitRoom(CurData->mLastRoomName, CurData->mID);
                    CurData->mLastRoomName = RoomName;
                }

                // 룸찾기, 없으면 처음으로 만든다
                if(!mRoomPool.contains(RoomName))
                    mRoomPool.insert(RoomName, new RoomData);
                auto CurRoom = mRoomPool.value(RoomName);

                // 내가 해당 룸에 없으면 추가함
                if(!CurRoom->mPeers.contains(CurData->mID))
                    CurRoom->mPeers.insert(CurData->mID, CurPeer);

                for(auto CurRoomPeer : CurRoom->mPeers)
                    CurRoomPeer->write(NewPacket);
            }
        }
    }
}

void ChatServer::errorPeer(QAbstractSocket::SocketError)
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    PeerData* CurPeerData = (PeerData*) CurPeer->userData(0);

    if(0 < CurPeerData->mLastRoomName.length())
        ExitRoom(CurPeerData->mLastRoomName, CurPeerData->mID);

    QString PeerID = "ID_" + QString::number(CurPeerData->mID);
    auto PeerList = mPeerWidget->findItems(PeerID, Qt::MatchStartsWith);
    for(auto CurPeer : PeerList)
        delete mPeerWidget->takeItem(mPeerWidget->row(CurPeer));

    AddLog(KOREAN("서버퇴장 : ") + PeerID);
}

void ChatServer::SetWidgets(QListWidget* room, QListWidget* peer, QListWidget* log)
{
    mRoomWidget = room;
    mPeerWidget = peer;
    mLogWidget = log;
}

void ChatServer::AddLog(QString text)
{
    mLogWidget->addItem(text);
    mLogWidget->scrollToBottom();
}

void ChatServer::ExitRoom(QString roomname, int peerid)
{
    if(mRoomPool.contains(roomname))
    {
        auto CurRoom = mRoomPool.value(roomname);
        CurRoom->mPeers.remove(peerid);
        if(CurRoom->mPeers.count() == 0)
            mRoomPool.remove(roomname);
    }
}
