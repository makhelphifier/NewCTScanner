// src/core/dataacquisitionservice.cpp

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
    stop();
    Log("DataAcquisitionService destroyed.");
}

void DataAcquisitionService::start(const ScanParameters &params, const QString &saveDirectory, const QString &savePrefix)
{
    if (m_saverThread) {
        stop();
    }
    m_currentParams = params;
    Log("DataAcquisitionService: Starting DataSaver consumer...");

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
    });

    m_saverThread->start();
}

void DataAcquisitionService::acquireSingleFrame()
{
    if (m_workerThread) {
        Log("Warning: Acquisition already in progress.");
        return;
    }
    Log("DataAcquisitionService: Starting single frame acquisition...");

    m_workerThread = new QThread(this);
    m_worker = new AcquisitionWorker(m_hardwareService, m_frameBuffer.get());
    m_worker->moveToThread(m_workerThread);

    ScanParameters singleFrameParams = m_currentParams;
    singleFrameParams.frameCount = 1;

    connect(m_workerThread, &QThread::started, m_worker, [=](){ m_worker->doAcquisition(singleFrameParams); });
    connect(m_worker, &AcquisitionWorker::finished, m_workerThread, &QThread::quit);
    connect(m_worker, &AcquisitionWorker::finished, m_worker, &AcquisitionWorker::deleteLater);
    connect(m_workerThread, &QThread::finished, this, [=](){
        m_workerThread = nullptr;
        m_worker = nullptr;
        Log("Single-frame acquisition thread finished.");
        emit acquisitionWorkerFinished();
    });

    connect(m_worker, &AcquisitionWorker::newFrameProduced, this, &DataAcquisitionService::onNewFrameProduced);
    connect(m_worker, &AcquisitionWorker::progressUpdated, this, &DataAcquisitionService::scanProgress);

    m_workerThread->start();
}

void DataAcquisitionService::onNewFrameProduced(FramePtr frame)
{
    emit newRawFrameReady(frame);
}


void DataAcquisitionService::stop()
{
    if (m_worker) {
        m_worker->stop();
    }
    if (m_dataSaver) {
        m_dataSaver->stop();
    }
}
