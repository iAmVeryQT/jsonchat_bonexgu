#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QListWidget>

class PeerData : public QObjectUserData
{
public:
    PeerData() : mID(MakeID()) {}
    ~PeerData() {}

private:
    static int MakeID()
    {
        static int lastid = -1;
        return ++lastid;
    }

public:
    const int mID;
    QString mLastUserName;
    QString mLastRoomName;
};

class RoomData
{
public:
    QMap<int, QTcpSocket*> mPeers;
};

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    ChatServer();

private slots:
    void acceptPeer();
    void readyPeer();
    void errorPeer(QAbstractSocket::SocketError);

public:
    void SetWidgets(QListWidget* room, QListWidget* peer, QListWidget* log);
    void UpdateRoom(QString roomname, int peercount);
    void RemoveRoom(QString roomname);
    void UpdatePeer(QString peerid, QString username);
    void RemovePeer(QString peerid);
    void AddLog(QString text);
    void ExitRoom(QString roomname, int peerid);

private:
    QListWidget* mRoomWidget;
    QListWidget* mPeerWidget;
    QListWidget* mLogWidget;
    QMap<QString, RoomData*> mRoomPool;
};

#endif // CHATSERVER_H
