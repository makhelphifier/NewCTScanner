
#ifndef FRAME_H
#define FRAME_H

#include <QImage>
#include <QDateTime>
#include <memory>

struct Frame {
    QImage image;

    quint64 frameNumber = 0;
    QDateTime timestamp;

    double angle = 0.0;

    double xrayVoltage_kV = 0.0;
    double xrayCurrent_uA = 0.0;
    double detectorTemperature_C = 0.0;
};

using FramePtr = std::shared_ptr<Frame>;

#endif // FRAME_H
