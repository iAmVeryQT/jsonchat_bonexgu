#include "chatserver.h"
#include <QDebug>

ChatServer::ChatServer()
{
    connect(this, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

void ChatServer::acceptPeer()
{
    QString TestMessage = "#json begin {";
    TestMessage += "'type':'chat',";
    TestMessage += "'room':'abc',";
    TestMessage += "'name':'server',";
    TestMessage += "'text':'hello!'";
    TestMessage += "} #json end";

    QTcpSocket* NewPeer = nextPendingConnection();
    NewPeer->write(TestMessage.toUtf8());
    qDebug() << TestMessage;
}
