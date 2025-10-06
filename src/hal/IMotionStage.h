#ifndef IMOTIONSTAGE_H
#define IMOTIONSTAGE_H

#include <QObject>
#include "common/HardwareStatus.h"

class IMotionStage : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IMotionStage() {}
public slots:

    virtual void moveTo(double position) = 0;
    virtual void reset() = 0;
    virtual void setSpeed(double speed_deg_per_sec) = 0;

signals:
    void moveFinished(bool success);
    void errorOccurred(const QString &errorMessage);
    void statusChanged(const MotionStageStatus &newStatus);

};

#endif // IMOTIONSTAGE_H
