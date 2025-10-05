#ifndef IMOTIONSTAGE_H
#define IMOTIONSTAGE_H

#include <QObject>

class IMotionStage : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IMotionStage() {}
public slots:

    virtual void moveTo(double position) = 0;
    virtual void reset() = 0;

signals:
    void moveFinished(bool success);
    void errorOccurred(const QString &errorMessage);
};

#endif // IMOTIONSTAGE_H
