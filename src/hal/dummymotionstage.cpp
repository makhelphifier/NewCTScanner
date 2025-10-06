#include "DummyMotionStage.h"
#include "common/Logger.h"

DummyMotionStage::DummyMotionStage(QObject *parent) : IMotionStage(parent)
{
    Log("DummyMotionStage created.");
    m_currentStatus.isConnected = true;
    m_currentStatus.isMoving = false;
    m_currentStatus.currentPosition = 0.0;
}

void DummyMotionStage::moveTo(double position)
{
    Log(QString("DummyMotionStage: Moving to %1 degrees.").arg(position));
    m_currentStatus.isMoving = true;
    emit statusChanged(m_currentStatus);

    QTimer::singleShot(100, this, [this, position](){
        Log(QString("DummyMotionStage: Move finished."));
        m_currentStatus.isMoving = false;
        m_currentStatus.currentPosition = position;
        emit statusChanged(m_currentStatus);
        emit moveFinished(true);
    });
}

void DummyMotionStage::reset()
{
    Log("DummyMotionStage: Resetting position.");
    m_currentStatus.isMoving = true;
    emit statusChanged(m_currentStatus);

    QTimer::singleShot(500, this, [this](){
        Log("DummyMotionStage: Reset finished.");
        m_currentStatus.isMoving = false;
        m_currentStatus.currentPosition = 0.0;
        emit statusChanged(m_currentStatus);
        emit moveFinished(true);
    });
}
