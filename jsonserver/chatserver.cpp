#include "chatserver.h"
#include <QDebug>

ChatServer::ChatServer()
{
    connect(this, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

void ChatServer::acceptPeer()
{
    auto NewPeer = nextPendingConnection();
    auto NewData = new PeerData();
    NewPeer->setUserData(0, NewData);
    mPeerMap.insert(NewData->mID, NewPeer);

    connect(NewPeer, SIGNAL(readyRead()), this, SLOT(readyPeer()));
    connect(NewPeer, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(errorPeer(QAbstractSocket::SocketError)));
}

void ChatServer::readyPeer()
{
    auto CurPeer = dynamic_cast<QTcpSocket*>(sender());
    auto CurData = dynamic_cast<PeerData*>(CurPeer->userData(0));
    const qint64 PacketSize = CurPeer->bytesAvailable();

    if(0 < PacketSize)
    {
        auto Packet = CurPeer->read(PacketSize);
        CurPeer->write(Packet);

        /*QString TestMessage = "#json begin {";
        TestMessage += "'type':'chat',";
        TestMessage += "'room':'abc',";
        TestMessage += "'name':'server',";
        TestMessage += "'text':'hello!'";
        TestMessage += "} #json end";*/
    }
}

void ChatServer::errorPeer(QAbstractSocket::SocketError error)
{
    auto CurPeer = dynamic_cast<QTcpSocket*>(sender());
    auto CurData = dynamic_cast<PeerData*>(CurPeer->userData(0));
    mPeerMap.remove(CurData->mID);
}
