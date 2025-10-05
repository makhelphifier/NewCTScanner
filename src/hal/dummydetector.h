#ifndef DUMMYDETECTOR_H
#define DUMMYDETECTOR_H

#include "IDetector.h"

class DummyDetector : public IDetector
{
    Q_OBJECT
public:
    explicit DummyDetector(QObject *parent = nullptr);

public slots:
    void acquireFrame() override;

private:
    int m_imageCounter;
};

#endif // DUMMYDETECTOR_H
