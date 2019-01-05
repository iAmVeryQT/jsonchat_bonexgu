#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

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

private slots:
    void on_ConnectBtn_clicked();

    void on_AddressEdit_textChanged(const QString &arg1);

    void on_TalkEdit_textEdited(const QString &arg1);

    void on_TalkEdit_returnPressed();

    void readyPeer();

private:
    Ui::MainWindow *ui;
    QString mAddress;
    QTcpSocket mSocket;
    QString mUserText;
    QString mRecvText;
};

#endif // MAINWINDOW_H
