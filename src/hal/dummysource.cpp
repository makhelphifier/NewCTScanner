#include "dummysource.h"
#include <QDebug>
#include <QPainter>
#include "common/Logger.h"
#include <QRandomGenerator>

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
    if (QRandomGenerator::global()->bounded(2) == 0) {
        QString errorMsg = "Failed to connect to X-Ray source (Simulated Error).";
        Log("ERROR: " + errorMsg);
        emit hardwareError(errorMsg);
        return false;
    }

    Log("DummySource: connect() called successfully.");
    return true;
}

void DummySource::disconnect()
{
    qDebug() << "DummySource: Disconnected.";
}

bool DummySource::setVoltage(double kv)
{
    Log(QString("DummySource: Setting voltage to %1 kV.").arg(kv));
    return true;
}

bool DummySource::turnOn(int numberOfFrames) {
    Log(QString("DummySource: X-Ray ON. Starting acquisition for %1 frames.").arg(numberOfFrames));
    m_imageCounter = 0;
    m_totalFrames = numberOfFrames;
    m_acquisitionTimer->start();
    return true;
}
bool DummySource::turnOff() {
    Log("DummySource: X-Ray OFF.");
    m_acquisitionTimer->stop();
    return true;
}

void DummySource::generateImage() {
    if (m_imageCounter >= m_totalFrames) {
        turnOff(); // 停止定时器
        emit acquisitionFinished(); // 发射完成信号
        return;
    }

    m_imageCounter++;

    // 创建图像 (代码不变)
    QImage image(256, 256, QImage::Format_RGB888);
    image.fill(Qt::darkGray);
    QPainter painter(&image);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 40));
    painter.drawText(image.rect(), Qt::AlignCenter, QString::number(m_imageCounter));
    painter.end();

    // 发射新图像和进度信号
    emit newImageReady(image);
    emit acquisitionProgress(m_imageCounter, m_totalFrames);
}
