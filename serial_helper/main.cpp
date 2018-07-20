#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/icon/blacktea.jpg"));
    w.setWindowTitle(QString(("串口调试")));
    w.show();

    return a.exec();
}
