#include "logmanager.h"
#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>

LogManager::LogManager()
{
    m_format = "%{time}{dd-MM-yyyy, HH:mm:ss.zzz} [%{type:-7}] [%{file:-25} %{line}] %{message}\n";
}

void LogManager::initConsoleAppender(){
    ConsoleAppender *defaultAppender = new ConsoleAppender;
    defaultAppender->setFormat(m_format);
    logger->registerAppender(defaultAppender);

    ConsoleAppender *qmlAppender = new ConsoleAppender;
    qmlAppender->setFormat(m_format);
    logger->registerCategoryAppender("qml", qmlAppender);
}

void LogManager::initRollingFileAppender(){
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    if (!QDir(cachePath).exists()){
        QDir(cachePath).mkpath(cachePath);
    }
    m_logPath = joinPath(cachePath, QString("%1.log").arg(qApp->applicationName()));

    RollingFileAppender * defaultAppender = new RollingFileAppender(m_logPath);
    defaultAppender->setFormat(m_format);
    defaultAppender->setLogFilesLimit(5);
    defaultAppender->setDatePattern(RollingFileAppender::DailyRollover);
    logger->registerAppender(defaultAppender);

    RollingFileAppender * qmlAppender = new RollingFileAppender(m_logPath);
    qmlAppender->setFormat(m_format);
    qmlAppender->setLogFilesLimit(5);
    qmlAppender->setDatePattern(RollingFileAppender::DailyRollover);
    logger->registerCategoryAppender("qml", qmlAppender);
}

void LogManager::debug_log_console_on(){
    #if !defined(QT_NO_DEBUG)
    LogManager::instance()->initConsoleAppender();
    #endif
    LogManager::instance()->initRollingFileAppender();
}


QString LogManager::joinPath(const QString &path, const QString &fileName){
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}


QString LogManager::getlogFilePath(){
    return m_logPath;
}

LogManager::~LogManager()
{

}
