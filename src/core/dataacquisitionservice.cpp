#include "dataacquisitionservice.h"
#include "core/FrameBuffer.h"
#include "core/AcquisitionWorker.h"
#include "core/DataSaver.h"
#include "core/hardwareservice.h"
#include "common/ScanParameters.h"
#include "common/Logger.h"

DataAcquisitionService::DataAcquisitionService(HardwareService* hardwareService, QObject *parent)
    : QObject(parent),
    m_hardwareService(hardwareService),
    m_frameBuffer(std::make_unique<FrameBuffer>()),
    m_workerThread(nullptr),
    m_worker(nullptr),
    m_saverThread(nullptr),
    m_dataSaver(nullptr)
{
    Log("DataAcquisitionService created.");
}

DataAcquisitionService::~DataAcquisitionService()
{
    stop(); // 确保在析构时所有东西都已停止
    Log("DataAcquisitionService destroyed.");
}

void DataAcquisitionService::start(const ScanParameters &params, const QString &saveDirectory, const QString &savePrefix)
{
    if (m_workerThread) { // 如果正在运行，先停止
        stop();
    }

    Log("DataAcquisitionService: Starting pipeline...");

    // 1. 创建并启动生产者 (AcquisitionWorker)
    m_workerThread = new QThread(this);
    m_worker = new AcquisitionWorker(m_hardwareService, m_frameBuffer.get());
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, [=](){ m_worker->doAcquisition(params); });
    connect(m_worker, &AcquisitionWorker::finished, m_workerThread, &QThread::quit);
    connect(m_worker, &AcquisitionWorker::finished, m_worker, &AcquisitionWorker::deleteLater);
    connect(m_workerThread, &QThread::finished, this, [=](){
        m_workerThread = nullptr;
        m_worker = nullptr;
        Log("Acquisition thread finished and cleaned up.");
        // 当生产者停止后，也通知消费者停止
        if (m_dataSaver) m_dataSaver->stop();
    });

    // 转发信号
    connect(m_worker, &AcquisitionWorker::newFrameProduced, this, &DataAcquisitionService::newFrameReady);
    connect(m_worker, &AcquisitionWorker::progressUpdated, this, &DataAcquisitionService::scanProgress);


    // 2. 创建并启动消费者 (DataSaver)
    m_saverThread = new QThread(this);
    m_dataSaver = new DataSaver();
    m_dataSaver->moveToThread(m_saverThread);

    connect(m_saverThread, &QThread::started, m_dataSaver, [=](){
        m_dataSaver->startConsuming(saveDirectory, savePrefix, m_frameBuffer.get());
    });
    connect(m_dataSaver, &DataSaver::finished, m_saverThread, &QThread::quit);
    connect(m_dataSaver, &DataSaver::finished, m_dataSaver, &DataSaver::deleteLater);
    connect(m_saverThread, &QThread::finished, this, [=](){
        m_saverThread = nullptr;
        m_dataSaver = nullptr;
        Log("Saver thread finished and cleaned up.");
        // 两个线程都结束后，可以认为整个采集过程结束了
        if(m_workerThread == nullptr) {
            emit acquisitionFinished();
        }
    });

    // 启动线程
    m_saverThread->start();
    m_workerThread->start();
}

void DataAcquisitionService::stop()
{
    if (m_worker) {
        m_worker->stop(); // 请求生产者停止
    }
    // 消费者将在生产者结束后自动停止
}
