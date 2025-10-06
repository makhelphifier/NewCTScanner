#include "systemsafetyservice.h"
#include "common/Logger.h"

SystemSafetyService::SystemSafetyService(QObject *parent)
    : QObject(parent), m_isDoorClosed(true)
{
    m_pollingTimer = new QTimer(this);
    connect(m_pollingTimer, &QTimer::timeout, this, &SystemSafetyService::pollSafetyStatus);
    m_pollingTimer->start(200);
    Log("SystemSafetyService created and is now polling.");
}

void SystemSafetyService::setDoorState(bool is_closed)
{
    if (m_isDoorClosed != is_closed) {
        m_isDoorClosed = is_closed;
        if (!m_isDoorClosed) {
            Log("SAFETY EVENT: Door was opened!");
        } else {
            Log("SAFETY INFO: Door was closed.");
        }
    }
}

void SystemSafetyService::pollSafetyStatus()
{
    if (!m_isDoorClosed) {
        static bool interlockTriggered = false;
        if (!interlockTriggered) {
            emit safetyInterlockTriggered("Emergency Stop: Scanner door was opened during operation!");
            interlockTriggered = true;
        }else {
            interlockTriggered = false;
        }
    }
}
