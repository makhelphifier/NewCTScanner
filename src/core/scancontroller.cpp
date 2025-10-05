#include "scancontroller.h"
#include "hal/DummySource.h"
#include <QTimer>
#include <QDebug>
#include "core/datasaver.h"
#include <QThread>
#include "core/configmanager.h"
#include "core/reconstructioncontroller.h"
#include "core/hardwareservice.h"
#include <QThread>
#include "hal/IDetector.h"
#include "hal/IMotionStage.h"

ScanController::ScanController(HardwareService* hardwareService)
    : QObject(nullptr), m_xraySource(nullptr),
    m_hardwareService(hardwareService)

{
    m_saverThread = new QThread(this);
    m_dataSaver = new DataSaver();
    m_dataSaver->moveToThread(m_saverThread);
    connect(m_saverThread, &QThread::finished, m_dataSaver, &QObject::deleteLater);
    m_saverThread->start();
    Log("DataSaver thread started.");

    setupStateMachine();
    // m_stateMachine->start();

    m_configManager = new ConfigManager(this);
    connect(m_configManager, &ConfigManager::configLoaded, this, &ScanController::configurationLoaded);


    m_reconController = new ReconstructionController();

    connect(m_reconController, &ReconstructionController::reconstructionProgress,
            this, &ScanController::reconstructionProgress);
    connect(m_reconController, &ReconstructionController::reconstructionFinished,
            this, &ScanController::reconstructionFinished);

    // connect(m_reconThread, &QThread::finished, m_reconController, &QObject::deleteLater);
    // m_reconThread->start();
    Log("Reconstruction thread started.");



}
void ScanController::init() {
    m_xraySource = m_hardwareService->xraySource();
    m_detector = m_hardwareService->detector();
    m_motionStage = m_hardwareService->motionStage();

    if (!m_xraySource || !m_detector || !m_motionStage){
        onHardwareError("Failed to get hardware from HardwareService.");
        return;
    }
    // 连接来自硬件的信号
    connect(m_detector, &IDetector::newImageReady,
            this, &ScanController::onNewImageFromSource, Qt::QueuedConnection);
    connect(m_detector, &IDetector::acquisitionFinished,
            this, &ScanController::onFrameAcquired, Qt::QueuedConnection);
    connect(m_motionStage, &IMotionStage::moveFinished,
            this, &ScanController::onMoveFinished, Qt::QueuedConnection);


    if (!m_xraySource->connect()){
        onHardwareError("Initial connection to X-Ray source failed.");
    }
    connect(m_xraySource, &IXRaySource::newImageReady,
            this, &ScanController::onNewImageFromSource);
    connect(m_xraySource, &IXRaySource::hardwareError, this, &ScanController::onHardwareError);
    connect(m_xraySource, &IXRaySource::acquisitionProgress, this, &ScanController::scanProgress);
    connect(m_xraySource, &IXRaySource::acquisitionFinished, this, &ScanController::onAcquisitionFinished);


    connect(this, &ScanController::commandTurnOn,
            m_xraySource, &IXRaySource::turnOn, Qt::QueuedConnection);
    connect(this, &ScanController::commandTurnOff,
            m_xraySource, &IXRaySource::turnOff, Qt::QueuedConnection);
    connect(this, &ScanController::commandSetVoltage,
            m_xraySource, &IXRaySource::setVoltage, Qt::QueuedConnection);

    connect(this, &ScanController::commandMoveTo,
            m_motionStage, &IMotionStage::moveTo, Qt::QueuedConnection);
    connect(this, &ScanController::commandAcquireFrame,
            m_detector, &IDetector::acquireFrame, Qt::QueuedConnection);
    m_stateMachine->start();

    Log("ScanController initialized and connected to hardware signals.");

}
ScanController::~ScanController()
{
    m_saverThread->quit();
    m_saverThread->wait();

    // m_reconThread->quit();
    // m_reconThread->wait();
}
void ScanController::requestScan()
{
    emit scanRequested();
    Log("UI requested to start scan.");

}

