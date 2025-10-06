#ifndef DATASAVER_H
#define DATASAVER_H

#include <QObject>
#include <QString>
#include <atomic>
#include "core/FrameBuffer.h"

class DataSaver : public QObject
{
    Q_OBJECT

public:
    explicit DataSaver(QObject *parent = nullptr);

public slots:
    void startConsuming(const QString &directory, const QString &prefix, FrameBuffer* sourceBuffer);
    void stop();

signals:
    void finished();

private:
    FrameBuffer* m_sourceBuffer = nullptr;
    std::atomic<bool> m_stopRequested;
};

#endif // DATASAVER_H
