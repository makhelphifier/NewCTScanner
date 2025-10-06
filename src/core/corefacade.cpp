// src/core/corefacade.cpp

#include "corefacade.h"
#include "hardwareservice.h"
#include "scancontroller.h"
#include "common/Logger.h"
#include <QThread>
#include "core/dataacquisitionservice.h"
#include "configmanager.h"
#include "reconstructioncontroller.h"
#include "systemsafetyservice.h"

CoreFacade::CoreFacade(QObject *parent) : QObject(parent)
{
    Log("CoreFacade created.");
    m_hardwareService.reset(new HardwareService());
    m_dataAcquisitionService.reset(new DataAcquisitionService(m_hardwareService.data()));
    m_configManager.reset(new ConfigManager());
    m_reconController.reset(new ReconstructionController());
    m_safetyService.reset(new SystemSafetyService());

    m_scanController.reset(new ScanController(
        m_hardwareService.data(),
        m_dataAcquisitionService.data(),
        m_configManager.data(),
        m_reconController.data(),
        m_safetyService.data() // <-- 3. 传递给 ScanController

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


    connect(m_dataAcquisitionService.data(), &DataAcquisitionService::newRawFrameReady,
            m_scanController.data(), &ScanController::onRawFrameReady);

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
SystemSafetyService* CoreFacade::safetyService() const
{
    return m_safetyService.data();
}

