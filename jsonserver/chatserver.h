#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QListWidget>

class PeerData : public QObjectUserData
{
public:
    PeerData() : mID(MakeID())
    {
    }

private:
    static int MakeID()
    {
        static int lastid = -1;
        return ++lastid;
    }

public:
    const int mID;
    QString mRoomName;
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
    void errorPeer(QAbstractSocket::SocketError error);

public:
    void SetLogWidget(QListWidget* widget);
    void AddLog(QString text);
    void ExitRoom(QString roomname, int peerid);

private:
    QListWidget* mRefWidget;
    QMap<QString, RoomData*> mRoomPool;
};

#endif // CHATSERVER_H
