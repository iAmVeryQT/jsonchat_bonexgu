#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    ChatServer();

private slots:
    void acceptPeer();
};

#endif // CHATSERVER_H
