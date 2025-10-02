#ifndef DUMMYSOURCE_H
#define DUMMYSOURCE_H

#include "hal/IXRaySource.h"
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
// 模拟的射线源，用于开发和测试
class DummySource :  public IXRaySource {
    Q_OBJECT
public:
    explicit DummySource(QObject *parent = nullptr);
    ~DummySource();

    bool connect() override;
    void disconnect() override ;
    bool setVoltage(double kv) override;
    bool turnOn() override ;
    bool turnOff() override;

private slots:
    // 定时器超时后会调用这个槽来生成图像
    void generateImage();

private:
    QTimer* m_acquisitionTimer;
    int m_imageCounter; // 用于在图像上绘制变化的数字
};

#endif // DUMMYSOURCE_H
