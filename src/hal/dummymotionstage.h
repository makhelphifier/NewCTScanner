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
private:
    MotionStageStatus m_currentStatus;
};

#endif // DUMMYMOTIONSTAGE_H
