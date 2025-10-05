#include "Logger.h"
#include <QDateTime>

Logger* Logger::m_instance = nullptr;

Logger* Logger::instance()
{
    if (m_instance == nullptr) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (m_instance == nullptr) {
            m_instance = new Logger();
        }
    }
    return m_instance;
}

Logger::Logger(QObject *parent) : QObject(parent)
{
    m_logFile.setFileName("ct_scanner_log.txt");
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream.setDevice(&m_logFile);
    }
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::log(const QString &message)
{
    QMutexLocker locker(&m_mutex);

    QString formattedMessage = QString("[%1] %2")
                                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                                   .arg(message);

    if (m_logStream.device()) {
        m_logStream << formattedMessage << "\n";
        m_logStream.flush();
    }

    emit newLogMessage(formattedMessage);
}
