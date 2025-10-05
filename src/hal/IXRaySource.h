#ifndef IXRAYSOURCE_H
#define IXRAYSOURCE_H
#include <QObject>
#include <QImage>

class IXRaySource: public QObject  {
    Q_OBJECT

public:
    virtual ~IXRaySource() {}
public slots:

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool setVoltage(double kv) = 0;
    virtual bool turnOn(int numberOfFrames) = 0;
    virtual bool turnOff() = 0;
signals:
    void newImageReady(const QImage &image);
    void hardwareError(const QString &errorMessage);
    void acquisitionProgress(int currentFrame, int totalFrames);
    void acquisitionFinished();
};

#endif // IXRAYSOURCE_H
