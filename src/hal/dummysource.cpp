#include "dummysource.h"
#include <QDebug>
#include <QPainter>

DummySource::DummySource(QObject *parent)
    : IXRaySource()
{
    m_acquisitionTimer = new QTimer(this);
    m_acquisitionTimer->setInterval(100);
    QObject::connect(m_acquisitionTimer, &QTimer::timeout, this, &DummySource::generateImage);

    m_imageCounter = 0;
}

DummySource::~DummySource()
{
}

bool DummySource::connect()
{
    qDebug() << "DummySource: Connected.";
    return true;
}

void DummySource::disconnect()
{
    qDebug() << "DummySource: Disconnected.";
}

bool DummySource::setVoltage(double kv)
{
    qDebug() << "DummySource: Voltage set to" << kv << "kV";
    return true;
}

bool DummySource::turnOn() {
    qDebug() << "DummySource: X-Ray ON. Starting acquisition timer.";
    m_acquisitionTimer->start();
    return true;
}

bool DummySource::turnOff() {
    qDebug() << "DummySource: X-Ray OFF. Stopping acquisition timer.";
    m_acquisitionTimer->stop();
    return true;
}

void DummySource::generateImage() {
    QImage image(256, 256, QImage::Format_RGB888);
    image.fill(Qt::darkGray);
    QPainter painter(&image);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 40));
    painter.drawText(image.rect(), Qt::AlignCenter, QString::number(m_imageCounter++));
    emit newImageReady(image);
}
