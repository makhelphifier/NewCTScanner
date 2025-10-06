
#include "scancontroller.h"
#include "core/configmanager.h"
#include "core/reconstructioncontroller.h"
#include "core/dataacquisitionservice.h"
#include "core/hardwareservice.h"
#include "hal/IMotionStage.h"
#include <QTimer>
#include <QDebug>

ScanController::ScanController(HardwareService* hardwareService,
                               DataAcquisitionService* dataAcquisitionService,
                               ConfigManager* configManager,
                               ReconstructionController* reconController)
    : QObject(nullptr),
    m_hardwareService(hardwareService),
    m_dataAcquisitionService(dataAcquisitionService),
    m_configManager(configManager),
    m_reconController(reconController),
    m_currentFrame(0),
    m_totalFrames(0)
{
    setupStateMachine();
    connect(m_configManager, &ConfigManager::configLoaded, this, &ScanController::configurationLoaded);

    connect(m_hardwareService->motionStage(), &IMotionStage::moveFinished, this, &ScanController::onMoveFinished);
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
    // In new design, stopping the service is sufficient to break the loop
    m_dataAcquisitionService->stop();
    emit stopRequested();
}

void ScanController::onScanning()
{
    emit statusUpdated("Scanning... X-Ray is ON");
    emit stateChanged(StateScanning);
    Log("State machine entered Scanning state. ScanController is now in charge.");

    m_currentFrame = 0;
    m_totalFrames = m_currentParams.frameCount;

    m_dataAcquisitionService->start(m_currentParams, m_saveDirectory, m_savePrefix);

    startNextScanStep();
}

void ScanController::startNextScanStep()
{
    if (m_currentFrame >= m_totalFrames) {
        Log("ScanController: All frames acquired. Completing scan.");
        m_dataAcquisitionService->stop();
        emit scanCompleted();
        emit reconstructionStarted();

        QMetaObject::invokeMethod(m_reconController, "startReconstruction", Qt::QueuedConnection,
                                  Q_ARG(QString, m_saveDirectory));
        return;
    }

    Log(QString("ScanController: Starting step %1 of %2.").arg(m_currentFrame + 1).arg(m_totalFrames));
    emit scanProgress(m_currentFrame + 1, m_totalFrames);

    double nextAngle = (360.0 / m_totalFrames) * m_currentFrame;
    m_hardwareService->moveTo(nextAngle);
}

void ScanController::onMoveFinished(bool success)
{
    if (!success) {
        emit errorOccurred("Motion stage failed to move.");
        emit forceErrorState();
        return;
    }

    QMetaObject::invokeMethod(m_dataAcquisitionService, "acquireSingleFrame", Qt::QueuedConnection);
}

void ScanController::onRawFrameReady(FramePtr frame)
{
    Log(QString("ScanController: Raw frame %1 received, injecting metadata.").arg(frame->frameNumber));

    SystemStatus currentStatus = m_hardwareService->getSystemStatus();

    frame->angle = currentStatus.motionStageStatus.currentPosition;
    frame->xrayVoltage_kV = currentStatus.xrayStatus.currentVoltage_kV;
    frame->xrayCurrent_uA = currentStatus.xrayStatus.currentCurrent_uA;

    emit newProjectionImage(frame->image);

    m_currentFrame++;

    startNextScanStep();
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

    m_scanningState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_preparingState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_errorState->addTransition(this, &ScanController::stopRequested, m_idleState);

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
    QTimer::singleShot(1000, this, [this](){
        emit preparationFinished();
    });
    Log("State machine entered Preparing state.");
}

void ScanController::onError()
{
    emit statusUpdated("Error! Please reset.");
    emit stateChanged(StateError);
    m_dataAcquisitionService->stop();
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
