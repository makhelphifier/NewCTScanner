#ifndef RECONSTRUCTIONTASK_H
#define RECONSTRUCTIONTASK_H

#include <QObject>
#include <QRunnable>
#include <QImage>


class ReconstructionTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ReconstructionTask(const QString &inputPath, QObject *parent = nullptr);

    void run() override;

signals:
    void taskProgress(int percentage);
    void taskFinished(const QImage &sliceImage);
    void statusMessage(const QString &message);

private:
    QString m_inputPath;
};

#endif // RECONSTRUCTIONTASK_H
