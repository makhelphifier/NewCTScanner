
#include "ui/mainwindow.h"
#include "common/ScanParameters.h"
#include "core/corefacade.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<ScanParameters>("ScanParameters");
    QApplication a(argc, argv);

    CoreFacade::instance().init();

    MainWindow w;
    w.show();
    return a.exec();
}
