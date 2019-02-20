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

    mPeerWidget->addItem("ID_" + QString::number(NewData->mID));
    mPeerWidget->sortItems();
    AddLog(KOREAN("서버입장 : ID_") + QString::number(NewData->mID));

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
        QString OneString = NewPacket;

        int EndPos = OneString.indexOf("#json end");
        if(EndPos != -1)
        {
            int BeginPos = OneString.indexOf("#json begin");
            if(BeginPos != -1)
            {
                BeginPos += 11;
                auto NewJson = QJsonDocument::fromJson(
                    OneString.mid(BeginPos, EndPos - BeginPos).toUtf8());

                // 자신의 이름이 달라졌을 경우
                auto UserName = NewJson["name"].toString("noname");
                if(CurData->mLastUserName != UserName)
                {
                    CurData->mLastUserName = UserName;
                    UpdatePeer("ID_" + QString::number(CurData->mID), UserName);
                }

                // 자신의 룸이름이 달라졌을 경우
                auto RoomName = NewJson["room"].toString("global");
                if(CurData->mLastRoomName != RoomName)
                {
                    // 이전 룸 이름이 존재한다면 방을 바꾸는 것이다!
                    if(0 < CurData->mLastRoomName.length())
                        ExitRoom(CurData->mLastRoomName, CurData->mID);
                    CurData->mLastRoomName = RoomName;
                }

                // 룸찾기, 없으면 처음으로 만든다
                if(!mRoomPool.contains(RoomName))
                {
                    mRoomPool.insert(RoomName, new RoomData);
                    mRoomWidget->addItem(RoomName);
                    mRoomWidget->sortItems();
                }
                auto CurRoom = mRoomPool.value(RoomName);

                // 내가 해당 룸에 없으면 추가함
                if(!CurRoom->mPeers.contains(CurData->mID))
                {
                    CurRoom->mPeers.insert(CurData->mID, CurPeer);
                    UpdateRoom(RoomName, CurRoom->mPeers.count());
                    AddLog("[" + RoomName + KOREAN("]입장 : ID_") + QString::number(CurData->mID));
                }

                // 해당 룸에 존재하는 모든 피어에게 회람
                for(auto CurRoomPeer : CurRoom->mPeers)
                    CurRoomPeer->write(NewPacket);
            }
        }
    }
}

void ChatServer::errorPeer(QAbstractSocket::SocketError)
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    auto CurData = (PeerData*) CurPeer->userData(0);

    if(0 < CurData->mLastRoomName.length())
        ExitRoom(CurData->mLastRoomName, CurData->mID);

    RemovePeer("ID_" + QString::number(CurData->mID));
    AddLog(KOREAN("서버퇴장 : ID_") + QString::number(CurData->mID));
}

void ChatServer::SetWidgets(QListWidget* room, QListWidget* peer, QListWidget* log)
{
    mRoomWidget = room;
    mPeerWidget = peer;
    mLogWidget = log;
}

void ChatServer::UpdateRoom(QString roomname, int peercount)
{
    auto RoomList = mRoomWidget->findItems(roomname, Qt::MatchStartsWith);
    for(auto CurRoom : RoomList)
        CurRoom->setText(roomname + "(" + QString::number(peercount) + "P)");
}

void ChatServer::RemoveRoom(QString roomname)
{
    auto RoomList = mRoomWidget->findItems(roomname, Qt::MatchStartsWith);
    for(auto CurRoom : RoomList)
        delete mRoomWidget->takeItem(mRoomWidget->row(CurRoom));
}

void ChatServer::UpdatePeer(QString peerid, QString username)
{
    auto PeerList = mPeerWidget->findItems(peerid, Qt::MatchStartsWith);
    for(auto CurPeer : PeerList)
        CurPeer->setText(peerid + "(" + username + ")");
}

void ChatServer::RemovePeer(QString peerid)
{
    auto PeerList = mPeerWidget->findItems(peerid, Qt::MatchStartsWith);
    for(auto CurPeer : PeerList)
        delete mPeerWidget->takeItem(mPeerWidget->row(CurPeer));
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
        {
            mRoomPool.remove(roomname);
            RemoveRoom(roomname);
        }
        else UpdateRoom(roomname, CurRoom->mPeers.count());
        AddLog("[" + roomname + KOREAN("]퇴장 : ID_") + QString::number(peerid));
    }
}
