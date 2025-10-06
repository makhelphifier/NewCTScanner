
#include "scancontroller.h"
#include "core/configmanager.h"
#include "core/reconstructioncontroller.h"
#include "core/dataacquisitionservice.h"
#include "core/hardwareservice.h"
#include "hal/IMotionStage.h"
#include <QTimer>
#include <QDebug>
#include <cmath>
#include "core/systemsafetyservice.h"

ScanController::ScanController(HardwareService* hardwareService,
                               DataAcquisitionService* dataAcquisitionService,
                               ConfigManager* configManager,
                               ReconstructionController* reconController,
                               SystemSafetyService* safetyService)
    : QObject(nullptr),
    m_hardwareService(hardwareService),
    m_dataAcquisitionService(dataAcquisitionService),
    m_configManager(configManager),
    m_reconController(reconController),
    m_safetyService(safetyService),
    m_currentFrame(0),
    m_totalFrames(0)
{
    setupStateMachine();
    connect(m_configManager, &ConfigManager::configLoaded, this, &ScanController::configurationLoaded);
    connect(m_safetyService, &SystemSafetyService::safetyInterlockTriggered,
            this, &ScanController::onSafetyInterlockTriggered);
}

ScanController::~ScanController() {}

void ScanController::init() {
    connect(m_hardwareService->motionStage(), &IMotionStage::moveFinished, this, &ScanController::onMoveFinished);
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

void ScanController::onScanning()
{
    if (m_stateMachine->configuration().contains(m_pausedState) == false) {
        emit statusUpdated("Scanning... X-Ray is ON");
        emit stateChanged(StateScanning);
        Log("State machine entered Scanning state. ScanController is now in charge.");

        m_currentFrame = 0;
        m_totalFrames = m_currentParams.frameCount;
        QMetaObject::invokeMethod(m_dataAcquisitionService, "start", Qt::QueuedConnection,
                                  Q_ARG(ScanParameters, m_currentParams),
                                  Q_ARG(QString, m_saveDirectory),
                                  Q_ARG(QString, m_savePrefix));
    } else {
        Log("State machine resumed Scanning state.");
        emit statusUpdated("Scanning... (Resumed)");
        emit stateChanged(StateScanning);
    }

    m_scanTimer.start();
    m_scanProgress = ScanProgress();

    startNextScanStep();
}

void ScanController::onPaused()
{
    emit statusUpdated("Paused");
    emit stateChanged(StatePaused);
    Log("State machine entered Paused state.");
}


void ScanController::startNextScanStep()
{

    if (m_stateMachine->configuration().contains(m_scanningState) == false) {
        Log("ScanController: startNextScanStep called but not in Scanning state. Ignoring.");
        return;
    }

    if (m_currentFrame >= m_totalFrames) {
        Log("ScanController: All frames acquired. Completing scan.");
        m_dataAcquisitionService->stop();
        emit scanCompleted();
        emit reconstructionStarted();

        QMetaObject::invokeMethod(m_reconController, "startReconstruction", Qt::QueuedConnection,
                                  Q_ARG(QString, m_saveDirectory));
        return;
    }

    double totalAngleSpan = m_currentParams.endAngle - m_currentParams.startAngle;
    double angleStep = (m_totalFrames > 1) ? (totalAngleSpan / (m_totalFrames - 1)) : 0.0;
    double nextAngle = m_currentParams.startAngle + (m_currentFrame * angleStep);

    nextAngle = fmod(nextAngle, 360.0);
    if (nextAngle < 0) {
        nextAngle += 360.0;
    }
    m_hardwareService->moveTo(nextAngle);
}




void ScanController::setupStateMachine()
{
    m_stateMachine = new QStateMachine(this);

    m_idleState = new QState(m_stateMachine);
    m_preparingState = new QState(m_stateMachine);
    m_scanningState = new QState(m_stateMachine);
    m_pausedState = new QState(m_stateMachine);
    m_errorState = new QState(m_stateMachine);

    connect(m_errorState, &QState::entered, this, &ScanController::onError);
    connect(m_idleState, &QState::entered, this, &ScanController::onIdle);
    connect(m_preparingState, &QState::entered, this, &ScanController::onPreparing);
    connect(m_scanningState, &QState::entered, this, &ScanController::onScanning);
    connect(m_pausedState, &QState::entered, this, &ScanController::onPaused);

    m_idleState->addTransition(this, &ScanController::scanRequested, m_preparingState);
    m_preparingState->addTransition(this, &ScanController::preparationFinished, m_scanningState);
    m_scanningState->addTransition(this, &ScanController::scanCompleted, m_idleState);

    m_scanningState->addTransition(this, &ScanController::pauseRequested, m_pausedState);
    m_pausedState->addTransition(this, &ScanController::resumeRequested, m_scanningState);


    m_scanningState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_preparingState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_pausedState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_errorState->addTransition(this, &ScanController::stopRequested, m_idleState);

    m_idleState->addTransition(this, &ScanController::forceErrorState, m_errorState);
    m_preparingState->addTransition(this, &ScanController::forceErrorState, m_errorState);
    m_scanningState->addTransition(this, &ScanController::forceErrorState, m_errorState);

    m_stateMachine->setInitialState(m_idleState);

    connect(this, &ScanController::resumeRequested, this, &ScanController::startNextScanStep);

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

    QString validationError;
    if (!validateParameters(validationError)) {
        emit errorOccurred(validationError); // 向UI发送具体的错误信息
        emit forceErrorState(); // 触发状态机进入错误状态
        return; // 中断准备过程
    }

    Log("ScanController: Setting hardware parameters for scan.");
    m_hardwareService->setVoltage(m_currentParams.voltage);
    m_hardwareService->setMotionSpeed(m_currentParams.rotationSpeed);

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
    m_hardwareService->turnOffXRay();
}

void ScanController::updateParameters(const ScanParameters &params)
{
    m_currentParams = params;
    Log(QString("Scan parameters updated: kV=%1, uA=%2, Frames=%3, Angle=%4°-%5°, Speed=%6°/s")
            .arg(params.voltage)
            .arg(params.current)
            .arg(params.frameCount)
            .arg(params.startAngle)
            .arg(params.endAngle)
            .arg(params.rotationSpeed));}

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


bool ScanController::validateParameters(QString &errorMessage)
{
    Log("Validating scan parameters...");

    if (m_currentParams.frameCount <= 1 && m_currentParams.startAngle != m_currentParams.endAngle) {
        errorMessage = "For a single frame, start and end angles must be the same.";
        Log("ERROR: " + errorMessage);
        return false;
    }

    if (m_currentParams.frameCount > 1) {
        double totalAngleSpan = std::abs(m_currentParams.endAngle - m_currentParams.startAngle);
        if (totalAngleSpan < 0.01 && m_currentParams.endAngle != m_currentParams.startAngle) {
        } else if (totalAngleSpan < 0.01) {
            errorMessage = "For multi-frame scan, start and end angles cannot be the same.";
            Log("ERROR: " + errorMessage);
            return false;
        }
    }


    if (m_currentParams.voltage < 20.0 || m_currentParams.voltage > 225.0) {
        errorMessage = QString("Voltage (%1 kV) is out of the valid range [20, 225] kV.").arg(m_currentParams.voltage);
        Log("ERROR: " + errorMessage);
        return false;
    }

    if (m_currentParams.rotationSpeed <= 0) {
        errorMessage = "Rotation speed must be positive.";
        Log("ERROR: " + errorMessage);
        return false;
    }

    Log("Scan parameters are valid.");
    return true;
}


void ScanController::updateScanProgress()
{
    if (m_currentFrame <= 0 || m_totalFrames == 0) return;

    m_scanProgress.currentProjection = m_currentFrame + 1;
    m_scanProgress.totalProjections = m_totalFrames;

    double totalAngleSpan = m_currentParams.endAngle - m_currentParams.startAngle;
    double angleStep = (m_totalFrames > 1) ? (totalAngleSpan / (m_totalFrames - 1)) : 0.0;
    m_scanProgress.currentAngle = m_currentParams.startAngle + (m_currentFrame * angleStep);

    m_scanProgress.percentComplete = static_cast<int>((static_cast<double>(m_scanProgress.currentProjection) / m_totalFrames) * 100.0);

    m_scanProgress.elapsedTime_ms = m_scanTimer.elapsed();
    if (m_currentFrame > 0) {
        qint64 timePerFrame_ms = m_scanProgress.elapsedTime_ms / (m_currentFrame + 1);
        m_scanProgress.estimatedTimeRemaining_ms = timePerFrame_ms * (m_totalFrames - (m_currentFrame + 1));
    }

    emit scanProgress(m_scanProgress);
}


void ScanController::onMoveFinished(bool success)
{
    if (!success) {
        emit errorOccurred("Motion stage failed to move.");
        emit forceErrorState();
        return;
    }

    Log("ScanController: Motion finished. Turning on X-Ray.");
    m_hardwareService->turnOnXRay();

    QTimer::singleShot(200, this, [this]() {
        if (m_stateMachine->configuration().contains(m_scanningState)) {
            Log("ScanController: X-Ray source stable. Acquiring frame.");
            QMetaObject::invokeMethod(m_dataAcquisitionService, "acquireSingleFrame", Qt::QueuedConnection);
        }
    });
}


void ScanController::onRawFrameReady(FramePtr frame)
{
    Log(QString("ScanController: Raw frame %1 received. Turning off X-Ray.").arg(frame->frameNumber));
    m_hardwareService->turnOffXRay();

    SystemStatus currentStatus = m_hardwareService->getSystemStatus();

    frame->angle = currentStatus.motionStageStatus.currentPosition;
    frame->xrayVoltage_kV = currentStatus.xrayStatus.currentVoltage_kV;
    frame->xrayCurrent_uA = currentStatus.xrayStatus.currentCurrent_uA;

    emit newProjectionImage(frame->image);

    m_currentFrame++;

    updateScanProgress();
    QTimer::singleShot(50, this, &ScanController::startNextScanStep);
}

void ScanController::onSafetyInterlockTriggered(const QString &reason)
{
    if (m_stateMachine->configuration().contains(m_idleState) ||
        m_stateMachine->configuration().contains(m_errorState)) {
        return;
    }

    Log("FATAL: Safety interlock triggered! Reason: " + reason);
    emit errorOccurred(reason);
    emit forceErrorState();
}
