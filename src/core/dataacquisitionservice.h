#ifndef DATAACQUISITIONSERVICE_H
#define DATAACQUISITIONSERVICE_H

#include <QObject>
#include <QImage>
#include <QThread>

class DataSaver;

class DataAcquisitionService : public QObject
{
    Q_OBJECT
public:
    explicit DataAcquisitionService(QObject *parent = nullptr);
    ~DataAcquisitionService();

public slots:
    void startSaving(const QString &directory, const QString &prefix);
    void stopSaving();
    void saveImage(const QImage &image);

private:
    DataSaver* m_dataSaver;
    QThread* m_saverThread;
};

#endif // DATAACQUISITIONSERVICE_H
