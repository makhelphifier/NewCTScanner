#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include <QObject>

class ScanController : public QObject
{
    Q_OBJECT
public:
    explicit ScanController(QObject *parent = nullptr);

signals:
};

#endif // SCANCONTROLLER_H
