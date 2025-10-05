#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT
private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger* instance();

    void log(const QString &message);

signals:
    void newLogMessage(const QString &message);

private:
    static Logger* m_instance;
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
};

#define Log(message) Logger::instance()->log(message)

#endif // LOGGER_H
