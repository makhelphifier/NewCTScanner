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
    layout->addWidget(m_statusLabel);
    this->setCentralWidget(centralWidget);
    this->setWindowTitle("New CT Scanner Control");

    connect(m_startXRayButton, &QPushButton::clicked,
            m_scanController, &ScanController::requestScan);

    connect(m_stopButton, &QPushButton::clicked,
            m_scanController, &ScanController::requestStop);

    connect(m_scanController, &ScanController::statusUpdated,
            this, &MainWindow::updateStatus);
    connect(m_scanController, &ScanController::statusUpdated,
            this, &MainWindow::updateStatus);
    connect(m_scanController, &ScanController::stateChanged,
            this, &MainWindow::onStateChanged);
    // 6. 启动线程
    m_workerThread->start();
}



MainWindow::~MainWindow()
{
    // 停止线程
    m_workerThread->quit();
    // 等待线程完全结束
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
