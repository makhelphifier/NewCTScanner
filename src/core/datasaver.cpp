#include "datasaver.h"
#include "common/Logger.h"
#include <QDir>
#include <QTimer>

DataSaver::DataSaver(QObject *parent)
    : QObject(parent), m_fileCounter(0), m_isSaving(false) {}

void DataSaver::startSaving(const QString &directory, const QString &prefix)
{
    Log(QString("DataSaver: Starting save session. Dir: %1, Prefix: %2").arg(directory).arg(prefix));
    m_directory = directory;
    m_prefix = prefix;
    m_fileCounter = 0;
    m_isSaving = true;

    QDir dir(m_directory);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QTimer::singleShot(0, this, &DataSaver::processImageQueue);
}

void DataSaver::stopSaving()
{
    Log("DataSaver: Stopping save session.");
    m_isSaving = false;
}

void DataSaver::queueImage(const QImage &image)
{
    if (!m_isSaving) return;

    QMutexLocker locker(&m_queueMutex);
    m_imageQueue.enqueue(image);

    QTimer::singleShot(0, this, &DataSaver::processImageQueue);
}

void DataSaver::processImageQueue()
{
    QMutexLocker locker(&m_queueMutex);
    if (m_imageQueue.isEmpty() || !m_isSaving) {
        return;
    }

    QImage image = m_imageQueue.dequeue();
    locker.unlock();

    QString filePath = QString("%1/%2_%3.png")
                           .arg(m_directory)
                           .arg(m_prefix)
                           .arg(m_fileCounter++, 4, 10, QChar('0')); // e.g., prefix_0000.png

    if (image.save(filePath, "PNG")) {
        // Log(QString("Saved image: %1").arg(filePath));
    } else {
        Log(QString("Error: Failed to save image %1").arg(filePath));
    }

    if(m_isSaving && !m_imageQueue.isEmpty()){
        QTimer::singleShot(0, this, &DataSaver::processImageQueue);
    }
}
