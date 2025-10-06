#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "core/scancontroller.h"
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include "common/HardwareStatus.h"
#include "common/ScanProgress.h"

class ScanController;
class QPushButton;
class HardwareService;
class SystemSafetyService;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void updateStatus(const QString &status);
    void onStateChanged(ScanController::ScanState newState);
    void updateImage(const QImage &image);
    void appendLogMessage(const QString &message);
    void displayError(const QString &errorMessage);
    void applyLoadedParameters(const ScanParameters &params);
    void onReconstructionStarted();
    void updateReconProgress(int percentage);
    void displayReconResult(const QImage &sliceImage);
    void onSystemStatusUpdated(const SystemStatus &status);
    void updateScanProgress(const ScanProgress &progress);

private:
    ScanController* m_scanController;
    QPushButton* m_startXRayButton;
    QPushButton* m_stopButton;
    QThread* m_workerThread;
    QLabel* m_statusLabel;
    QDoubleSpinBox* m_voltageSpinBox;
    QDoubleSpinBox* m_currentSpinBox;
    QSpinBox* m_frameCountSpinBox;
    QLabel* m_imageDisplayLabel;
    QPlainTextEdit* m_logDisplay;
    QLineEdit* m_saveDirLineEdit;
    QLineEdit* m_savePrefixLineEdit;
    QPushButton* m_browseButton;
    void setParametersEnabled(bool enabled);
    QProgressBar* m_scanProgressBar;
    QPushButton* m_loadConfigButton;
    QPushButton* m_saveConfigButton;
    QLabel* m_reconDisplayLabel;
    QProgressBar* m_reconProgressBar;
    HardwareService* m_hardwareService;
    QPushButton* m_pauseButton;
    QPushButton* m_resumeButton;
    QDoubleSpinBox* m_startAngleSpinBox;
    QDoubleSpinBox* m_endAngleSpinBox;
    QDoubleSpinBox* m_rotationSpeedSpinBox;
    SystemSafetyService* m_safetyService;
    QPushButton* m_doorButton;

private slots:
    void onParametersChanged();
    void onSavePathChanged();
    void browseForDirectory();
    void onSaveConfig();
    void onLoadConfig();
    void onDoorButtonClicked();

signals:
    void parametersChanged(const ScanParameters &params);
    void savePathChanged(const QString &directory, const QString &prefix);
};
#endif // MAINWINDOW_H
