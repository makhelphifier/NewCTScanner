#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include "core/scancontroller.h"
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>

class ScanController;
class QPushButton;

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
    void updateScanProgress(int current, int total);
    void applyLoadedParameters(const ScanParameters &params);
    void onReconstructionStarted();
    void updateReconProgress(int percentage);
    void displayReconResult(const QImage &sliceImage);

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

private slots:
    void onParametersChanged();
    void onSavePathChanged();
    void browseForDirectory();
    void onSaveConfig();
    void onLoadConfig();
signals:
    void parametersChanged(const ScanParameters &params);
    void savePathChanged(const QString &directory, const QString &prefix);
};
#endif // MAINWINDOW_H
