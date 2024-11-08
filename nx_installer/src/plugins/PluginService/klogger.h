#ifndef KLOGGER_H
#define KLOGGER_H

#include <QMutex>
#include <QString>

#include<QDateTime>

#define LOG_FILE_NAME QString("/lingmo-os-installer_") \
    + QDateTime::currentDateTime().toString("yyyy-MM-dd")\
    + QString(".log")

static QMutex g_logMutex;
static QString g_logPath;

namespace KServer {

//设置日志路径
void setLogPath(const QString &path);
void setLogLevel(int level);
void WriteLogFile(QString path, QString logInfo);
void customLogMessageHandle(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);



}

#endif // KLOGGER_H
