#include "trayicon.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    trayicon w;
    QString ResPath = QCoreApplication::applicationDirPath() + "/../../assets/";
    QIcon icon(ResPath + "icon/server.png");
    w.setIcon(icon);
    w.show();

    return a.exec();
}
