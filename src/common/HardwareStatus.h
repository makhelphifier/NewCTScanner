#ifndef HARDWARESTATUS_H
#define HARDWARESTATUS_H

#include <QObject>
#include <QMetaType>

struct XRaySourceStatus {
    bool isConnected = false;
    bool isXRayOn = false;
    double currentVoltage_kV = 0.0;
    double currentCurrent_uA = 0.0;
    double tubeTemperature = 0.0;
};

struct MotionStageStatus {
    bool isConnected = false;
    bool isMoving = false;
    double currentPosition = 0.0;
};

struct DetectorStatus {
    bool isConnected = false;
    bool isAcquiring = false;
    int framesAcquired = 0;
};

struct SystemStatus {
    XRaySourceStatus xrayStatus;
    MotionStageStatus motionStageStatus;
    DetectorStatus detectorStatus;
};

Q_DECLARE_METATYPE(XRaySourceStatus)
Q_DECLARE_METATYPE(MotionStageStatus)
Q_DECLARE_METATYPE(DetectorStatus)
Q_DECLARE_METATYPE(SystemStatus)

#endif // HARDWARESTATUS_H
