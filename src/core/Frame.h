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
    // 未来可以添加更多元数据, 如温度、电压等
};

// 使用智能指针来管理Frame的生命周期，可以提升内存管理效率
using FramePtr = std::shared_ptr<Frame>;

#endif // FRAME_H
