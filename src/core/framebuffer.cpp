#include "framebuffer.h"

FrameBuffer::FrameBuffer(size_t capacity)
    : m_capacity(capacity)
{
    m_buffer.resize(capacity);
}

FrameBuffer::~FrameBuffer()
{
    clear();
}

bool FrameBuffer::push(FramePtr frame, int timeout_ms)
{
    QMutexLocker locker(&m_mutex);

    // 如果缓冲区是满的，等待直到有空间或超时
    if (m_count == m_capacity) {
        if (!m_notFull.wait(&m_mutex, timeout_ms)) {
            return false; // 超时
        }
    }

    m_buffer[m_tail] = frame;
    m_tail = (m_tail + 1) % m_capacity;
    m_count++;

    // 通知可能在等待的消费者
    m_notEmpty.wakeOne();
    return true;
}

FramePtr FrameBuffer::pop(int timeout_ms)
{
    QMutexLocker locker(&m_mutex);

    // 如果缓冲区是空的，等待直到有数据或超时
    if (m_count == 0) {
        if (!m_notEmpty.wait(&m_mutex, timeout_ms)) {
            return nullptr; // 超时
        }
    }

    FramePtr frame = m_buffer[m_head];
    m_head = (m_head + 1) % m_capacity;
    m_count--;

    // 通知可能在等待的生产者
    m_notFull.wakeOne();
    return frame;
}

size_t FrameBuffer::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_count;
}

bool FrameBuffer::isEmpty() const
{
    QMutexLocker locker(&m_mutex);
    return m_count == 0;
}

void FrameBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_head = 0;
    m_tail = 0;
    m_count = 0;
    m_notFull.wakeAll(); // 唤醒所有可能在等待的生产者
}
