#include "dummysource.h"
#include <QDebug>
#include <QPainter>
#include "common/Logger.h"
#include <QRandomGenerator>
#include <QThread>
#include <QString>


DummySource::DummySource(QObject *parent)
    : IXRaySource()
{
    m_statusTimer = new QTimer(this);
    QObject::connect(m_statusTimer, &QTimer::timeout, this, &DummySource::pollStatus);
    m_statusTimer->start(500);

    m_currentStatus.isConnected = false;
    m_currentStatus.isXRayOn = false;
}

DummySource::~DummySource()
{
}

bool DummySource::connect()
{
    if (QRandomGenerator::global()->bounded(2) == 0) {
        QString errorMsg = "Failed to connect to X-Ray source (Simulated Error).";
        Log("ERROR: " + errorMsg);
        emit hardwareError(errorMsg);
        m_currentStatus.isConnected = false;
        return false;
    }

    Log("DummySource: connect() called successfully.");
    m_currentStatus.isConnected = true;
    return true;
}
void DummySource::disconnect()
{
    qDebug() << "DummySource: Disconnected.";
}
bool DummySource::setVoltage(double kv)
{
    Log(QString("DummySource: Setting voltage to %1 kV.").arg(kv));
    m_currentStatus.currentVoltage_kV = kv;
    return true;
}

bool DummySource::turnOn(int ) {
    Log(QString("DummySource: X-Ray ON."));
    m_currentStatus.isXRayOn = true;
    return true;
}

bool DummySource::turnOff() {
    Log("DummySource: X-Ray OFF.");
    m_currentStatus.isXRayOn = false;
    return true;
}

void DummySource::pollStatus()
{
    if(m_currentStatus.isConnected) {
        m_currentStatus.tubeTemperature = 35.5 + QRandomGenerator::global()->bounded(5.0);
    }
    emit statusChanged(m_currentStatus);
}
