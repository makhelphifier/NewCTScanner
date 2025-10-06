#include "mainwindow.h"
#include "core/scancontroller.h"
#include <QPushButton>
#include <QVBoxLayout>
#include "common/Logger.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QFileDialog>
#include "core/corefacade.h"
#include "core/reconstructioncontroller.h"
#include "core/hardwareservice.h"
#include <QTime>
#include "core/systemsafetyservice.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_scanController = CoreFacade::instance().scanController();
    m_hardwareService = CoreFacade::instance().hardwareService();
    m_safetyService = CoreFacade::instance().safetyService();

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startXRayButton = new QPushButton("Start Scan", this);
    m_stopButton = new QPushButton("Stop/Reset", this);
    m_pauseButton = new QPushButton("Pause", this);
    m_resumeButton = new QPushButton("Resume", this);
    buttonLayout->addWidget(m_startXRayButton);
    buttonLayout->addWidget(m_pauseButton);
    buttonLayout->addWidget(m_resumeButton);
    buttonLayout->addWidget(m_stopButton);
    layout->addLayout(buttonLayout);

    m_statusLabel = new QLabel("Initializing...", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    QFormLayout *formLayout = new QFormLayout();
    m_voltageSpinBox = new QDoubleSpinBox(this);
    m_voltageSpinBox->setRange(20.0, 225.0);
    m_voltageSpinBox->setValue(100.0);
    m_voltageSpinBox->setSuffix(" kV");

    m_currentSpinBox = new QDoubleSpinBox(this);
    m_currentSpinBox->setRange(10.0, 1000.0);
    m_currentSpinBox->setValue(50.0);
    m_currentSpinBox->setSuffix(" uA");
    m_frameCountSpinBox = new QSpinBox(this);
    m_frameCountSpinBox->setRange(1, 10000);
    m_frameCountSpinBox->setValue(360);

    m_startAngleSpinBox = new QDoubleSpinBox(this);
    m_startAngleSpinBox->setRange(0.0, 360.0);
    m_startAngleSpinBox->setValue(0.0);
    m_startAngleSpinBox->setSuffix(" °");

    m_endAngleSpinBox = new QDoubleSpinBox(this);
    m_endAngleSpinBox->setRange(0.0, 360.0);
    m_endAngleSpinBox->setValue(360.0);
    m_endAngleSpinBox->setSuffix(" °");

    m_rotationSpeedSpinBox = new QDoubleSpinBox(this);
    m_rotationSpeedSpinBox->setRange(0.1, 10.0);
    m_rotationSpeedSpinBox->setValue(1.0);
    m_rotationSpeedSpinBox->setSuffix(" °/s");


    formLayout->addRow("Voltage:", m_voltageSpinBox);
    formLayout->addRow("Current:", m_currentSpinBox);
    formLayout->addRow("Frame Count:", m_frameCountSpinBox);
    formLayout->addRow("Start Angle:", m_startAngleSpinBox);
    formLayout->addRow("End Angle:", m_endAngleSpinBox);
    formLayout->addRow("Rotation Speed:", m_rotationSpeedSpinBox);

    QHBoxLayout* saveLayout = new QHBoxLayout();
    m_saveDirLineEdit = new QLineEdit("./scan_data", this);
    m_savePrefixLineEdit = new QLineEdit("scan", this);
    m_browseButton = new QPushButton("Browse...", this);
    saveLayout->addWidget(new QLabel("Save Path:", this));
    saveLayout->addWidget(m_saveDirLineEdit);
    saveLayout->addWidget(new QLabel("Prefix:", this));
    saveLayout->addWidget(m_savePrefixLineEdit);
    saveLayout->addWidget(m_browseButton);
    layout->addLayout(saveLayout);

    QHBoxLayout* configLayout = new QHBoxLayout();
    m_loadConfigButton = new QPushButton("Load Config", this);
    m_saveConfigButton = new QPushButton("Save Config", this);
    configLayout->addWidget(m_loadConfigButton);
    configLayout->addWidget(m_saveConfigButton);
    m_doorButton = new QPushButton("Door Closed", this);
    m_doorButton->setCheckable(true);
    m_doorButton->setChecked(true);
    configLayout->addWidget(m_doorButton);
    layout->addLayout(configLayout);

    layout->addLayout(formLayout);
    m_imageDisplayLabel = new QLabel(this);
    m_imageDisplayLabel->setMinimumSize(256, 256);
    m_imageDisplayLabel->setStyleSheet("background-color: black; border: 1px solid gray;");
    m_imageDisplayLabel->setAlignment(Qt::AlignCenter);
    m_imageDisplayLabel->setText("IMAGE DISPLAY");
    layout->addWidget(m_imageDisplayLabel);

    QVBoxLayout* reconLayout = new QVBoxLayout();
    m_reconDisplayLabel = new QLabel(this);
    m_reconDisplayLabel->setMinimumSize(256, 256);
    m_reconDisplayLabel->setStyleSheet("background-color: black; border: 1px solid gray;");
    m_reconDisplayLabel->setAlignment(Qt::AlignCenter);
    m_reconDisplayLabel->setText("RECONSTRUCTION RESULT");

    m_reconProgressBar = new QProgressBar(this);
    m_reconProgressBar->setTextVisible(true);
    m_reconProgressBar->setValue(0);
    m_reconProgressBar->setFormat("Reconstruction: %p%");

    reconLayout->addWidget(m_reconDisplayLabel);
    reconLayout->addWidget(m_reconProgressBar);

    layout->addLayout(reconLayout);
    m_scanProgressBar = new QProgressBar(this);
    m_scanProgressBar->setTextVisible(true);
    m_scanProgressBar->setValue(0);
    layout->addWidget(m_scanProgressBar);

    m_logDisplay = new QPlainTextEdit(this);
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setMaximumHeight(150);
    layout->addWidget(m_logDisplay);
    layout->addWidget(m_statusLabel);
    this->setCentralWidget(centralWidget);
    this->setWindowTitle("New CT Scanner Control");
    connect(m_startXRayButton, &QPushButton::clicked,
            m_scanController, &ScanController::requestScan);
    connect(m_stopButton, &QPushButton::clicked,
            m_scanController, &ScanController::requestStop);
    connect(m_pauseButton, &QPushButton::clicked,
            m_scanController, &ScanController::requestPause);
    connect(m_resumeButton, &QPushButton::clicked,
            m_scanController, &ScanController::requestResume);
    connect(m_scanController, &ScanController::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(m_voltageSpinBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::onParametersChanged);
    connect(m_currentSpinBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::onParametersChanged);
    connect(m_frameCountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::onParametersChanged);
    connect(m_startAngleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onParametersChanged);
    connect(m_endAngleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onParametersChanged);
    connect(m_rotationSpeedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onParametersChanged);
    connect(this, &MainWindow::parametersChanged,
            m_scanController, &ScanController::updateParameters);
    connect(m_scanController, &ScanController::newProjectionImage,
            this, &MainWindow::updateImage, Qt::QueuedConnection);
    connect(Logger::instance(), &Logger::newLogMessage,
            this, &MainWindow::appendLogMessage, Qt::QueuedConnection);
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::browseForDirectory);
    connect(m_saveDirLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSavePathChanged);
    connect(m_savePrefixLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSavePathChanged);
    connect(this, &MainWindow::savePathChanged, m_scanController, &ScanController::updateSavePath);
    connect(m_scanController, &ScanController::errorOccurred,
            this, &MainWindow::displayError, Qt::QueuedConnection);
    connect(m_scanController, SIGNAL(scanProgress(const ScanProgress&)),
            this, SLOT(updateScanProgress(const ScanProgress&)));

    connect(m_loadConfigButton, &QPushButton::clicked, this, &MainWindow::onLoadConfig);
    connect(m_saveConfigButton, &QPushButton::clicked, this, &MainWindow::onSaveConfig);
    connect(m_doorButton, &QPushButton::clicked, this, &MainWindow::onDoorButtonClicked);

    ReconstructionController* reconController = CoreFacade::instance().reconstructionController();
    connect(m_scanController, &ScanController::reconstructionStarted, this, &MainWindow::onReconstructionStarted);
    connect(reconController, &ReconstructionController::reconstructionProgress, this, &MainWindow::updateReconProgress);
    connect(reconController, &ReconstructionController::reconstructionFinished, this, &MainWindow::displayReconResult);
    connect(m_hardwareService, &HardwareService::systemStatusUpdated,
            this, &MainWindow::onSystemStatusUpdated, Qt::QueuedConnection);

    onSavePathChanged();
    Log("Application started. UI is ready.");
    onParametersChanged();
    onSystemStatusUpdated(m_hardwareService->getSystemStatus());
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void MainWindow::onStateChanged(ScanController::ScanState newState)
{
    bool isIdle = (newState == ScanController::StateIdle);
    m_saveDirLineEdit->setEnabled(isIdle);
    m_savePrefixLineEdit->setEnabled(isIdle);
    m_browseButton->setEnabled(isIdle);

    m_loadConfigButton->setEnabled(isIdle);
    m_saveConfigButton->setEnabled(isIdle);

    switch (newState)
    {
    case ScanController::StateIdle:
        m_startXRayButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        setParametersEnabled(true);
        m_pauseButton->setEnabled(false);
        m_resumeButton->setEnabled(false);
        m_scanProgressBar->setValue(0);
        break;

    case ScanController::StatePreparing:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        setParametersEnabled(false);
        m_pauseButton->setEnabled(false);
        m_resumeButton->setEnabled(false);
        break;

    case ScanController::StateScanning:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        setParametersEnabled(false);
        // m_scanProgressBar->setValue(0);
        m_pauseButton->setEnabled(true);
        m_resumeButton->setEnabled(false);
        break;
    case ScanController::StatePaused:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_pauseButton->setEnabled(false);
        m_resumeButton->setEnabled(true);
        setParametersEnabled(false);
        break;

    case ScanController::StateError:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        setParametersEnabled(false);
        m_pauseButton->setEnabled(false);
        m_resumeButton->setEnabled(false);
        break;
    }
}
void MainWindow::onParametersChanged()
{
    ScanParameters params;
    params.voltage = m_voltageSpinBox->value();
    params.current = m_currentSpinBox->value();
    params.frameCount = m_frameCountSpinBox->value();
    params.startAngle = m_startAngleSpinBox->value();
    params.endAngle = m_endAngleSpinBox->value();
    params.rotationSpeed = m_rotationSpeedSpinBox->value();
    emit parametersChanged(params);
}

void MainWindow::updateImage(const QImage &image)
{
    m_imageDisplayLabel->setPixmap(QPixmap::fromImage(image).scaled(
        m_imageDisplayLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));
}

void MainWindow::appendLogMessage(const QString &message)
{
    m_logDisplay->appendPlainText(message);
    m_logDisplay->ensureCursorVisible();
}

void MainWindow::browseForDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Save Directory", m_saveDirLineEdit->text());
    if (!dir.isEmpty()) {
        m_saveDirLineEdit->setText(dir);
    }
}

