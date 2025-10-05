#include "reconstructioncontroller.h"
#include "common/Logger.h"
#include <QThread>
#include <QPainter>
#include <QDir>
#include "core/reconstructiontask.h"

#include <QThreadPool>

ReconstructionController::ReconstructionController(QObject *parent) : QObject(parent) {}

void ReconstructionController::startReconstruction(const QString &inputDataPath)
{
    Log("ReconstructionController: Creating and submitting a new ReconstructionTask.");

    ReconstructionTask *task = new ReconstructionTask(inputDataPath);

    connect(task, &ReconstructionTask::taskProgress, this, &ReconstructionController::reconstructionProgress, Qt::QueuedConnection);
    connect(task, &ReconstructionTask::taskFinished, this, &ReconstructionController::reconstructionFinished, Qt::QueuedConnection);
    connect(task, &ReconstructionTask::statusMessage, this, &ReconstructionController::statusMessage, Qt::QueuedConnection);

    connect(task, &ReconstructionTask::taskFinished, task, &ReconstructionTask::deleteLater);

    QThreadPool::globalInstance()->start(task);
}
