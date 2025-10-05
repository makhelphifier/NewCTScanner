#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "common/ScanParameters.h"
#include <QObject>
#include <QString>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);

public slots:
    bool saveConfig(const ScanParameters &params, const QString &filePath);
    ScanParameters loadConfig(const QString &filePath);

signals:

    void configLoaded(const ScanParameters &params);
    void error(const QString &message);
};

#endif // CONFIGMANAGER_H
