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
public:
    double voltage = 100.0;
    double current = 50.0;
    int frameCount = 360; // 默认采集360帧
    // ... (在成员变量下面)
    // 将结构体转换为QJsonObject
    QJsonObject toJson() const {
        QJsonObject json;
        json["voltage"] = voltage;
        json["current"] = current;
        json["frameCount"] = frameCount;
        return json;
    }

    // 从QJsonObject填充结构体
    static ScanParameters fromJson(const QJsonObject &json) {
        ScanParameters params;
        if (json.contains("voltage")) params.voltage = json["voltage"].toDouble();
        if (json.contains("current")) params.current = json["current"].toDouble();
        if (json.contains("frameCount")) params.frameCount = json["frameCount"].toInt();
        return params;
    }
};

#endif // SCANPARAMETERS_H
