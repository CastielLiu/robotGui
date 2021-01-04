#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    g_appDir = a.applicationDirPath();
    g_cmdDir = g_appDir + "/../command/";
    MainWindow w;
    w.show();

    return a.exec();
}
