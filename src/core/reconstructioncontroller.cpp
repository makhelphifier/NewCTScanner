#include "reconstructioncontroller.h"
#include "common/Logger.h"
#include <QThread>
#include <QPainter>
#include <QDir>

ReconstructionController::ReconstructionController(QObject *parent) : QObject(parent) {}

void ReconstructionController::startReconstruction(const QString &inputDataPath)
{
    Log("ReconstructionController: Starting reconstruction process.");
    emit statusMessage("Reconstruction started...");

    // --- 模拟耗时的重建过程 ---
    // 在真实应用中，这里会调用复杂的算法
    int totalSteps = 100;
    for (int i = 0; i <= totalSteps; ++i) {
        // 检查线程是否被中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            Log("ReconstructionController: Reconstruction was interrupted.");
            emit statusMessage("Reconstruction cancelled.");
            return;
        }

        // 模拟计算延迟
        QThread::msleep(50); // 每一步耗时50毫秒，总共5秒
        emit reconstructionProgress(i);
    }
    // --- 模拟结束 ---

    Log("ReconstructionController: Reconstruction finished.");

    // 创建一张模拟的重建结果图像
    QImage resultSlice(256, 256, QImage::Format_RGB888);
    resultSlice.fill(Qt::black);
    QPainter painter(&resultSlice);
    painter.setPen(Qt::green);
    painter.setFont(QFont("Arial", 20));
    painter.drawText(resultSlice.rect(), Qt::AlignCenter, "Reconstructed\nSlice");
    painter.end();

    emit statusMessage("Reconstruction finished successfully.");
    emit reconstructionFinished(resultSlice);
}
