#ifndef DATAACQUISITIONSERVICE_H
#define DATAACQUISITIONSERVICE_H

#include <QObject>
#include <QThread>
#include <memory>
#include "core/Frame.h"

class FrameBuffer;
class AcquisitionWorker;
class DataSaver;
class HardwareService;
class ScanParameters;

class DataAcquisitionService : public QObject
{
    Q_OBJECT

public:
    explicit DataAcquisitionService(HardwareService* hardwareService, QObject *parent = nullptr);
    ~DataAcquisitionService();

public slots:
    void start(const ScanParameters &params, const QString &saveDirectory, const QString &savePrefix);
    void stop();

signals:
    void acquisitionFinished();
    void newFrameReady(FramePtr frame); // 用于实时显示
    void scanProgress(int current, int total);

private:
    HardwareService* m_hardwareService;
    std::unique_ptr<FrameBuffer> m_frameBuffer;

    QThread* m_workerThread;
    AcquisitionWorker* m_worker;

    QThread* m_saverThread;
    DataSaver* m_dataSaver;
};

#endif // DATAACQUISITIONSERVICE_H
