#include "klogger.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

static int g_logLevel = QtDebugMsg;

namespace KServer {
void setLogPath(const QString &path)
{
    g_logPath = path;
    qDebug() <<"path ::" << g_logPath;

}
void setLogLevel(int level)
{
    g_logLevel = level;
}

bool static FindDirExist(const QString &dirPath)
{
    QDir dir(dirPath);
    if(dir.exists()) {
        return true;
    }
    return dir.mkpath(dirPath);
}

void WriteLogFile(QString path, QString logInfo)
{
    g_logMutex.lock();
    QFile file(path);
    QFileInfo fileInfo(file);

    if(!FindDirExist(fileInfo.absoluteDir().absolutePath())) {
        return;
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream ts(&file);
    ts << logInfo.toUtf8() <<endl;
    file.close();
    g_logMutex.unlock();
}

void customLogMessageHandle(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    Q_UNUSED(ctx);
    if(type < g_logLevel) {
        return;
    }
    QString logInfo = QString("");
    QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    switch (type) {
    case QtDebugMsg:
        logInfo = QString("%1 [Debug] %2 %3 %4 ").arg(logTime, QString::number(ctx.line), ctx.function, msg);
        break;

    case QtWarningMsg:
        logInfo = QString("%1 [Warning] %2 %3 %4").arg(logTime,/* ctx.file,*/ QString::number(ctx.line), ctx.function, msg);
        break;

    case QtCriticalMsg:
        logInfo = QString("%1 [Critical] %2 %3 %4").arg(logTime, QString::number(ctx.line), ctx.function, msg);
        break;

    case QtFatalMsg:
        logInfo = QString("%1 [Fatal] %2 %3 %4 %5").arg(logTime, QString::number(ctx.line), ctx.function, msg);
        break;

    case QtInfoMsg:
        logInfo = QString("%1 [Info] %2 %3 %4").arg(logTime, QString::number(ctx.line), ctx.function, msg);
        break;
    default:
        break;
    }
    WriteLogFile(g_logPath, logInfo);
}

}
