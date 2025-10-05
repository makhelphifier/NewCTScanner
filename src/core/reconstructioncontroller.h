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
    // 启动重建过程的槽
    void startReconstruction(const QString &inputDataPath);

signals:
    // 报告重建进度
    void reconstructionProgress(int percentage);
    // 重建完成，并返回一张模拟的切片图像
    void reconstructionFinished(const QImage &sliceImage);
    // 报告状态/日志信息
    void statusMessage(const QString &message);
};

#endif // RECONSTRUCTIONCONTROLLER_H
