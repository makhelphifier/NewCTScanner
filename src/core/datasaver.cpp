#include "datasaver.h"
#include "common/Logger.h"
#include <QDir>
#include <QThread>

DataSaver::DataSaver(QObject *parent)
    : QObject(parent), m_stopRequested(false) {}

void DataSaver::stop()
{
    Log("DataSaver: Stop requested.");
    m_stopRequested = true;
}

void DataSaver::startConsuming(const QString &directory, const QString &prefix, FrameBuffer *sourceBuffer)
{
    Log(QString("DataSaver: Starting consuming loop. Dir: %1").arg(directory));
    m_stopRequested = false;
    m_sourceBuffer = sourceBuffer;

    QDir dir(directory);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    int fileCounter = 0;
    while (!m_stopRequested)
    {
        FramePtr frame = m_sourceBuffer->pop(100);

        if (frame) {
            QString filePath = QString("%1/%2_%3.png")
                                   .arg(directory)
                                   .arg(prefix)
                                   .arg(++fileCounter, 4, 10, QChar('0'));

            if (!frame->image.save(filePath, "PNG")) {
                Log(QString("Error: Failed to save image %1").arg(filePath));
            }
        } else {
            if (m_sourceBuffer->isEmpty()) {
                QThread::msleep(10);
            }
        }
    }

    Log("DataSaver: Consuming loop finished.");
    emit finished();
}
