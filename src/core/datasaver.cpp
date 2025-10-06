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
        // 尝试从缓冲区弹出一帧，最多等待100ms
        FramePtr frame = m_sourceBuffer->pop(100);

        if (frame) { // 如果成功获取到帧
            QString filePath = QString("%1/%2_%3.png")
                                   .arg(directory)
                                   .arg(prefix)
                                   .arg(++fileCounter, 4, 10, QChar('0'));

            if (!frame->image.save(filePath, "PNG")) {
                Log(QString("Error: Failed to save image %1").arg(filePath));
            }
        } else {
            // 如果超时了还没拿到 frame，检查下是不是采集已经结束并且缓冲区也空了
            if (m_sourceBuffer->isEmpty()) {
                // 可以加一个更复杂的逻辑，比如等待worker线程结束后再退出
                // 为简化，我们这里只做短暂sleep
                QThread::msleep(10);
            }
        }
    }

    Log("DataSaver: Consuming loop finished.");
    emit finished();
}
