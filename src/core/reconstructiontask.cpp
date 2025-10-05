#include "reconstructiontask.h"
#include <QThread>
#include <QPainter>
#include "common/Logger.h"

ReconstructionTask::ReconstructionTask(const QString &inputPath, QObject *parent)
    : QObject(parent), m_inputPath(inputPath)
{
}

void ReconstructionTask::run()
{
    Log("ReconstructionTask: Starting in thread pool.");
    emit statusMessage("Reconstruction started...");

    int totalSteps = 100;
    for (int i = 0; i <= totalSteps; ++i) {
        QThread::msleep(50);
        emit taskProgress(i);
    }

    Log("ReconstructionTask: Finished.");

    QImage resultSlice(256, 256, QImage::Format_RGB888);
    resultSlice.fill(Qt::black);
    QPainter painter(&resultSlice);
    painter.setPen(Qt::yellow);
    painter.setFont(QFont("Arial", 20));
    painter.drawText(resultSlice.rect(), Qt::AlignCenter, "Reconstructed\n (Thread Pool)");
    painter.end();

    emit statusMessage("Reconstruction finished successfully.");
    emit taskFinished(resultSlice);
}
