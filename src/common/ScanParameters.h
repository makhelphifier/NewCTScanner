#ifndef SCANPARAMETERS_H
#define SCANPARAMETERS_H

#include <QObject>
#include <QMetaType>

struct ScanParameters {
    Q_GADGET
    Q_PROPERTY(double voltage MEMBER voltage)
    Q_PROPERTY(double current MEMBER current)
public:
    double voltage = 100.0;
    double current = 50.0;
};

#endif // SCANPARAMETERS_H
