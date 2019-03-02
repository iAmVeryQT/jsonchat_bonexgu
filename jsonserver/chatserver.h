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
    void SendMessageForAdmin(QTcpSocket* peer, QString name);
    void SendMessageForBlack(QTcpSocket* peer, QString name);

    // 로그관련
    void SetWidgets(QListWidget* room, QListWidget* peer, QListWidget* log);
    void AddLog(QString text);
    void UpdateRoom(QString roomname, int peercount);
    void RemoveRoom(QString roomname);
    void UpdatePeer(QString peerid, QString username);
    void RemovePeer(QString peerid);

    // 제어관련
    void ClosePeer(QTcpSocket* peer);
    void ExitRoom(QString roomname, int peerid);
    void AddAdmin(QString username);
    void AddBlack(QString username);
    void ClearAdminAndBlack();

private:
    QListWidget* mRoomWidget;
    QListWidget* mPeerWidget;
    QListWidget* mLogWidget;

private:
    QString mStringDump;
    QMap<QString, RoomData*> mRoomPool;
    QMap<QString, bool> mSpecialPool;
};

#endif // CHATSERVER_H
