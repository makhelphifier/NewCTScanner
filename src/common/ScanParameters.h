// src/common/ScanParameters.h

#ifndef SCANPARAMETERS_H
#define SCANPARAMETERS_H

#include <QObject>
#include <QMetaType>
#include <QJsonObject>
#include <QJsonDocument>

struct ScanParameters {
    Q_GADGET
    Q_PROPERTY(double voltage MEMBER voltage)
    Q_PROPERTY(double current MEMBER current)
    Q_PROPERTY(int frameCount MEMBER frameCount)

    Q_PROPERTY(double startAngle MEMBER startAngle)
    Q_PROPERTY(double endAngle MEMBER endAngle)

    Q_PROPERTY(double rotationSpeed MEMBER rotationSpeed)


public:
    double voltage = 100.0;
    double current = 50.0;
    int frameCount = 360;

    double startAngle = 0.0;
    double endAngle = 360.0;

    double rotationSpeed = 1.0;

    QJsonObject toJson() const {
        QJsonObject json;
        json["voltage"] = voltage;
        json["current"] = current;
        json["frameCount"] = frameCount;
        json["startAngle"] = startAngle;
        json["endAngle"] = endAngle;
        json["rotationSpeed"] = rotationSpeed;
        return json;
    }
    static ScanParameters fromJson(const QJsonObject &json) {
        ScanParameters params;
        if (json.contains("voltage")) params.voltage = json["voltage"].toDouble();
        if (json.contains("current")) params.current = json["current"].toDouble();
        if (json.contains("frameCount")) params.frameCount = json["frameCount"].toInt();
        if (json.contains("startAngle")) params.startAngle = json["startAngle"].toDouble();
        if (json.contains("endAngle")) params.endAngle = json["endAngle"].toDouble();
        if (json.contains("rotationSpeed")) params.rotationSpeed = json["rotationSpeed"].toDouble();
        return params;
    }
};

#endif // SCANPARAMETERS_H
