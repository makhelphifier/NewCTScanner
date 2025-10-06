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

    if (m_count == m_capacity) {
        if (!m_notFull.wait(&m_mutex, timeout_ms)) {
            return false;
        }
    }

    m_buffer[m_tail] = frame;
    m_tail = (m_tail + 1) % m_capacity;
    m_count++;

    m_notEmpty.wakeOne();
    return true;
}

FramePtr FrameBuffer::pop(int timeout_ms)
{
    QMutexLocker locker(&m_mutex);

    if (m_count == 0) {
        if (!m_notEmpty.wait(&m_mutex, timeout_ms)) {
            return nullptr;
        }
    }

    FramePtr frame = m_buffer[m_head];
    m_head = (m_head + 1) % m_capacity;
    m_count--;

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
    m_notFull.wakeAll();
}
