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

private slots:
    void on_admin_clicked();

    void on_blacklist_clicked();

    void on_normal_clicked();

private:
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

private:
    Ui::MainWindow *ui;
    ChatServer mServer;
};

#endif // MAINWINDOW_H
