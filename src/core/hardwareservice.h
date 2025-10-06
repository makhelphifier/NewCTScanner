#ifndef HARDWARESERVICE_H
#define HARDWARESERVICE_H

#include <QObject>
#include <QThread>

class IXRaySource;
class IDetector;
class IMotionStage;

class HardwareService : public QObject
{
    Q_OBJECT
public:
    explicit HardwareService(QObject *parent = nullptr);
    ~HardwareService();

    void init();
public slots:
    void setVoltage(double kv);
    void turnOnXRay();
    void turnOffXRay();
    void moveTo(double position);
    void acquireFrame();

    IXRaySource* xraySource() const;
    IDetector* detector() const;
    IMotionStage* motionStage() const;


private:
    IXRaySource* m_xraySource;
    QThread* m_hardwareThread;
    IDetector* m_detector;
    IMotionStage* m_motionStage;
};

#endif // HARDWARESERVICE_H
