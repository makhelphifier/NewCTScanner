#include "DummyDetector.h"
#include <QPainter>
#include <QTimer>
#include "common/Logger.h"

DummyDetector::DummyDetector(QObject *parent) : IDetector(parent), m_imageCounter(0)
{
    Log("DummyDetector created.");
}

void DummyDetector::acquireFrame()
{
    m_imageCounter++;
    QTimer::singleShot(50, this, [this](){
        QImage image(256, 256, QImage::Format_RGB888);
        image.fill(Qt::darkCyan);
        QPainter painter(&image);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 40));
        painter.drawText(image.rect(), Qt::AlignCenter, QString::number(m_imageCounter));
        painter.end();

        emit newImageReady(image);
        emit acquisitionFinished();
    });
}
