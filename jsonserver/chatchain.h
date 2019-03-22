#ifndef CHATCHAIN_H
#define CHATCHAIN_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QLineEdit>
#include <QListWidget>

class ChainData : public QObjectUserData
{
public:
    ChainData(bool need_firsttime) : mID(MakeID()) {mFirstTime = need_firsttime;}
    ~ChainData() {}

private:
    static int MakeID()
    {
        static int lastid = -1;
        return ++lastid;
    }

public:
    const int mID;
    bool mFirstTime;
    QString mNick;
    QString mStringDump;
};

class ChatServer;

class ChatChain : public QTcpServer
{
    Q_OBJECT

public:
    ChatChain();
    ~ChatChain();

public:
    void BroadcastMessage(const QByteArray& bytes);

public:
    void SetServer(ChatServer* server);
    void Connect(QString address, QString nick);
    void SetChainEditor(QLineEdit* editor, QListWidget* log);

private slots:
    void acceptPeer();
    void readyPeer();
    void errorPeer(QAbstractSocket::SocketError);

private:
    void JsonParser(QTcpSocket* sock);
    void OnJson(QTcpSocket* sock, const QByteArray& bytes);

private:
    ChatServer* mRefServer;
    QMap<int, QTcpSocket*> mChainSockets;
    QTcpSocket mMyConnector;
    QLineEdit* mRefEditor;
    QListWidget* mLogWidget;
};

#endif // CHATCHAIN_H
