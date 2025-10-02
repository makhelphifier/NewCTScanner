#include "mainwindow.h"
#include "core/scancontroller.h"
#include <QPushButton>
#include <QVBoxLayout>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_workerThread = new QThread(this);
    m_scanController = new ScanController();

    m_scanController->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_scanController, &QObject::deleteLater);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    m_startXRayButton = new QPushButton("Start X-Ray", this);
    m_stopButton = new QPushButton("Stop/Reset", this);
    layout->addWidget(m_stopButton);
    layout->addWidget(m_startXRayButton);
    m_statusLabel = new QLabel("Initializing...", this);
    m_statusLabel->setAlignment(Qt::AlignCenter); // 居中显示
    QFormLayout *formLayout = new QFormLayout();
    m_voltageSpinBox = new QDoubleSpinBox(this);
    m_voltageSpinBox->setRange(20.0, 225.0);
    m_voltageSpinBox->setValue(100.0);
    m_voltageSpinBox->setSuffix(" kV");

    m_currentSpinBox = new QDoubleSpinBox(this);
    m_currentSpinBox->setRange(10.0, 1000.0);
    m_currentSpinBox->setValue(50.0);
    m_currentSpinBox->setSuffix(" uA");

    formLayout->addRow("Voltage:", m_voltageSpinBox);
    formLayout->addRow("Current:", m_currentSpinBox);

    layout->addLayout(formLayout);
    m_imageDisplayLabel = new QLabel(this);
    m_imageDisplayLabel->setMinimumSize(256, 256);
    m_imageDisplayLabel->setStyleSheet("background-color: black; border: 1px solid gray;");
    m_imageDisplayLabel->setAlignment(Qt::AlignCenter);
    m_imageDisplayLabel->setText("IMAGE DISPLAY");
    layout->addWidget(m_imageDisplayLabel);
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
    connect(this, &MainWindow::parametersChanged,
            m_scanController, &ScanController::updateParameters);
    connect(m_scanController, &ScanController::newProjectionImage,
            this, &MainWindow::updateImage, Qt::QueuedConnection);
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
    switch (newState)
    {
    case ScanController::StateIdle:
        m_startXRayButton->setEnabled(true);  // 空闲时，可以开始
        m_stopButton->setEnabled(false); // 空闲时，无需停止
        break;
    case ScanController::StatePreparing:
        m_startXRayButton->setEnabled(false); // 准备中，不能重复开始
        m_stopButton->setEnabled(true);  // 准备中，可以停止
        break;
    case ScanController::StateScanning:
        m_startXRayButton->setEnabled(false); // 扫描中，不能重复开始
        m_stopButton->setEnabled(true);  // 扫描中，可以停止
        break;
    }
}
void MainWindow::onParametersChanged()
{
    ScanParameters params;
    params.voltage = m_voltageSpinBox->value();
    params.current = m_currentSpinBox->value();
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
