#include "DummyMotionStage.h"
#include "common/Logger.h"

DummyMotionStage::DummyMotionStage(QObject *parent) : IMotionStage(parent)
{
    Log("DummyMotionStage created.");
}

void DummyMotionStage::moveTo(double position)
{
    Log(QString("DummyMotionStage: Moving to %1 degrees.").arg(position));
    QTimer::singleShot(100, this, [this](){
        Log(QString("DummyMotionStage: Move finished."));
        emit moveFinished(true);
    });
}

void DummyMotionStage::reset()
{
    Log("DummyMotionStage: Resetting position.");
    QTimer::singleShot(500, this, [this](){
        Log("DummyMotionStage: Reset finished.");
        emit moveFinished(true);
    });
}
