#ifndef DATASAVER_H
#define DATASAVER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QQueue>
#include <QMutex>

class DataSaver : public QObject
{
    Q_OBJECT
public:
    explicit DataSaver(QObject *parent = nullptr);

public slots:
    void startSaving(const QString &directory, const QString &prefix);
    void stopSaving();
    void queueImage(const QImage &image);

private:
    void processImageQueue();

    QString m_directory;
    QString m_prefix;
    int m_fileCounter;
    bool m_isSaving;

    QQueue<QImage> m_imageQueue;
    QMutex m_queueMutex;
};

#endif // DATASAVER_H
