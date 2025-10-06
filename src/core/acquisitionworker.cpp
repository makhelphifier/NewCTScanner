#include "acquisitionworker.h"
#include "core/hardwareservice.h"
#include "common/ScanParameters.h"
#include "common/Logger.h"
#include "hal/IDetector.h"
#include "hal/IMotionStage.h"

AcquisitionWorker::AcquisitionWorker(HardwareService *hardwareService, FrameBuffer *frameBuffer, QObject *parent)
    : QObject(parent),
    m_hardwareService(hardwareService),
    m_frameBuffer(frameBuffer),
    m_stopRequested(false),
    m_currentFrame(0)
{
}

void AcquisitionWorker::doAcquisition(const ScanParameters &params)
{
    Log("AcquisitionWorker: Starting acquisition loop.");
    m_stopRequested = false;
    m_currentFrame = 0;
    m_params = params;

    // 连接硬件信号到这个工作线程的槽
    connect(m_hardwareService->motionStage(), &IMotionStage::moveFinished, this, &AcquisitionWorker::onMoveFinished);
    connect(m_hardwareService->detector(), &IDetector::newImageReady, this, &AcquisitionWorker::onFrameAcquired);

    m_hardwareService->turnOnXRay();
    startNextStep();
}

void AcquisitionWorker::stop()
{
    Log("AcquisitionWorker: Stop requested.");
    m_stopRequested = true;
}

void AcquisitionWorker::startNextStep()
{
    if (m_stopRequested || m_currentFrame >= m_params.frameCount) {
        m_hardwareService->turnOffXRay();
        Log("AcquisitionWorker: Acquisition finished.");
        // 断开连接，避免多余的信号处理
        disconnect(m_hardwareService->motionStage(), &IMotionStage::moveFinished, this, &AcquisitionWorker::onMoveFinished);
        disconnect(m_hardwareService->detector(), &IDetector::newImageReady, this, &AcquisitionWorker::onFrameAcquired);
        emit finished();
        return;
    }

    emit progressUpdated(m_currentFrame + 1, m_params.frameCount);

    double nextAngle = (360.0 / m_params.frameCount) * m_currentFrame;
    m_hardwareService->moveTo(nextAngle);
}

void AcquisitionWorker::onMoveFinished(bool success)
{
    if (!success) {
        emit error("Motion stage failed to move.");
        stop();
        return;
    }

    if (m_stopRequested) return;

    // 移动到位后，触发一次图像采集
    m_hardwareService->acquireFrame();
}

void AcquisitionWorker::onFrameAcquired(const QImage &image)
{
    if (m_stopRequested) return;

    // 创建一个新的Frame并填充数据
    FramePtr newFrame = std::make_shared<Frame>();
    newFrame->image = image;
    newFrame->frameNumber = m_currentFrame + 1;
    newFrame->timestamp = QDateTime::currentDateTime();
    newFrame->angle = (360.0 / m_params.frameCount) * m_currentFrame;

    // 将Frame推入缓冲区
    if (!m_frameBuffer->push(newFrame)) {
        Log("Warning: FrameBuffer is full, a frame was dropped.");
    }

    emit newFrameProduced(newFrame);

    m_currentFrame++;
    startNextStep(); // 开始下一步（移动到新位置）
}
