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
        return false;
    }

    Log("DummySource: connect() called successfully.");
    return true;
}

void DummySource::disconnect()
{
    qDebug() << "DummySource: Disconnected.";
}
bool DummySource::setVoltage(double kv)
{
    Log(QString("DummySource: Setting voltage to %1 kV.").arg(kv));
    return true;
}

bool DummySource::turnOn(int ) {
    Log(QString("DummySource: X-Ray ON."));
    return true;
}

bool DummySource::turnOff() {
    Log("DummySource: X-Ray OFF.");
    return true;
}
