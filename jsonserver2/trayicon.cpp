#include "trayicon.h"
#include <QEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QDebug>

trayicon::trayicon()
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

trayicon::~trayicon()
{
}

void trayicon::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Context: // R버튼 클릭
    case QSystemTrayIcon::Trigger: // L버튼 클릭
    case QSystemTrayIcon::MiddleClick: // 휠버튼 클릭
        {
            const QPoint ClickPos = QCursor::pos();
            const QSize MenuSize = mMenu.size();
            const QRect GeometryRect = QGuiApplication::screenAt(ClickPos)->geometry();
            const int PosX = (GeometryRect.right() < ClickPos.x() + MenuSize.width())?
                ClickPos.x() - MenuSize.width() : ClickPos.x();
            const int PosY = (GeometryRect.bottom() < ClickPos.y() + MenuSize.height())?
                ClickPos.y() - MenuSize.height() : ClickPos.y();
            mMenu.move(PosX, PosY);
            mMenu.popup();
        }
        break;
    default:
        break;
    }
}