void ScanController::requestStop()
{
    qDebug() << "Stop requested by UI.";
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

    m_scanningState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_preparingState->addTransition(this, &ScanController::stopRequested, m_idleState);

    m_idleState->addTransition(this, &ScanController::forceErrorState, m_errorState);
    m_preparingState->addTransition(this, &ScanController::forceErrorState, m_errorState);
    m_scanningState->addTransition(this, &ScanController::forceErrorState, m_errorState);

    m_errorState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_scanningState->addTransition(this, &ScanController::scanCompleted, m_idleState);

    // m_stateMachine->addState(m_idleState);
    // m_stateMachine->addState(m_preparingState);
    // m_stateMachine->addState(m_scanningState);

    m_stateMachine->setInitialState(m_idleState);
}

void ScanController::onIdle()
{
    QMetaObject::invokeMethod(m_dataSaver, "stopSaving", Qt::QueuedConnection);
    emit statusUpdated("Idle");
    if (m_xraySource) emit commandTurnOff();
    emit stateChanged(StateIdle);
    Log("State machine entered Idle state.");

}

void ScanController::onPreparing()
{
    emit statusUpdated("Preparing...");
    emit stateChanged(StatePreparing);
    QTimer::singleShot(2000, this, [this](){
        emit preparationFinished();
    });
    Log("State machine entered Preparing state.");

}

void ScanController::onScanning()
{
    m_currentFrame = 0;
    QMetaObject::invokeMethod(m_dataSaver, "startSaving", Qt::QueuedConnection,
                              Q_ARG(QString, m_saveDirectory),
                              Q_ARG(QString, m_savePrefix));
    emit statusUpdated("Scanning... X-Ray is ON");
    emit stateChanged(StateScanning);

    if (m_xraySource) {
        emit commandSetVoltage(m_currentParams.voltage);
        emit commandTurnOn(0);
    }

    QTimer::singleShot(500, this, &ScanController::startNextAcquisitionStep);

    Log(QString("State machine entered Scanning state with params: kV=%1, uA=%2").arg(m_currentParams.voltage).arg(m_currentParams.current));
}

void ScanController::updateParameters(const ScanParameters &params)
{
    m_currentParams = params;
    qDebug() << "Parameters updated in Core: kV=" << m_currentParams.voltage
             << "uA=" << m_currentParams.current;
    Log(QString("Scan parameters updated: kV=%1, uA=%2").arg(params.voltage).arg(params.current));
}

void ScanController::onNewImageFromSource(const QImage &image)
{
    emit newProjectionImage(image);

    QMetaObject::invokeMethod(m_dataSaver, "queueImage", Qt::QueuedConnection,
                              Q_ARG(QImage, image));
}
void ScanController::updateSavePath(const QString &directory, const QString &prefix)
{
    m_saveDirectory = directory;
    m_savePrefix = prefix;
    Log(QString("Save path updated. Dir: %1, Prefix: %2").arg(directory).arg(prefix));
}
void ScanController::onHardwareError(const QString &errorMessage)
{
    emit errorOccurred(errorMessage);
    emit forceErrorState();
}

void ScanController::onError()
{
    emit statusUpdated("Error! Please reset.");
    emit stateChanged(StateError);
    if (m_xraySource) emit commandTurnOff();
    QMetaObject::invokeMethod(m_dataSaver, "stopSaving", Qt::QueuedConnection);
}

void ScanController::onAcquisitionFinished()
{
    Log("ScanController: All frames acquired. Turning off X-Ray and triggering reconstruction.");

    if (m_xraySource) emit commandTurnOff();
    emit scanCompleted();

    emit reconstructionStarted();

    QMetaObject::invokeMethod(m_reconController, "startReconstruction", Qt::QueuedConnection,
                              Q_ARG(QString, m_saveDirectory));
}
void ScanController::saveConfiguration(const QString &filePath)
{
    m_configManager->saveConfig(m_currentParams, filePath);
}

void ScanController::loadConfiguration(const QString &filePath)
{
    m_configManager->loadConfig(filePath);
}

void ScanController::startNextAcquisitionStep()
{
    if (m_currentFrame >= m_currentParams.frameCount) {
        onAcquisitionFinished();
        return;
    }

    emit scanProgress(m_currentFrame + 1, m_currentParams.frameCount);

    double nextAngle = (360.0 / m_currentParams.frameCount) * m_currentFrame;
    emit commandMoveTo(nextAngle);
}

void ScanController::onMoveFinished(bool success)
{
    if (!success) {
        onHardwareError("Motion stage failed to move.");
        return;
    }
    emit commandAcquireFrame();
}

void ScanController::onFrameAcquired()
{
    m_currentFrame++;
    startNextAcquisitionStep();
}
