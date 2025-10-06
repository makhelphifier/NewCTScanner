
#include "dataacquisitionservice.h"
#include "core/datasaver.h"
#include "common/Logger.h"

DataAcquisitionService::DataAcquisitionService(QObject *parent) : QObject(parent)
{
    m_saverThread = new QThread(this);
    m_dataSaver = new DataSaver();
    m_dataSaver->moveToThread(m_saverThread);
    connect(m_saverThread, &QThread::finished, m_dataSaver, &QObject::deleteLater);
    m_saverThread->start();
    Log("DataAcquisitionService created and DataSaver thread started.");
}

DataAcquisitionService::~DataAcquisitionService()
{
    m_saverThread->quit();
    m_saverThread->wait();
    Log("DataAcquisitionService destroyed.");
}

void DataAcquisitionService::startSaving(const QString &directory, const QString &prefix)
{
    QMetaObject::invokeMethod(m_dataSaver, [=]() {
        m_dataSaver->startSaving(directory, prefix);
    }, Qt::QueuedConnection);
}

void DataAcquisitionService::stopSaving()
{
    QMetaObject::invokeMethod(m_dataSaver, [=]() {
        m_dataSaver->stopSaving();
    }, Qt::QueuedConnection);}

void DataAcquisitionService::saveImage(const QImage &image)
{
    QMetaObject::invokeMethod(m_dataSaver, [=]() {
        m_dataSaver->queueImage(image);
    }, Qt::QueuedConnection);
}