void MainWindow::onSavePathChanged()
{
    emit savePathChanged(m_saveDirLineEdit->text(), m_savePrefixLineEdit->text());
}

void MainWindow::displayError(const QString &errorMessage)
{
    QMessageBox::critical(this, "System Error", errorMessage);
}

void MainWindow::setParametersEnabled(bool enabled)
{
    m_voltageSpinBox->setEnabled(enabled);
    m_currentSpinBox->setEnabled(enabled);
    m_frameCountSpinBox->setEnabled(enabled);
    m_saveDirLineEdit->setEnabled(enabled);
    m_savePrefixLineEdit->setEnabled(enabled);
    m_browseButton->setEnabled(enabled);
    m_startAngleSpinBox->setEnabled(enabled);
    m_endAngleSpinBox->setEnabled(enabled);
    m_rotationSpeedSpinBox->setEnabled(enabled);
}



void MainWindow::onLoadConfig()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Load Configuration", "", "JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        m_scanController->loadConfiguration(filePath);
    }
}

void MainWindow::onSaveConfig()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save Configuration", "", "JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        m_scanController->saveConfiguration(filePath);
    }
}

void MainWindow::applyLoadedParameters(const ScanParameters &params)
{
    Log("Applying loaded parameters to UI.");
    m_voltageSpinBox->setValue(params.voltage);
    m_currentSpinBox->setValue(params.current);
    m_frameCountSpinBox->setValue(params.frameCount);
    m_startAngleSpinBox->setValue(params.startAngle);
    m_endAngleSpinBox->setValue(params.endAngle);
    m_rotationSpeedSpinBox->setValue(params.rotationSpeed);

    onParametersChanged();
}


