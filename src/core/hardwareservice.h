#ifndef HARDWARESERVICE_H
#define HARDWARESERVICE_H

#include <QObject>
#include <QThread>
#include "common/HardwareStatus.h"


class IXRaySource;
class IDetector;
class IMotionStage;

class HardwareService : public QObject
{
    Q_OBJECT
public:
    explicit HardwareService(QObject *parent = nullptr);
    ~HardwareService();
    SystemStatus getSystemStatus() const;
    void init();
signals:
    void systemStatusUpdated(const SystemStatus &status);
public slots:
    void setVoltage(double kv);
    void turnOnXRay();
    void turnOffXRay();
    void moveTo(double position);
    void acquireFrame();
    void setMotionSpeed(double speed_deg_per_sec);

    IXRaySource* xraySource() const;
    IDetector* detector() const;
    IMotionStage* motionStage() const;

private slots:
    void onXRayStatusChanged(const XRaySourceStatus &status);
    void onDetectorStatusChanged(const DetectorStatus &status);
    void onMotionStageStatusChanged(const MotionStageStatus &status);

private:
    IXRaySource* m_xraySource;
    QThread* m_hardwareThread;
    IDetector* m_detector;
    IMotionStage* m_motionStage;
    SystemStatus m_systemStatus;

};

#endif // HARDWARESERVICE_H
