#include "DummyDetector.h"
#include <QPainter>
#include <QTimer>
#include "common/Logger.h"

DummyDetector::DummyDetector(QObject *parent) : IDetector(parent), m_imageCounter(0)
{
    Log("DummyDetector created.");
    m_currentStatus.isConnected = true;
    m_currentStatus.isAcquiring = false;
    m_currentStatus.framesAcquired = 0;
}
void DummyDetector::acquireFrame()
{
    m_currentStatus.isAcquiring = true;
    emit statusChanged(m_currentStatus);

    m_imageCounter++;
    QTimer::singleShot(50, this, [this](){
        QImage image(256, 256, QImage::Format_RGB888);
        image.fill(Qt::darkCyan);
        QPainter painter(&image);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 40));
        painter.drawText(image.rect(), Qt::AlignCenter, QString::number(m_imageCounter));
        painter.end();

        m_currentStatus.isAcquiring = false;
        m_currentStatus.framesAcquired = m_imageCounter;
        emit statusChanged(m_currentStatus);

        emit newImageReady(image);
        emit acquisitionFinished();
    });
}
