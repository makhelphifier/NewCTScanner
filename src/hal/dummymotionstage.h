#ifndef DUMMYMOTIONSTAGE_H
#define DUMMYMOTIONSTAGE_H

#include "IMotionStage.h"
#include <QTimer>

class DummyMotionStage : public IMotionStage
{
    Q_OBJECT
public:
    explicit DummyMotionStage(QObject *parent = nullptr);

public slots:
    void moveTo(double position) override;
    void reset() override;
    void setSpeed(double speed_deg_per_sec) override;

private:
    MotionStageStatus m_currentStatus;
    double m_speed_deg_per_sec = 1.0;

};

#endif // DUMMYMOTIONSTAGE_H
