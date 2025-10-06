#ifndef IDETECTOR_H
#define IDETECTOR_H

#include <QObject>
#include <QImage>
#include "common/HardwareStatus.h"

class IDetector : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IDetector() {}
public slots:

    virtual void acquireFrame() = 0;

signals:
    void newImageReady(const QImage &image);
    void acquisitionFinished();
    void errorOccurred(const QString &errorMessage);
    void statusChanged(const DetectorStatus &newStatus);
};

#endif // IDETECTOR_H
