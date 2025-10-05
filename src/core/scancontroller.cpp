#include "scancontroller.h"
#include "hal/DummySource.h"
#include <QTimer>
#include <QDebug>
#include "core/datasaver.h"
#include <QThread>
#include "core/configmanager.h"
#include "core/reconstructioncontroller.h"


ScanController::ScanController()
    : QObject(nullptr), m_xraySource(nullptr)
{
    m_saverThread = new QThread(this);
    m_dataSaver = new DataSaver();
    m_dataSaver->moveToThread(m_saverThread);
    connect(m_saverThread, &QThread::finished, m_dataSaver, &QObject::deleteLater);
    m_saverThread->start();
    Log("DataSaver thread started.");

    setupStateMachine();
    m_stateMachine->start();

    m_configManager = new ConfigManager(this);
    //将配置管理器的加载成功信号连接到本类的信号
    connect(m_configManager, &ConfigManager::configLoaded, this, &ScanController::configurationLoaded);


    // ... (在 m_dataSaver 线程之后)
    m_reconThread = new QThread(this);
    m_reconController = new ReconstructionController();
    m_reconController->moveToThread(m_reconThread);

    // 连接重建模块的信号到本类的转发信号
    connect(m_reconController, &ReconstructionController::reconstructionProgress,
            this, &ScanController::reconstructionProgress);
    connect(m_reconController, &ReconstructionController::reconstructionFinished,
            this, &ScanController::reconstructionFinished);

    connect(m_reconThread, &QThread::finished, m_reconController, &QObject::deleteLater);
    m_reconThread->start();
    Log("Reconstruction thread started.");



}
void ScanController::init() {
    m_xraySource = new DummySource();
    if (!m_xraySource->connect()){
        onHardwareError("Initial connection to X-Ray source failed.");
    }
    connect(m_xraySource, &IXRaySource::newImageReady,
            this, &ScanController::onNewImageFromSource);
    connect(m_xraySource, &IXRaySource::hardwareError, this, &ScanController::onHardwareError);
    connect(m_xraySource, &IXRaySource::acquisitionProgress, this, &ScanController::scanProgress);
    connect(m_xraySource, &IXRaySource::acquisitionFinished, this, &ScanController::onAcquisitionFinished);
}
ScanController::~ScanController()
{
    if (m_xraySource) {
        delete m_xraySource;
        m_xraySource = nullptr;
    }
    m_saverThread->quit();
    m_saverThread->wait();

    m_reconThread->quit();
    m_reconThread->wait();
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

    m_idleState = new QState();
    m_preparingState = new QState();
    m_scanningState = new QState();
    m_errorState = new QState();

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

    m_stateMachine->addState(m_idleState);
    m_stateMachine->addState(m_preparingState);
    m_stateMachine->addState(m_scanningState);

    m_stateMachine->setInitialState(m_idleState);


}

void ScanController::onIdle()
{
    QMetaObject::invokeMethod(m_dataSaver, "stopSaving", Qt::QueuedConnection);
    emit statusUpdated("Idle");
    if (m_xraySource) m_xraySource->turnOff();
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
    QMetaObject::invokeMethod(m_dataSaver, "startSaving", Qt::QueuedConnection,
                              Q_ARG(QString, m_saveDirectory),
                              Q_ARG(QString, m_savePrefix));
    emit statusUpdated("Scanning... X-Ray is ON");
    emit stateChanged(StateScanning);
    if (m_xraySource) {
        m_xraySource->setVoltage(m_currentParams.voltage);
        m_xraySource->turnOn(m_currentParams.frameCount); // <-- 传递帧数
    }
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
    if (m_xraySource) m_xraySource->turnOff();
    QMetaObject::invokeMethod(m_dataSaver, "stopSaving", Qt::QueuedConnection);
}



void ScanController::onAcquisitionFinished()
{
    Log("ScanController: Received acquisition finished signal. Triggering reconstruction.");

    emit reconstructionStarted();

    // 通过元调用，将重建任务交给重建线程
    QMetaObject::invokeMethod(m_reconController, "startReconstruction", Qt::QueuedConnection,
                              Q_ARG(QString, m_saveDirectory));

    emit scanCompleted(); // 告诉状态机扫描本身已完成
}


void ScanController::saveConfiguration(const QString &filePath)
{
    m_configManager->saveConfig(m_currentParams, filePath);
}

void ScanController::loadConfiguration(const QString &filePath)
{
    m_configManager->loadConfig(filePath);
}
