#include "corefacade.h"
#include "hardwareservice.h"
#include "scancontroller.h"
#include "common/Logger.h"
#include <QThread>
#include "core/dataacquisitionservice.h"
#include "configmanager.h"
#include "reconstructioncontroller.h"


CoreFacade::CoreFacade(QObject *parent) : QObject(parent)
{
    Log("CoreFacade created.");
    m_hardwareService.reset(new HardwareService());
    m_dataAcquisitionService.reset(new DataAcquisitionService());
    m_configManager.reset(new ConfigManager());
    m_reconController.reset(new ReconstructionController());
    m_scanController.reset(new ScanController(
        m_hardwareService.data(),
        m_configManager.data(),
        m_reconController.data()
        ));
}
CoreFacade::~CoreFacade()
{
    Log("CoreFacade destroyed.");
}

CoreFacade& CoreFacade::instance()
{
    static CoreFacade self;
    return self;
}

void CoreFacade::init()
{
    Log("CoreFacade initializing...");
    m_hardwareService->init();
    connect(m_scanController.data(), &ScanController::commandStartSaving,
            m_dataAcquisitionService.data(), &DataAcquisitionService::startSaving);
    connect(m_scanController.data(), &ScanController::commandStopSaving,
            m_dataAcquisitionService.data(), &DataAcquisitionService::stopSaving);
    connect(m_scanController.data(), &ScanController::commandSaveImage,
            m_dataAcquisitionService.data(), &DataAcquisitionService::saveImage);

    connect(m_scanController.data(), &ScanController::reconstructionStarted,
            m_reconController.data(), [this](){
                QString path = m_scanController->getSaveDirectory();
                QMetaObject::invokeMethod(m_reconController.data(), "startReconstruction", Qt::QueuedConnection,
                                          Q_ARG(QString, path));
            });

    QThread* scanControllerThread = new QThread();
    m_scanController->moveToThread(scanControllerThread);
    connect(scanControllerThread, &QThread::started, m_scanController.data(), &ScanController::init);
    connect(scanControllerThread, &QThread::finished, m_scanController.data(), &QObject::deleteLater);
    connect(scanControllerThread, &QThread::finished, scanControllerThread, &QObject::deleteLater);
    scanControllerThread->start();

    Log("CoreFacade initialization complete.");
}

HardwareService* CoreFacade::hardwareService() const
{
    return m_hardwareService.data();
}

ScanController* CoreFacade::scanController() const
{
    return m_scanController.data();
}

DataAcquisitionService* CoreFacade::dataAcquisitionService() const
{
    return m_dataAcquisitionService.data();
}

ConfigManager* CoreFacade::configManager() const
{
    return m_configManager.data();
}

ReconstructionController* CoreFacade::reconstructionController() const
{
    return m_reconController.data();
}
