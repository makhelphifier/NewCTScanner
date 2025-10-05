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
    // 保存参数到指定文件路径
    bool saveConfig(const ScanParameters &params, const QString &filePath);
    // 从指定文件路径加载参数
    ScanParameters loadConfig(const QString &filePath);

signals:
    // 当加载成功时，发出带有新参数的信号
    void configLoaded(const ScanParameters &params);
    // 当加载或保存失败时，发出错误信息
    void error(const QString &message);
};

#endif // CONFIGMANAGER_H
