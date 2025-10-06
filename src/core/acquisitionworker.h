#ifndef ACQUISITIONWORKER_H
#define ACQUISITIONWORKER_H

#include <QObject>
#include <atomic>
#include "core/framebuffer.h"
#include "common/ScanParameters.h"
#include "common/ScanProgress.h"
class HardwareService;

class AcquisitionWorker : public QObject
{
    Q_OBJECT

public:
    explicit AcquisitionWorker(HardwareService* hardwareService,
                               FrameBuffer* frameBuffer,
                               QObject *parent = nullptr);

public slots:
    void doAcquisition(const ScanParameters &params);
    void stop();

signals:
    void finished();
    void error(const QString &message);
    void newFrameProduced(FramePtr frame);
    void progressUpdated(const ScanProgress &progress);

private slots:
    void onFrameAcquired(const QImage &image);
    void acquireNextFrame();

private:
    HardwareService* m_hardwareService;
    FrameBuffer* m_frameBuffer;
    std::atomic<bool> m_stopRequested;

    ScanParameters m_params;
    int m_currentFrame;
    int m_totalFrames;
};

#endif
