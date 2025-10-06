#ifndef SCANPROGRESS_H
#define SCANPROGRESS_H

#include <QObject>
#include <QMetaType>
#include <QElapsedTimer>

struct ScanProgress {
    int currentProjection = 0;
    int totalProjections = 0;
    double currentAngle = 0.0;
    int percentComplete = 0;
    qint64 elapsedTime_ms = 0;
    qint64 estimatedTimeRemaining_ms = 0;
};

Q_DECLARE_METATYPE(ScanProgress)

#endif // SCANPROGRESS_H
