#include "ui/mainwindow.h"
#include "common/ScanParameters.h"
#include "core/corefacade.h"
#include <QApplication>
#include "common/HardwareStatus.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<ScanParameters>("ScanParameters");
    qRegisterMetaType<XRaySourceStatus>("XRaySourceStatus");
    qRegisterMetaType<MotionStageStatus>("MotionStageStatus");
    qRegisterMetaType<DetectorStatus>("DetectorStatus");
    qRegisterMetaType<SystemStatus>("SystemStatus");
    qRegisterMetaType<FramePtr>("FramePtr");

    QApplication a(argc, argv);

    CoreFacade::instance().init();

    MainWindow w;
    w.show();
    return a.exec();
}
