#ifndef IXRAYSOURCE_H
#define IXRAYSOURCE_H
#include <QObject>
#include <QImage>

class IXRaySource: public QObject  {
    Q_OBJECT

public:
    virtual ~IXRaySource() {}

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool setVoltage(double kv) = 0;
    virtual bool turnOn(int numberOfFrames) = 0;
    virtual bool turnOff() = 0;
signals:
    void newImageReady(const QImage &image);
    void hardwareError(const QString &errorMessage);
    // 报告扫描进度
    void acquisitionProgress(int currentFrame, int totalFrames);
    // 报告扫描完成
    void acquisitionFinished();
};

#endif // IXRAYSOURCE_H
