// src/core/scancontroller.h

#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include "common/ScanParameters.h"
#include "common/Logger.h"
#include "core/Frame.h"

class ConfigManager;
class ReconstructionController;
class HardwareService;
class DataAcquisitionService;

class ScanController : public QObject
{
    Q_OBJECT
public:
    explicit ScanController(HardwareService* hardwareService,
                            DataAcquisitionService* dataAcquisitionService,
                            ConfigManager* configManager,
                            ReconstructionController* reconController);
    ~ScanController();

    enum ScanState {
        StateIdle,
        StatePreparing,
        StateScanning,
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

    void onRawFrameReady(FramePtr frame);

private slots:
    void onIdle();
    void onPreparing();
    void onScanning();
    void onError();

    void onMoveFinished(bool success);

signals:
    void statusUpdated(const QString &status);
    void stateChanged(ScanState newState);
    void newProjectionImage(const QImage &image);
    void errorOccurred(const QString &errorMessage);
    void scanProgress(int current, int total);
    void reconstructionStarted();
    void configurationLoaded(const ScanParameters &params);

    void scanRequested();
    void stopRequested();
    void preparationFinished();
    void forceErrorState();
    void scanCompleted();

private:
    void setupStateMachine();
    void startNextScanStep();
    HardwareService* m_hardwareService;
    DataAcquisitionService* m_dataAcquisitionService;
    ConfigManager* m_configManager;
    ReconstructionController* m_reconController;

    QStateMachine* m_stateMachine;
    QState* m_idleState;
    QState* m_preparingState;
    QState* m_scanningState;
    QState* m_errorState;

    ScanParameters m_currentParams;
    QString m_saveDirectory;
    QString m_savePrefix;

    int m_currentFrame;
    int m_totalFrames;
};
#endif // SCANCONTROLLER_H
