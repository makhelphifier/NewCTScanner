#ifndef DUMMYDETECTOR_H
#define DUMMYDETECTOR_H

#include "IDetector.h"
#include "common/HardwareStatus.h"


class DummyDetector : public IDetector
{
    Q_OBJECT
public:
    explicit DummyDetector(QObject *parent = nullptr);

public slots:
    void acquireFrame() override;

private:
    int m_imageCounter;
    DetectorStatus m_currentStatus;
};

#endif // DUMMYDETECTOR_H
