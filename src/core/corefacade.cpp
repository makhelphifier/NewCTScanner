#include "corefacade.h"
#include "hardwareservice.h"
#include "scancontroller.h"
#include "common/Logger.h"
#include <QThread>
#include "core/dataacquisitionservice.h"
#include "configmanager.h"
#include "reconstructioncontroller.h"

// 替换现有的 CoreFacade::CoreFacade 构造函数
CoreFacade::CoreFacade(QObject *parent) : QObject(parent)
{
    Log("CoreFacade created.");
    // 必须先创建 HardwareService
    m_hardwareService.reset(new HardwareService());
    // 再创建需要 HardwareService 的服务
    m_dataAcquisitionService.reset(new DataAcquisitionService(m_hardwareService.data()));

    m_configManager.reset(new ConfigManager());
    m_reconController.reset(new ReconstructionController());

    // ScanController 现在依赖 DataAcquisitionService
    m_scanController.reset(new ScanController(
        m_dataAcquisitionService.data(),
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
    connect(m_dataAcquisitionService.data(), &DataAcquisitionService::scanProgress,
            m_scanController.data(), &ScanController::scanProgress);

    connect(m_dataAcquisitionService.data(), &DataAcquisitionService::newFrameReady,
            m_scanController.data(), [this](FramePtr frame){
                // 从FramePtr中提取QImage并转发
                emit m_scanController->newProjectionImage(frame->image);
            });

    connect(m_dataAcquisitionService.data(), &DataAcquisitionService::acquisitionFinished,
            m_scanController.data(), &ScanController::onAcquisitionFinished);
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
