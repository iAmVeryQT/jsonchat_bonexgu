#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class PeerData : public QObjectUserData
{
public:
    PeerData() : mID(MakeID())
    {
        mRoomID = -1;
    }

    virtual ~PeerData()
    {
    }

private:
    static int MakeID()
    {
        static int globalid = -1;
        return ++globalid;
    }

public:
    const int mID;
    int mRoomID;
};

class RoomData
{
public:
    RoomData() : mID(MakeID())
    {
    }

    virtual ~RoomData()
    {
    }

private:
    static int MakeID()
    {
        static int globalid = -1;
        return ++globalid;
    }

public:
    const int mID;
    QList<int> mPeerID;
};

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    ChatServer();

private slots:
    void acceptPeer();
    void readyPeer();
    void errorPeer(QAbstractSocket::SocketError error);

private:
    QMap<int, QTcpSocket*> mPeerMap;
    QMap<int, RoomData> mRoomMap;
};

#endif // CHATSERVER_H
