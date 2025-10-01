#include "scancontroller.h"
#include "hal/DummySource.h" // 包含具体的硬件实现
#include <QTimer> // 我们会用一个定时器来模拟“准备”过程
#include <QDebug>
ScanController::ScanController()
    : QObject(nullptr)
{
    m_xraySource = new DummySource();
    setupStateMachine();

    // 启动状态机
    m_stateMachine->start();
}

ScanController::~ScanController()
{
    // 清理资源
    if (m_xraySource) {
        delete m_xraySource;
        m_xraySource = nullptr;
    }
}
void ScanController::requestScan()
{
    qDebug() << "Scan requested by UI.";
    // 发射一个内部信号，状态机会捕捉到这个信号并触发状态转换
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

    // 1. 创建状态
    m_idleState = new QState();
    m_preparingState = new QState();
    m_scanningState = new QState();

    // 2. 定义状态进入时要执行的动作
    connect(m_idleState, &QState::entered, this, &ScanController::onIdle);
    connect(m_preparingState, &QState::entered, this, &ScanController::onPreparing);
    connect(m_scanningState, &QState::entered, this, &ScanController::onScanning);

    // 3. 定义状态转换
    //    从 Idle -> Preparing, 当收到 scanRequested 信号时
    m_idleState->addTransition(this, &ScanController::scanRequested, m_preparingState);

    //    从 Preparing -> Scanning, 当收到 preparationFinished 信号时
    m_preparingState->addTransition(this, &ScanController::preparationFinished, m_scanningState);

    //    从任何状态 -> Idle, 当收到 stopRequested 信号时 (复位功能)
    m_scanningState->addTransition(this, &ScanController::stopRequested, m_idleState);
    m_preparingState->addTransition(this, &ScanController::stopRequested, m_idleState);


    // 4. 将状态添加到状态机
    m_stateMachine->addState(m_idleState);
    m_stateMachine->addState(m_preparingState);
    m_stateMachine->addState(m_scanningState);

    // 5. 设置初始状态
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
    // 模拟一个2秒的准备过程（例如：等待旋转台加速）
    QTimer::singleShot(2000, this, [this](){
        // 准备完成后，发射信号以转换到下一个状态
        emit preparationFinished();
        emit stateChanged(StatePreparing);
    });
}

void ScanController::onScanning()
{
    emit statusUpdated("Scanning... X-Ray is ON");
    if (m_xraySource) m_xraySource->turnOn();
    emit stateChanged(StateScanning);
}
