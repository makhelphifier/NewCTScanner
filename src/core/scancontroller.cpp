#include "scancontroller.h"
#include "core/configmanager.h"
#include "core/reconstructioncontroller.h"
#include "core/dataacquisitionservice.h"
#include <QTimer>
#include <QDebug>

ScanController::ScanController(DataAcquisitionService* dataAcquisitionService,
                               ConfigManager* configManager,
                               ReconstructionController* reconController)
    : QObject(nullptr),
    m_dataAcquisitionService(dataAcquisitionService),
    m_configManager(configManager),
    m_reconController(reconController)
{
    setupStateMachine();
    connect(m_configManager, &ConfigManager::configLoaded, this, &ScanController::configurationLoaded);
}

ScanController::~ScanController() {}

void ScanController::init() {
    QTimer::singleShot(0, m_stateMachine, &QStateMachine::start);
    Log("ScanController initialized.");
}

void ScanController::requestScan()
{
    emit scanRequested();
    Log("UI requested to start scan.");
}

void ScanController::requestStop()
{
    Log("UI requested to stop scan.");
    m_dataAcquisitionService->stop();
    emit stopRequested();
}

void ScanController::setupStateMachine()
{
    m_stateMachine = new QStateMachine(this);

    m_idleState = new QState(m_stateMachine);
    m_preparingState = new QState(m_stateMachine);
    m_scanningState = new QState(m_stateMachine);
    m_errorState = new QState(m_stateMachine);

    connect(m_errorState, &QState::entered, this, &ScanController::onError);
    connect(m_idleState, &QState::entered, this, &ScanController::onIdle);
    connect(m_preparingState, &QState::entered, this, &ScanController::onPreparing);
    connect(m_scanningState, &QState::entered, this, &ScanController::onScanning);

    m_idleState->addTransition(this, &ScanController::scanRequested, m_preparingState);
    m_preparingState->addTransition(this, &ScanController::preparationFinished, m_scanningState);
    m_scanningState->addTransition(this, &ScanController::scanCompleted, m_idleState);

    // Stop/Reset transitions
    m_scanningState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_preparingState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_errorState->addTransition(this, &ScanController::stopRequested, m_idleState);

    // Error transitions
    m_idleState->addTransition(this, &ScanController::forceErrorState, m_errorState);
    m_preparingState->addTransition(this, &ScanController::forceErrorState, m_errorState);
    m_scanningState->addTransition(this, &ScanController::forceErrorState, m_errorState);


    m_stateMachine->setInitialState(m_idleState);
}

void ScanController::onIdle()
{
    emit statusUpdated("Idle");
    emit stateChanged(StateIdle);
    Log("State machine entered Idle state.");
}

void ScanController::onPreparing()
{
    emit statusUpdated("Preparing...");
    emit stateChanged(StatePreparing);
    // 可以在此做一些硬件预热等操作
    QTimer::singleShot(1000, this, [this](){ // 模拟准备时间
        emit preparationFinished();
    });
    Log("State machine entered Preparing state.");
}

void ScanController::onScanning()
{
    emit statusUpdated("Scanning... X-Ray is ON");
    emit stateChanged(StateScanning);

    Log("State machine entered Scanning state. Delegating to DataAcquisitionService.");
    // 将采集任务全权委托给DataAcquisitionService
    m_dataAcquisitionService->start(m_currentParams, m_saveDirectory, m_savePrefix);
}

void ScanController::onError()
{
    emit statusUpdated("Error! Please reset.");
    emit stateChanged(StateError);
    m_dataAcquisitionService->stop();
}

void ScanController::onAcquisitionFinished()
{
    Log("ScanController: Notified that acquisition is finished. Completing scan.");
    emit scanCompleted();
    emit reconstructionStarted();

    QMetaObject::invokeMethod(m_reconController, "startReconstruction", Qt::QueuedConnection,
                              Q_ARG(QString, m_saveDirectory));
}

void ScanController::updateParameters(const ScanParameters &params)
{
    m_currentParams = params;
    Log(QString("Scan parameters updated: kV=%1, uA=%2, Frames=%3").arg(params.voltage).arg(params.current).arg(params.frameCount));
}

void ScanController::updateSavePath(const QString &directory, const QString &prefix)
{
    m_saveDirectory = directory;
    m_savePrefix = prefix;
    Log(QString("Save path updated. Dir: %1, Prefix: %2").arg(directory).arg(prefix));
}

QString ScanController::getSaveDirectory() const
{
    return m_saveDirectory;
}

void ScanController::saveConfiguration(const QString &filePath)
{
    m_configManager->saveConfig(m_currentParams, filePath);
}

void ScanController::loadConfiguration(const QString &filePath)
{
    m_configManager->loadConfig(filePath);
}
