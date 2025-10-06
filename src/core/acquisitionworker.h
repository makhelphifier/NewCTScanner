#ifndef ACQUISITIONWORKER_H
#define ACQUISITIONWORKER_H

#include <QObject>
#include <atomic>
#include "core/framebuffer.h"
#include "common/ScanParameters.h"

class HardwareService;
class ScanParameters;

class AcquisitionWorker : public QObject
{
    Q_OBJECT

public:
    explicit AcquisitionWorker(HardwareService* hardwareService,
                               FrameBuffer* frameBuffer,
                               QObject *parent = nullptr);

public slots:
    // 开始执行采集循环
    void doAcquisition(const ScanParameters &params);
    // 请求停止采集
    void stop();

signals:
    void finished();
    void error(const QString &message);
    void newFrameProduced(FramePtr frame); // 每产生一帧就发出此信号
    void progressUpdated(int current, int total);

private slots:
    void onMoveFinished(bool success);
    void onFrameAcquired(const QImage &image);

private:
    void startNextStep();

    HardwareService* m_hardwareService;
    FrameBuffer* m_frameBuffer;
    std::atomic<bool> m_stopRequested;

    ScanParameters m_params;
    int m_currentFrame;
};

#endif // ACQUISITIONWORKER_H
