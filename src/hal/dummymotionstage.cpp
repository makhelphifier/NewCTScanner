#include "DummyMotionStage.h"
#include "common/Logger.h"
#include <cmath>


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

    double distance = std::abs(position - m_currentStatus.currentPosition);
    double speed = (m_speed_deg_per_sec > 0.01) ? m_speed_deg_per_sec : 1.0;
    int duration_ms = static_cast<int>((distance / speed) * 1000.0);



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

void DummyMotionStage::setSpeed(double speed_deg_per_sec)
{
    Log(QString("DummyMotionStage: Setting speed to %1 deg/s.").arg(speed_deg_per_sec));
    m_speed_deg_per_sec = speed_deg_per_sec;
}
