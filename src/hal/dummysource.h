#ifndef DUMMYSOURCE_H
#define DUMMYSOURCE_H

#include "hal/IXRaySource.h"
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include "common/HardwareStatus.h"

class DummySource :  public IXRaySource {
    Q_OBJECT
public:
    explicit DummySource(QObject *parent = nullptr);
    ~DummySource();
public slots:

    bool connect() override;
    void disconnect() override ;
    bool setVoltage(double kv) override;
    bool turnOn(int numberOfFrames) override;
    bool turnOff() override;

private slots:
    void pollStatus();

private:
    QTimer* m_acquisitionTimer;
    int m_imageCounter;
    int m_totalFrames;
    QTimer* m_statusTimer;
    XRaySourceStatus m_currentStatus;

};

#endif // DUMMYSOURCE_H
