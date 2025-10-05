
#include "configmanager.h"
#include "common/Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) {}

bool ConfigManager::saveConfig(const ScanParameters &params, const QString &filePath)
{
    QJsonObject jsonObj = params.toJson();
    QJsonDocument doc(jsonObj);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QString msg = "Failed to open file for writing: " + filePath;
        Log("ERROR: " + msg);
        emit error(msg);
        return false;
    }

    file.write(doc.toJson());
    file.close();
    Log("Configuration saved to: " + filePath);
    return true;
}

ScanParameters ConfigManager::loadConfig(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QString msg = "Failed to open file for reading: " + filePath;
        Log("ERROR: " + msg);
        emit error(msg);
        return ScanParameters();
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject()) {
        QString msg = "Invalid JSON file format: " + filePath;
        Log("ERROR: " + msg);
        emit error(msg);
        return ScanParameters();
    }

    ScanParameters params = ScanParameters::fromJson(doc.object());
    Log("Configuration loaded from: " + filePath);
    emit configLoaded(params);
    return params;
}
