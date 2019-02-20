#include "chatserver.h"
#include <QDebug>
#include <QJsonDocument>

#define KOREAN(STR) QString::fromWCharArray(L##STR)

ChatServer::ChatServer()
{
    mRefWidget = nullptr;
    connect(this, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

void ChatServer::acceptPeer()
{
    QTcpSocket* NewPeer = nextPendingConnection();
    auto NewPeerData = new PeerData();
    NewPeer->setUserData(0, NewPeerData);

    AddLog(KOREAN("손님이 서버에 입장하였습니다 : ") + QString::number(NewPeerData->mID));

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
                // 룸이름 얻기
                auto NewJson = QJsonDocument::fromJson(
                    OneString.mid(BeginPos, EndPos - BeginPos).toUtf8());
                auto RoomName = NewJson["room"].toString("global");

                // 자신의 룸이름이 달라졌을 경우
                if(CurData->mRoomName != RoomName)
                {
                    // 이전 룸 이름이 존재한다면 방을 바꾸는 것이다!
                    if(0 < CurData->mRoomName.length())
                        ExitRoom(CurData->mRoomName, CurData->mID);
                    CurData->mRoomName = RoomName;
                }

                // 룸찾기, 없으면 처음으로 만든다
                if(!mRoomPool.contains(RoomName))
                {
                    mRoomPool.insert(RoomName, new RoomData);
                    AddLog(KOREAN("룸이 생성되었습니다 : ") + RoomName);
                }
                auto CurRoom = mRoomPool.value(RoomName);

                // 내가 해당 룸에 없으면 추가함
                if(!CurRoom->mPeers.contains(CurData->mID))
                {
                    CurRoom->mPeers.insert(CurData->mID, CurPeer);
                    AddLog(KOREAN("룸에 회원이 입장하였습니다 : ")
                        + QString::number(CurData->mID) + " >> " + RoomName);
                }

                // 해당 룸에 존재하는 모든 피어에게 회람
                for(auto CurRoomPeer : CurRoom->mPeers)
                    CurRoomPeer->write(NewPacket);
            }
        }
    }
}

void ChatServer::errorPeer(QAbstractSocket::SocketError error)
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    auto CurData = (PeerData*) CurPeer->userData(0);

    if(0 < CurData->mRoomName.length())
        ExitRoom(CurData->mRoomName, CurData->mID);

    AddLog(KOREAN("손님이 서버에서 퇴장하였습니다 : ") + QString::number(CurData->mID));
}

void ChatServer::SetLogWidget(QListWidget* widget)
{
    mRefWidget = widget;
}

void ChatServer::AddLog(QString text)
{
    mRefWidget->addItem(text);
    mRefWidget->scrollToBottom();
}

void ChatServer::ExitRoom(QString roomname, int peerid)
{
    if(mRoomPool.contains(roomname))
    {
        AddLog(KOREAN("룸에서 회원이 퇴장하였습니다 : ") + QString::number(peerid) + " << " + roomname);

        auto CurRoom = mRoomPool.value(roomname);
        CurRoom->mPeers.remove(peerid);
        if(CurRoom->mPeers.count() == 0)
        {
            mRoomPool.remove(roomname);
            AddLog(KOREAN("룸이 파괴되었습니다 : ") + roomname);
        }
    }
}
