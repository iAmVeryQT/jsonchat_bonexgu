#ifndef TRAYMENU_H
#define TRAYMENU_H

#include "mainwindow.h"
#include <QWidget>
#include <QEventLoop>

namespace Ui {
class traymenu;
}

class traymenu : public QWidget
{
    Q_OBJECT

public:
    explicit traymenu(QWidget *parent = nullptr);
    ~traymenu();

public:
    void popup();

private:
    void focusOutEvent(QFocusEvent*) Q_DECL_OVERRIDE;

private slots:
    void on_exit_clicked();

    void on_gui_clicked();

    void on_onoff_clicked();

private:
    Ui::traymenu *ui;
    MainWindow mWindow;
    bool mServerOn;
};

#endif // TRAYMENU_H
