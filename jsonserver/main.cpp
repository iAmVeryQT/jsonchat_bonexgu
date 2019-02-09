#include "trayicon.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    trayicon w;
    QIcon icon("../assets/icon/server.png");
    w.setIcon(icon);
    w.show();

    return a.exec();
}
