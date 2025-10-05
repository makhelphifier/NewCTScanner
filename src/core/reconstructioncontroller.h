#ifndef RECONSTRUCTIONCONTROLLER_H
#define RECONSTRUCTIONCONTROLLER_H

#include <QObject>
#include <QString>
#include <QImage>

class ReconstructionController : public QObject
{
    Q_OBJECT
public:
    explicit ReconstructionController(QObject *parent = nullptr);

public slots:
    void startReconstruction(const QString &inputDataPath);

signals:
    void reconstructionProgress(int percentage);
    void reconstructionFinished(const QImage &sliceImage);
    void statusMessage(const QString &message);
};

#endif // RECONSTRUCTIONCONTROLLER_H
