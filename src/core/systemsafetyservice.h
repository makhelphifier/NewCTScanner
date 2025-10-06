#ifndef SYSTEMSAFETYSERVICE_H
#define SYSTEMSAFETYSERVICE_H

#include <QObject>
#include <QTimer>

class SystemSafetyService : public QObject
{
    Q_OBJECT
public:
    explicit SystemSafetyService(QObject *parent = nullptr);

signals:
    void safetyInterlockTriggered(const QString &reason);

public slots:
    void setDoorState(bool is_closed);

private slots:
    void pollSafetyStatus();

private:
    bool m_isDoorClosed;
    QTimer* m_pollingTimer;
};

#endif // SYSTEMSAFETYSERVICE_H
