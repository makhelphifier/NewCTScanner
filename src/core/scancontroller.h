#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include "common/ScanParameters.h"
#include "common/Logger.h"
#include "core/Frame.h" // 新增

class ConfigManager;
class IXRaySource;
class DataSaver;
class QThread;
class ReconstructionController;
class HardwareService;
class IDetector;
class IMotionStage;
class DataAcquisitionService;
class ScanController : public QObject
{
    Q_OBJECT
public:
    explicit ScanController(DataAcquisitionService* dataAcquisitionService,
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
    void requestScan();
    void requestStop();
    void updateParameters(const ScanParameters &params);
    void init();
    void updateSavePath(const QString &directory, const QString &prefix);
    void saveConfiguration(const QString &filePath);
    void loadConfiguration(const QString &filePath);
    void onAcquisitionFinished();

private slots:
    void onPreparing();
    void onScanning();
    void onIdle();
    void onError();

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
    DataAcquisitionService* m_dataAcquisitionService;
    QStateMachine* m_stateMachine;

    QState* m_idleState;
    QState* m_preparingState;
    QState* m_scanningState;
    QState* m_errorState;

    ScanParameters m_currentParams;
    QString m_saveDirectory;
    QString m_savePrefix;

    ConfigManager* m_configManager;
    ReconstructionController* m_reconController;
};
#endif // SCANCONTROLLER_H
