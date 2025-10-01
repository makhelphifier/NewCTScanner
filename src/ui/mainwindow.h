#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include "core/scancontroller.h"


// 前向声明
class ScanController;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void updateStatus(const QString &status);
    void onStateChanged(ScanController::ScanState newState);

private:
    // UI 持有业务逻辑层的实例
    ScanController* m_scanController;

    // UI 上的控件
    QPushButton* m_startXRayButton;
    QPushButton* m_stopButton;


    QThread* m_workerThread;
    QLabel* m_statusLabel;
};
#endif // MAINWINDOW_H
