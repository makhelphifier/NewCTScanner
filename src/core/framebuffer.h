#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "core/Frame.h"
#include <vector>
#include <QMutex>
#include <QWaitCondition>

class FrameBuffer {
public:
    explicit FrameBuffer(size_t capacity = 512);
    ~FrameBuffer();

    bool push(FramePtr frame, int timeout_ms = 100);

    FramePtr pop(int timeout_ms = 100);

    size_t size() const;
    bool isEmpty() const;
    void clear();

private:
    std::vector<FramePtr> m_buffer;
    size_t m_capacity;
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_count = 0;

    mutable QMutex m_mutex;
    QWaitCondition m_notEmpty;
    QWaitCondition m_notFull;
};

#endif // FRAMEBUFFER_H
