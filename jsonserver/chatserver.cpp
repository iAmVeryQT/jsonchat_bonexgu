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
        mStringDump += NewPacket;

        int BeginPos = 0, EndPos = 0;
        while((EndPos = mStringDump.indexOf("#json end")) != -1)
        {
            BeginPos = mStringDump.indexOf("#json begin");
            if(BeginPos != -1 && BeginPos < EndPos)
            {
                BeginPos += 11;
                auto NewJson = QJsonDocument::fromJson(
                    mStringDump.mid(BeginPos, EndPos - BeginPos).toUtf8());

                auto UserName = NewJson["name"].toString("noname");
                if(0 < UserName.length())
                {
                    // 최초 한번 관리자/블랙리스트인지 확인
                    bool IsBlack = false;
                    if(CurData->mLastUserName.length() == 0)
                    {
                        if(mSpecialPool.contains(UserName))
                        {
                            auto Value = mSpecialPool[UserName];
                            if(Value)
                            {
                                SendMessageForAdmin(CurPeer, UserName);
                            }
                            else
                            {
                                SendMessageForBlack(CurPeer, UserName);
                                IsBlack = true;
                            }
                        }
                    }

                    if(!IsBlack)
                    {
                        // 자신의 이름이 달라졌을 경우
                        if(CurData->mLastUserName != UserName)
                        {
                            CurData->mLastUserName = UserName;
                            UpdatePeer("ID_" + QString::number(CurData->mID), UserName);
                        }

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

                        auto Talk = NewJson["text"].toString("");
                        if(0 < Talk.length())
                        {
                            // 귓속말처리
                            auto Whisper = NewJson["to"].toString("");
                            if(0 < Whisper.length())
                            {
                                for(auto CurRoomPeer : CurRoom->mPeers)
                                {
                                    auto CurRoomPeerData = (PeerData*) CurRoomPeer->userData(0);
                                    if(CurRoomPeerData->mLastUserName == Whisper)
                                    {
                                        CurRoomPeer->write(NewPacket);
                                        AddLog(KOREAN("귓속말 : ID_") + QString::number(CurData->mID)
                                            + KOREAN(" → ID_") + QString::number(CurRoomPeerData->mID));
                                    }
                                }
                            }
                            // 해당 룸에 존재하는 모든 피어에게 회람
                            else for(auto CurRoomPeer : CurRoom->mPeers)
                                CurRoomPeer->write(NewPacket);
                        }
                    }
                }
            }
            mStringDump.remove(0, EndPos + 9);
        }
    }
}

void ChatServer::errorPeer(QAbstractSocket::SocketError)
{
    QTcpSocket* CurPeer = (QTcpSocket*) sender();
    ClosePeer(CurPeer);
}

void ChatServer::SendMessageForAdmin(QTcpSocket* peer, QString name)
{
    QString Msg = "#json begin {";
    Msg += "\"type\":\"chat\",";
    Msg += "\"room\":\"-server-\",";
    Msg += KOREAN("\"name\":\"서버메시지\",");
    Msg += "\"text\":\"" + name + KOREAN("님은 관리자입니다.\"");
    Msg += "} #json end";
    peer->write(Msg.toUtf8().constData());
}

void ChatServer::SendMessageForBlack(QTcpSocket* peer, QString name)
{
    QString Msg = "#json begin {";
    Msg += "\"type\":\"chat\",";
    Msg += "\"room\":\"-server-\",";
    Msg += KOREAN("\"name\":\"서버메시지\",");
    Msg += "\"text\":\"" + name + KOREAN("님은 블랙리스트입니다. <강제종료>\"");
    Msg += "} #json end";
    peer->write(Msg.toUtf8().constData());
    ClosePeer(peer);
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
    {
        CurPeer->setText(peerid + "(" + username + ")");
        CurPeer->setData(1, username);
    }
}

void ChatServer::RemovePeer(QString peerid)
{
    auto PeerList = mPeerWidget->findItems(peerid, Qt::MatchStartsWith);
    for(auto CurPeer : PeerList)
        delete mPeerWidget->takeItem(mPeerWidget->row(CurPeer));
}

void ChatServer::ClosePeer(QTcpSocket* peer)
{
    auto CurData = (PeerData*) peer->userData(0);

    if(0 < CurData->mLastRoomName.length())
        ExitRoom(CurData->mLastRoomName, CurData->mID);

    RemovePeer("ID_" + QString::number(CurData->mID));
    AddLog(KOREAN("서버퇴장 : ID_") + QString::number(CurData->mID));
    peer->close();
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

void ChatServer::AddAdmin(QString username)
{
    if(!mSpecialPool.contains(username))
    {
        mSpecialPool.insert(username, true);
        AddLog(KOREAN("관리자추가 : ") + username);

        QTcpSocket* FindPeer = nullptr;
        for(auto CurRoom : mRoomPool)
        for(auto CurRoomPeer : CurRoom->mPeers)
        {
            auto CurRoomPeerData = (PeerData*) CurRoomPeer->userData(0);
            if(CurRoomPeerData->mLastUserName == username)
                FindPeer = CurRoomPeer;
        }
        if(FindPeer)
            SendMessageForAdmin(FindPeer, username);
    }
}

void ChatServer::AddBlack(QString username)
{
    if(!mSpecialPool.contains(username))
    {
        mSpecialPool.insert(username, false);
        AddLog(KOREAN("블랙리스트추가 : ") + username);

        QTcpSocket* FindPeer = nullptr;
        for(auto CurRoom : mRoomPool)
        for(auto CurRoomPeer : CurRoom->mPeers)
        {
            auto CurRoomPeerData = (PeerData*) CurRoomPeer->userData(0);
            if(CurRoomPeerData->mLastUserName == username)
                FindPeer = CurRoomPeer;
        }
        if(FindPeer)
            SendMessageForBlack(FindPeer, username);
    }
}

void ChatServer::ClearAdminAndBlack()
{
    mSpecialPool.clear();
    AddLog(KOREAN("관리자/블랙리스트의 초기화"));
}
