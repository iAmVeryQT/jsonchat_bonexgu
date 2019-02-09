#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatserver.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void ServerOn();
    void ServerOff();

private:
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

private:
    Ui::MainWindow *ui;
    ChatServer mServer;
};

#endif // MAINWINDOW_H
