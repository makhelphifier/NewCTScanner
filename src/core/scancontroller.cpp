#include "scancontroller.h"
#include "hal/DummySource.h"
#include <QTimer>
#include <QDebug>
ScanController::ScanController()
    : QObject(nullptr)
{
    m_xraySource = new DummySource();
    setupStateMachine();
    connect(m_xraySource, &IXRaySource::newImageReady,
            this, &ScanController::onNewImageFromSource);

    m_stateMachine->start();
}

ScanController::~ScanController()
{
    if (m_xraySource) {
        delete m_xraySource;
        m_xraySource = nullptr;
    }
}
void ScanController::requestScan()
{
    qDebug() << "Scan requested by UI.";
    emit scanRequested();
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

    connect(m_idleState, &QState::entered, this, &ScanController::onIdle);
    connect(m_preparingState, &QState::entered, this, &ScanController::onPreparing);
    connect(m_scanningState, &QState::entered, this, &ScanController::onScanning);

    m_idleState->addTransition(this, &ScanController::scanRequested, m_preparingState);

    m_preparingState->addTransition(this, &ScanController::preparationFinished, m_scanningState);

    m_scanningState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_preparingState->addTransition(this, &ScanController::stopRequested, m_idleState);


    m_stateMachine->addState(m_idleState);
    m_stateMachine->addState(m_preparingState);
    m_stateMachine->addState(m_scanningState);

    m_stateMachine->setInitialState(m_idleState);
}

void ScanController::onIdle()
{
    emit statusUpdated("Idle");
    if (m_xraySource) m_xraySource->turnOff();
    emit stateChanged(StateIdle);
}
void ScanController::onPreparing()
{
    emit statusUpdated("Preparing...");
    emit stateChanged(StatePreparing);
    QTimer::singleShot(2000, this, [this](){
        emit preparationFinished();
    });
}

void ScanController::onScanning()
{
    emit statusUpdated("Scanning... X-Ray is ON");
    if (m_xraySource) m_xraySource->turnOn();
    emit stateChanged(StateScanning);
    if (m_xraySource) {
        m_xraySource->setVoltage(m_currentParams.voltage);
        m_xraySource->turnOn();
    }
    emit stateChanged(StateScanning);
}
void ScanController::updateParameters(const ScanParameters &params)
{
    m_currentParams = params;
    qDebug() << "Parameters updated in Core: kV=" << m_currentParams.voltage
             << "uA=" << m_currentParams.current;
}

void ScanController::onNewImageFromSource(const QImage &image)
{    emit newProjectionImage(image);
}