void MainWindow::onReconstructionStarted()
{
    m_reconProgressBar->setValue(0);
    m_reconDisplayLabel->setText("RECONSTRUCTING...");
}

void MainWindow::updateReconProgress(int percentage)
{
    m_reconProgressBar->setValue(percentage);
}

void MainWindow::displayReconResult(const QImage &sliceImage)
{
    m_reconProgressBar->setValue(100);
    m_reconDisplayLabel->setPixmap(QPixmap::fromImage(sliceImage).scaled(
        m_reconDisplayLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));
}

void MainWindow::onSystemStatusUpdated(const SystemStatus &status)
{
    if (!status.xrayStatus.isConnected) {
        m_statusLabel->setText("X-Ray Source Disconnected");
    } else {
        QString xrayStatus = status.xrayStatus.isXRayOn ? "X-Ray ON" : "X-Ray OFF";
        m_statusLabel->setText(
            QString("X-Ray: %1 | Voltage: %2 kV | Temp: %3 °C")
                .arg(xrayStatus)
                .arg(status.xrayStatus.currentVoltage_kV, 0, 'f', 1)
                .arg(status.xrayStatus.tubeTemperature, 0, 'f', 1)
            );
    }

    Log(QString("Motion Stage at %1 degrees.").arg(status.motionStageStatus.currentPosition));

    Log(QString("Detector frames acquired: %1").arg(status.detectorStatus.framesAcquired));
}


void ScanController::requestPause()
{
    Log("UI requested to pause scan.");
    emit pauseRequested();
}

void ScanController::requestResume()
{
    Log("UI requested to resume scan.");
    emit resumeRequested();
}


void MainWindow::updateScanProgress(const ScanProgress &progress)
{
    if (progress.totalProjections > 0) {
        m_scanProgressBar->setRange(0, 100);
        m_scanProgressBar->setValue(progress.percentComplete);

        QTime remainingTime = QTime::fromMSecsSinceStartOfDay(static_cast<int>(progress.estimatedTimeRemaining_ms));
        QString statusText = QString("Scanning: %1/%2 frames | Angle: %3° | Remaining: %4")
                                 .arg(progress.currentProjection)
                                 .arg(progress.totalProjections)
                                 .arg(progress.currentAngle, 0, 'f', 1)
                                 .arg(remainingTime.toString("mm:ss"));

        m_statusLabel->setText(statusText);
    }
}


void MainWindow::onDoorButtonClicked()
{
    bool is_checked = m_doorButton->isChecked();
    if (is_checked) {
        m_doorButton->setText("Door Closed");
    } else {
        m_doorButton->setText("DOOR OPEN");
    }
    m_safetyService->setDoorState(is_checked);
}
