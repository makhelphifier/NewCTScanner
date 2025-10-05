#include "mainwindow.h"
#include "core/scancontroller.h"
#include <QPushButton>
#include <QVBoxLayout>
#include "common/Logger.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QFileDialog> // 别忘了包含


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_workerThread = new QThread(this);
    m_scanController = new ScanController();

    m_scanController->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_scanController, &ScanController::init);

    connect(m_workerThread, &QThread::finished, m_scanController, &QObject::deleteLater);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    m_startXRayButton = new QPushButton("Start X-Ray", this);
    m_stopButton = new QPushButton("Stop/Reset", this);
    layout->addWidget(m_stopButton);
    layout->addWidget(m_startXRayButton);
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
    formLayout->addRow("Voltage:", m_voltageSpinBox);
    formLayout->addRow("Current:", m_currentSpinBox);
    formLayout->addRow("Frame Count:", m_frameCountSpinBox);
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
    layout->addLayout(configLayout);

    layout->addLayout(formLayout);
    m_imageDisplayLabel = new QLabel(this);
    m_imageDisplayLabel->setMinimumSize(256, 256);
    m_imageDisplayLabel->setStyleSheet("background-color: black; border: 1px solid gray;");
    m_imageDisplayLabel->setAlignment(Qt::AlignCenter);
    m_imageDisplayLabel->setText("IMAGE DISPLAY");
    layout->addWidget(m_imageDisplayLabel);



    // 在主布局的某个合适位置（例如，图像显示区旁边或下面）
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

    // 你可能需要一个主水平布局来并列显示采集图像和重建图像
    // 例如： QHBoxLayout* displayLayout = new QHBoxLayout();
    // displayLayout->addWidget(m_imageDisplayLabel);
    // displayLayout->addLayout(reconLayout);
    // layout->addLayout(displayLayout);
    layout->addLayout(reconLayout); // 简单起见，先垂直添加


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

    connect(m_scanController, &ScanController::statusUpdated,
            this, &MainWindow::updateStatus);

    connect(m_scanController, &ScanController::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(m_voltageSpinBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::onParametersChanged);
    connect(m_currentSpinBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::onParametersChanged);
    connect(m_frameCountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::onParametersChanged);

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
    connect(m_scanController, &ScanController::scanProgress, this, &MainWindow::updateScanProgress);
    connect(m_loadConfigButton, &QPushButton::clicked, this, &MainWindow::onLoadConfig);
    connect(m_saveConfigButton, &QPushButton::clicked, this, &MainWindow::onSaveConfig);
    connect(m_scanController, &ScanController::reconstructionStarted, this, &MainWindow::onReconstructionStarted);
    connect(m_scanController, &ScanController::reconstructionProgress, this, &MainWindow::updateReconProgress);
    connect(m_scanController, &ScanController::reconstructionFinished, this, &MainWindow::displayReconResult);
    // 连接ScanController的配置加载信号到UI的更新槽
    connect(m_scanController, &ScanController::configurationLoaded, this, &MainWindow::applyLoadedParameters);


    // 发送一次初始路径
    onSavePathChanged();

    // 发送第一条日志
    Log("Application started. UI is ready.");
    onParametersChanged();
    m_workerThread->start();
}



MainWindow::~MainWindow()
{
    m_workerThread->quit();
    m_workerThread->wait();
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
        m_scanProgressBar->setValue(0);
        break;

    case ScanController::StatePreparing:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        setParametersEnabled(false);
        break;

    case ScanController::StateScanning:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        setParametersEnabled(false);
        m_scanProgressBar->setValue(0);
        break;

    case ScanController::StateError:
        m_startXRayButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        setParametersEnabled(false);
        break;
    }
}
void MainWindow::onParametersChanged()
{
    ScanParameters params;
    params.voltage = m_voltageSpinBox->value();
    params.current = m_currentSpinBox->value();
    params.frameCount = m_frameCountSpinBox->value();
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
}


void MainWindow::updateScanProgress(int current, int total)
{
    if (total > 0) {
        m_scanProgressBar->setRange(0, total);
        m_scanProgressBar->setValue(current);
    }
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
    // 更新UI上的所有控件
    m_voltageSpinBox->setValue(params.voltage);
    m_currentSpinBox->setValue(params.current);
    m_frameCountSpinBox->setValue(params.frameCount);

    // 确保更新也通知到后台
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
