#ifndef IDETECTOR_H
#define IDETECTOR_H

#include <QObject>
#include <QImage>

class IDetector : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IDetector() {}
public slots:

    virtual void acquireFrame() = 0;

signals:
    void newImageReady(const QImage &image);
    void acquisitionFinished();
    void errorOccurred(const QString &errorMessage);
};

#endif // IDETECTOR_H
