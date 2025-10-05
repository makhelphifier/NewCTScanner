#include "hardwareservice.h"
#include "hal/dummysource.h"
#include "common/Logger.h"
#include "hal/dummydetector.h"
#include "hal/dummymotionstage.h"


HardwareService::HardwareService(QObject *parent)
    : QObject(parent), m_xraySource(nullptr), m_hardwareThread(nullptr)
{
    Log("HardwareService created.");
}

HardwareService::~HardwareService()
{
    if (m_hardwareThread) {
        m_hardwareThread->quit();
        m_hardwareThread->wait();
    }
    Log("HardwareService destroyed.");
}

void HardwareService::init()
{
    m_hardwareThread = new QThread(this);

    m_xraySource = new DummySource();
    m_detector = new DummyDetector();
    m_motionStage = new DummyMotionStage();

    m_xraySource->moveToThread(m_hardwareThread);
    m_detector->moveToThread(m_hardwareThread);
    m_motionStage->moveToThread(m_hardwareThread);

    connect(m_hardwareThread, &QThread::finished, m_xraySource, &QObject::deleteLater);
    connect(m_hardwareThread, &QThread::finished, m_detector, &QObject::deleteLater);
    connect(m_hardwareThread, &QThread::finished, m_motionStage, &QObject::deleteLater);

    m_hardwareThread->start();
    Log("Hardware thread started.");


    connect(m_hardwareThread, &QThread::finished, m_xraySource, &QObject::deleteLater);

    m_hardwareThread->start();
    Log("Hardware thread started.");

    bool connected = QMetaObject::invokeMethod(m_xraySource, "connect", Qt::QueuedConnection);
    if (!connected) {
        Log("ERROR: Failed to invoke connect method on X-Ray source.");
    }
}

IXRaySource* HardwareService::xraySource() const
{
    return m_xraySource;
}
IDetector* HardwareService::detector() const { return m_detector; }
IMotionStage* HardwareService::motionStage() const { return m_motionStage; }
