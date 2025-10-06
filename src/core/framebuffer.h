#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "core/Frame.h"
#include <vector>
#include <QMutex>
#include <QWaitCondition>

class FrameBuffer {
public:
    explicit FrameBuffer(size_t capacity = 512); // 默认缓冲区大小为512帧
    ~FrameBuffer();

    // 生产者调用：将一帧数据放入缓冲区，带超时
    bool push(FramePtr frame, int timeout_ms = 100);

    // 消费者调用：从缓冲区取出一帧数据，带超时
    FramePtr pop(int timeout_ms = 100);

    size_t size() const;
    bool isEmpty() const;
    void clear();

private:
    std::vector<FramePtr> m_buffer;
    size_t m_capacity;
    size_t m_head = 0; // 读取位置
    size_t m_tail = 0; // 写入位置
    size_t m_count = 0; // 当前帧数

    mutable QMutex m_mutex;
    QWaitCondition m_notEmpty; // 非空条件变量
    QWaitCondition m_notFull;  // 非满条件变量
};

#endif // FRAMEBUFFER_H
