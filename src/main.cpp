#include "ui/mainwindow.h"
#include "common/ScanParameters.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<ScanParameters>("ScanParameters");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
