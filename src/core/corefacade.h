// src/core/corefacade.h

#ifndef COREFACADE_H
#define COREFACADE_H

#include <QObject>
#include <QScopedPointer>

class HardwareService;
class ScanController;
class ConfigManager;
class ReconstructionController;
class DataAcquisitionService;
class ConfigManager;
class ReconstructionController;

class CoreFacade : public QObject
{
    Q_OBJECT
private:
    explicit CoreFacade(QObject *parent = nullptr);
    ~CoreFacade();

public:
    CoreFacade(const CoreFacade&) = delete;
    CoreFacade& operator=(const CoreFacade&) = delete;

    static CoreFacade& instance();

    void init();

    HardwareService* hardwareService() const;
    ScanController* scanController() const;
    DataAcquisitionService* dataAcquisitionService() const;
    ConfigManager* configManager() const;
    ReconstructionController* reconstructionController() const;

private:
    QScopedPointer<HardwareService> m_hardwareService;
    QScopedPointer<ScanController> m_scanController;
    QScopedPointer<DataAcquisitionService> m_dataAcquisitionService;
    QScopedPointer<ConfigManager> m_configManager;
    QScopedPointer<ReconstructionController> m_reconController;

};

#endif // COREFACADE_H
