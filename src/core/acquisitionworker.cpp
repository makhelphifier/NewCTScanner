#include "acquisitionworker.h"
#include "core/hardwareservice.h"
#include "common/ScanParameters.h"
#include "common/Logger.h"
#include "hal/IDetector.h"

AcquisitionWorker::AcquisitionWorker(HardwareService *hardwareService, FrameBuffer *frameBuffer, QObject *parent)
    : QObject(parent),
    m_hardwareService(hardwareService),
    m_frameBuffer(frameBuffer),
    m_stopRequested(false),
    m_currentFrame(0),
    m_totalFrames(0)
{
}

void AcquisitionWorker::doAcquisition(const ScanParameters &params)
{
    Log("AcquisitionWorker: Starting acquisition loop.");
    m_stopRequested = false;
    m_currentFrame = 0;
    m_params = params;
    m_totalFrames = params.frameCount;

    connect(m_hardwareService->detector(), &IDetector::newImageReady, this, &AcquisitionWorker::onFrameAcquired);

    m_hardwareService->turnOnXRay();
    acquireNextFrame();
}

void AcquisitionWorker::stop()
{
    Log("AcquisitionWorker: Stop requested.");
    m_stopRequested = true;
}

void AcquisitionWorker::acquireNextFrame()
{
    if (m_stopRequested || m_currentFrame >= m_totalFrames) {
        m_hardwareService->turnOffXRay();
        Log("AcquisitionWorker: Acquisition finished.");
        disconnect(m_hardwareService->detector(), &IDetector::newImageReady, this, &AcquisitionWorker::onFrameAcquired);
        emit finished();
        return;
    }

    emit progressUpdated(m_currentFrame + 1, m_totalFrames);

    m_hardwareService->acquireFrame();
}

void AcquisitionWorker::onFrameAcquired(const QImage &image)
{
    if (m_stopRequested) return;

    m_currentFrame++;

    FramePtr newFrame = std::make_shared<Frame>();
    newFrame->image = image;
    newFrame->frameNumber = m_currentFrame;
    newFrame->timestamp = QDateTime::currentDateTime();

    if (!m_frameBuffer->push(newFrame)) {
        Log("Warning: FrameBuffer is full, a frame was dropped.");
    }

    emit newFrameProduced(newFrame);

    acquireNextFrame();
}
