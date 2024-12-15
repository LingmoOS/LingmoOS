// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "log.h"

static int s_logLevel = QtDebugMsg;
static quint64 _logDaysRemain = DEFALT_REMAIN_TIME;
static int _rotateSize = MAXLOGSIZE;
static quint64 _remainDisk = DEFALT_REMAIN_SIZE;

void setLogPath(const QString &path)
{
    s_logPath = path;
}
void setLogDir(const QString &dir)
{
    _logDir = dir;
}
void setLogLevel(int level)
{
    s_logLevel = level;
}

bool static ensureDirExist(const QString &dirPath)
{
    QDir dir(dirPath);
    if (dir.exists()) {
        return true;
    }

    return dir.mkpath(dirPath);
}
QFileInfoList GetLogList()
{
    QStringList nameFilter;
    nameFilter << "*.log";
    QDir logDir(_logDir);
    auto logList = logDir.entryInfoList(nameFilter, QDir::Files, QDir::Name);
    return logList;
}
void CheckLogTime()
{
    auto curTime = QDateTime::currentDateTime();
    auto logList = GetLogList();
    for (int i = 0; i < logList.size() - 1; ++i) {
        auto logInfo = logList[i];
        auto createdTime = logInfo.created();
        auto elapseDays = createdTime.daysTo(curTime);
        if (elapseDays > _logDaysRemain) {
            auto logPath = logInfo.absoluteFilePath();
            QDir dir;
            dir.remove(logPath);
        }
    }
}
quint64 GetDiskFreeSpace()
{
    QStorageInfo storage = QStorageInfo::root();
    storage.refresh(); //获得最新磁盘信息

    if (storage.isReadOnly())
        qDebug() << "isReadOnly:" << storage.isReadOnly();
    else {
        qDebug() << "name:" << storage.name();
        qDebug() << "fileSystemType:" << storage.fileSystemType();
        qDebug() << "size:" << storage.bytesTotal() / 1000 / 1000 << "MB";
        qDebug() << "availableSize:" << storage.bytesAvailable() / 1000 / 1000 << "MB";
    }
    return (quint64)storage.bytesAvailable();
}
void CheckFreeDisk()
{
    auto freeSpace = GetDiskFreeSpace();

    if (freeSpace < _remainDisk) {
        auto logList = GetLogList();
        for (int i = 0; i < logList.size() - 1; ++i) {
            QDir dir;
            dir.remove(logList[i].absolutePath());
            freeSpace = GetDiskFreeSpace();
            if (freeSpace > _remainDisk) {
                break;
            }
        }
    }
}
bool CheckRotateSize()
{
    bool ret = _logFile.size() >= _rotateSize;
    return ret;
}
bool CheckRotateTimePoint()
{
    QFileInfo curLogInfo(_logFile);
    auto curLogCreateDate = curLogInfo.created();
    auto curDate = QDateTime::currentDateTime();
    auto dayElapse = curLogCreateDate.daysTo(curDate);
    if (dayElapse >= 1) {
        return true;
    }

    return false;
}
void CloseLog()
{
    _logFile.close();
}

void WriteVersion()
{
    if (CheckRotateSize() || CheckRotateTimePoint()) {
        CloseLog();
        CreateNewLog();
    }
    s_logMutex.lock();
    QFile outFile(s_logPath);
    QFileInfo fileInfo(outFile);
    if (!ensureDirExist(fileInfo.absoluteDir().absolutePath()))
        return;

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream ts(&outFile);
    auto appName = QCoreApplication::applicationName();
    auto version = QCoreApplication::applicationVersion();
    ts << appName << " " << version << endl;
    outFile.close();
    s_logMutex.unlock();
}
void CreateNewLog()
{
    auto appName = QCoreApplication::applicationName();
    auto version = QCoreApplication::applicationVersion();
    auto curTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    auto logName = _logDir + appName + "_" + curTime + ".log";
    _logFile.setFileName(logName);
    setLogPath(logName);
    _logFile.open(QIODevice::ReadWrite | QIODevice::Append);
    //日志头写入App\系统参数
    WriteVersion();
}

void customLogMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    if (type < s_logLevel) {
        return;
    }

    QString logInfo;
    switch (type) {
    case QtDebugMsg:
        logInfo = QString("Debug:");
        break;
    case QtWarningMsg:
        logInfo = QString("Warning:");
        break;
    case QtCriticalMsg:
        logInfo = QString("Critical:");
        break;
    case QtFatalMsg:
        logInfo = QString("Fatal:");
    }
    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(ctx.file)).arg(ctx.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(logInfo).arg(context_info).arg(msg).arg(current_date);
    s_logMutex.lock();
    QFile outFile(s_logPath);
    QFileInfo fileInfo(outFile);
    if (!ensureDirExist(fileInfo.absoluteDir().absolutePath()))
        return;

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream ts(&outFile);
    ts << message.toUtf8() << endl;
    outFile.close();
    s_logMutex.unlock();
}
