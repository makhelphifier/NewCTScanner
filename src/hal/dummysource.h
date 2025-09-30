#ifndef DUMMYSOURCE_H
#define DUMMYSOURCE_H

#include "hal/IXRaySource.h"
#include <QObject>
#include <QDebug>

// 模拟的射线源，用于开发和测试
class DummySource : public QObject, public IXRaySource {
    Q_OBJECT
public:
    explicit DummySource(QObject *parent = nullptr) : QObject(parent) {}

    bool connect() override { qDebug() << "DummySource: Connected."; return true; }
    void disconnect() override { qDebug() << "DummyDummySource: Disconnected."; }
    bool setVoltage(double kv) override { qDebug() << "DummySource: Voltage set to" << kv << "kV"; return true; }
    bool turnOn() override { qDebug() << "DummySource: X-Ray ON."; return true; }
    bool turnOff() override { qDebug() << "DummySource: X-Ray OFF."; return true; }
};

#endif // DUMMYSOURCE_H
