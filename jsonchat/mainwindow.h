﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>

#include "emojipopup.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void OnMessage(const char* text);
    void SendMessage(QString talk);

private slots:
    void on_ConnectBtn_clicked();

    void on_AddressEdit_textChanged(const QString &arg1);

    void on_RoomName_textEdited(const QString &arg1);

    void on_UserName_textEdited(const QString &arg1);

    void on_TalkEdit_textEdited(const QString &arg1);

    void on_TalkEdit_returnPressed();

    void readyPeer();

    void on_FileBtn_pressed();

    void on_DownloadBtn_pressed();

    void on_EmojiButton_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket mSocket;
    QString mAddress;
    QString mRoomName;
    QString mUserName;
    QString mUserText;
    QString mRecvText;
    QList<QFile*> mFileWorks;
    EmojiPopup mEmoji;
};

#endif // MAINWINDOW_H
