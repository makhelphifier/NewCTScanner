#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include "common/ScanParameters.h"
#include "common/Logger.h"

class ConfigManager;
class IXRaySource;
class DataSaver;
class QThread;
class ReconstructionController;
class HardwareService;
class IDetector;
class IMotionStage;

class ScanController : public QObject
{
    Q_OBJECT
public:
    explicit ScanController(HardwareService* hardwareService);
    ~ScanController();
    enum ScanState {
        StateIdle,
        StatePreparing,
        StateScanning,
        StateError
    };
public slots:
    void requestScan();
    void requestStop();
    void updateParameters(const ScanParameters &params);
    void init();
    void updateSavePath(const QString &directory, const QString &prefix);
    void saveConfiguration(const QString &filePath);
    void loadConfiguration(const QString &filePath);

private slots:
    void onPreparing();
    void onScanning();
    void onIdle();
    void onNewImageFromSource(const QImage &image);
    void onHardwareError(const QString &errorMessage);
    void onError();
    void onAcquisitionFinished();
    void onMoveFinished(bool success);
    void onFrameAcquired();

signals:
    void statusUpdated(const QString &status);
    void scanRequested();
    void stopRequested();
    void preparationFinished();
    void stateChanged(ScanState newState);
    void newProjectionImage(const QImage &image);
    void errorOccurred(const QString &errorMessage);
    void forceErrorState();
    void scanProgress(int current, int total);
    void scanCompleted();
    void configurationLoaded(const ScanParameters &params);
    void reconstructionStarted();
    void reconstructionProgress(int percentage);
    void reconstructionFinished(const QImage &sliceImage);

    void commandTurnOn(int numberOfFrames);
    void commandTurnOff();
    void commandSetVoltage(double kv);
    void commandMoveTo(double position);
    void commandAcquireFrame();
private:
    void startNextAcquisitionStep();
    IXRaySource* m_xraySource;
    HardwareService* m_hardwareService;
    QStateMachine* m_stateMachine;
    QState* m_idleState;
    QState* m_preparingState;
    QState* m_scanningState;
    void setupStateMachine();
    ScanParameters m_currentParams;
    DataSaver* m_dataSaver;
    QThread* m_saverThread;
    QString m_saveDirectory;
    QString m_savePrefix;
    QState* m_errorState;
    ConfigManager* m_configManager;
    ReconstructionController* m_reconController;
    // QThread* m_reconThread;
    IDetector* m_detector;
    IMotionStage* m_motionStage;
    int m_currentFrame;

};

#endif // SCANCONTROLLER_H
