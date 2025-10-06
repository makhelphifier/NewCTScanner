// src/core/scancontroller.h

#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include "common/ScanParameters.h"
#include "common/Logger.h"
#include "core/Frame.h"
#include <QState>
#include "common/ScanProgress.h"
#include <QElapsedTimer>

class ConfigManager;
class ReconstructionController;
class HardwareService;
class DataAcquisitionService;
class SystemSafetyService;

class ScanController : public QObject
{
    Q_OBJECT
public:
    explicit ScanController(HardwareService* hardwareService,
                            DataAcquisitionService* dataAcquisitionService,
                            ConfigManager* configManager,
                            ReconstructionController* reconController,
                            SystemSafetyService* safetyService);
      ~ScanController();

    enum ScanState {
        StateIdle,
        StatePreparing,
        StateScanning,
        StatePaused,
        StateError
    };
    QString getSaveDirectory() const;

public slots:
    void init();
    void requestScan();
    void requestStop();
    void updateParameters(const ScanParameters &params);
    void updateSavePath(const QString &directory, const QString &prefix);
    void saveConfiguration(const QString &filePath);
    void loadConfiguration(const QString &filePath);
    void requestPause();
    void requestResume();
    void onRawFrameReady(FramePtr frame);

private slots:
    void onIdle();
    void onPreparing();
    void onScanning();
    void onError();
    void onPaused();

    void onMoveFinished(bool success);
    void onSafetyInterlockTriggered(const QString &reason);

signals:
    void statusUpdated(const QString &status);
    void stateChanged(ScanState newState);
    void newProjectionImage(const QImage &image);
    void errorOccurred(const QString &errorMessage);
    void scanProgress(int current, int total);
    void reconstructionStarted();
    void configurationLoaded(const ScanParameters &params);
    void pauseRequested();
    void resumeRequested();
    void scanRequested();
    void stopRequested();
    void preparationFinished();
    void forceErrorState();
    void scanCompleted();
    void scanProgress(const ScanProgress &progress);

private:
    void setupStateMachine();
    void startNextScanStep();
    bool validateParameters(QString &errorMessage);
    void updateScanProgress();

    HardwareService* m_hardwareService;
    DataAcquisitionService* m_dataAcquisitionService;
    ConfigManager* m_configManager;
    ReconstructionController* m_reconController;

    QStateMachine* m_stateMachine;
    QState* m_idleState;
    QState* m_preparingState;
    QState* m_scanningState;
    QState* m_errorState;
    QState* m_pausedState;

    ScanParameters m_currentParams;
    QString m_saveDirectory;
    QString m_savePrefix;

    int m_currentFrame;
    int m_totalFrames;

    ScanProgress m_scanProgress;
    QElapsedTimer m_scanTimer;

    SystemSafetyService* m_safetyService;

};
#endif // SCANCONTROLLER_H
