#ifndef TRAYICON_H
#define TRAYICON_H

#include "traymenu.h"
#include <QSystemTrayIcon>

class trayicon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    trayicon();
    ~trayicon() Q_DECL_OVERRIDE;

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    traymenu mMenu;
};

#endif // TRAYICON_H
