#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
// 前向声明
class IXRaySource;

class ScanController : public QObject
{
    Q_OBJECT
public:
    explicit ScanController();
    ~ScanController();
    enum ScanState {
        StateIdle,
        StatePreparing,
        StateScanning
    };
public slots:
    // UI请求开始扫描
    void requestScan();
    // UI请求停止/复位
    void requestStop();
private slots:
    // 进入“准备”状态时执行
    void onPreparing();
    // 进入“扫描”状态时执行
    void onScanning();
    // 进入“空闲”状态时执行
    void onIdle();
signals:
    void statusUpdated(const QString &status);
    void scanRequested();
    void stopRequested();
    void preparationFinished();
    void stateChanged(ScanState newState);

private:
    // 使用接口指针，而不是具体实现
    IXRaySource* m_xraySource;
    // 状态机和状态
    QStateMachine* m_stateMachine;
    QState* m_idleState;
    QState* m_preparingState;
    QState* m_scanningState;

    // 私有方法，用于初始化状态机
    void setupStateMachine();
};

#endif // SCANCONTROLLER_H
