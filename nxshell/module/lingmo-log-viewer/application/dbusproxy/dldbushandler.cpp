// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dldbushandler.h"
#include <QDebug>
#include <QStandardPaths>
#include <QLoggingCategory>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logDBusHandler, "org.lingmo.log.viewer.dbus.handler")
#else
Q_LOGGING_CATEGORY(logDBusHandler, "org.lingmo.log.viewer.dbus.handler", QtInfoMsg)
#endif

DLDBusHandler *DLDBusHandler::m_statichandeler = nullptr;

DLDBusHandler *DLDBusHandler::instance(QObject *parent)
{
    if (parent != nullptr && m_statichandeler == nullptr) {
        m_statichandeler = new DLDBusHandler(parent);
    }
    return m_statichandeler;
}

DLDBusHandler::~DLDBusHandler()
{
    quit();
}

DLDBusHandler::DLDBusHandler(QObject *parent)
    : QObject(parent)
{
    m_dbus = new LingmoLogviewerInterface("com.lingmo.logviewer",
                                          "/com/lingmo/logviewer",
                                          QDBusConnection::systemBus(),
                                          this);
    //Note: when dealing with remote objects, it is not always possible to determine if it exists when creating a QDBusInterface.
    if (!m_dbus->isValid() && !m_dbus->lastError().message().isEmpty()) {
        qCCritical(logDBusHandler) << "dbus com.lingmo.logviewer isValid false error:" << m_dbus->lastError() << m_dbus->lastError().message();
    }
    qCDebug(logDBusHandler) << "dbus com.lingmo.logviewer isValid true";
}

/*!
 * \~chinese \brief DLDBusHandler::readLog 读取日志文件
 * \~chinese \param filePath 文件路径
 * \~chinese \return 读取的日志
 */
QString DLDBusHandler::readLog(const QString &filePath)
{
    QString tempFilePath = createFilePathCacheFile(filePath);
    QFile file(tempFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open filePath cache file:" << tempFilePath;
        return QString("");
    }
    const int fd = file.handle();
    if (fd <= 0) {
        qWarning() << "originPath file fd error. filePath cache file:" << tempFilePath;
        return QString("");
    }

    QDBusUnixFileDescriptor dbusFd(fd);
    QString log = m_dbus->readLog(dbusFd);

    file.close();
    releaseFilePathCacheFile(tempFilePath);

    return log;
}

/*!
 * \~chinese \brief DLDBusHandler::readLogLinesInRange 获取指定行数范围的日志内容，默认读取500条数据
 * \~chinese \param filePath 文件路径
 * \~chinese \param startLine 起始行
 * \~chinese \param lineCount 获取行数
 * \~chinese \return 读取的日志
 */
QStringList DLDBusHandler::readLogLinesInRange(const QString &filePath, qint64 startLine, qint64 lineCount, bool bReverse)
{
    QString tempFilePath = createFilePathCacheFile(filePath);
    QFile file(tempFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open filePath cache file:" << tempFilePath;
        return QStringList();
    }
    const int fd = file.handle();
    if (fd <= 0) {
        qWarning() << "originPath file fd error. filePath cache file:" << tempFilePath;
        return QStringList();
    }

    QDBusUnixFileDescriptor dbusFd(fd);
    QStringList lines = m_dbus->readLogLinesInRange(dbusFd, startLine, lineCount, bReverse);

    file.close();
    releaseFilePathCacheFile(tempFilePath);

    return lines;
}

QString DLDBusHandler::openLogStream(const QString &filePath)
{
    return m_dbus->openLogStream(filePath);
}

QString DLDBusHandler::readLogInStream(const QString &token)
{
    return m_dbus->readLogInStream(token);
}

QStringList DLDBusHandler::whiteListOutPaths()
{
    return m_dbus->whiteListOutPaths();
}

/*!
 * \~chinese \brief DLDBusHandler::exitCode 返回进程状态
 * \~chinese \return 进程返回值
 */
int DLDBusHandler::exitCode()
{
    return m_dbus->exitCode();
}

/*!
 * \~chinese \brief DLDBusHandler::quit 退出服务端程序
 */
void DLDBusHandler::quit()
{
    m_dbus->quit();
}

QStringList DLDBusHandler::getFileInfo(const QString &flag, bool unzip)
{
    QDBusPendingReply<QStringList> reply = m_dbus->getFileInfo(flag, unzip);
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(logDBusHandler) << "call dbus iterface 'getFileInfo()' failed. error info:" << reply.error().message();
    } else {
        filePath = reply.value();
    }
    return filePath;
}

QStringList DLDBusHandler::getOtherFileInfo(const QString &flag, bool unzip)
{
    QDBusPendingReply<QStringList> reply = m_dbus->getOtherFileInfo(flag, unzip);
    reply.waitForFinished();
    QStringList filePathList;
    if (reply.isError()) {
        qCWarning(logDBusHandler) << "call dbus iterface 'getOtherFileInfo()' failed. error info:" << reply.error().message();
    } else {
        filePathList = reply.value();
    }
    return filePathList;
}


bool DLDBusHandler::exportLog(const QString &outDir, const QString &in, bool isFile)
{
    return m_dbus->exportLog(outDir, in, isFile);
}

bool DLDBusHandler::isFileExist(const QString &filePath)
{
    QString ret = m_dbus->isFileExist(filePath);
    return ret == "exist";
}

quint64 DLDBusHandler::getFileSize(const QString &filePath)
{
    return m_dbus->getFileSize(filePath);
}

qint64 DLDBusHandler::getLineCount(const QString &filePath)
{
    return m_dbus->getLineCount(filePath);
}

QString DLDBusHandler::executeCmd(const QString &cmd)
{
    return m_dbus->executeCmd(cmd);
}

QString DLDBusHandler::createFilePathCacheFile(const QString &logFilePath)
{
    QString tempFilePath = m_tempDir.path() + QDir::separator() + "Log_file_path.txt";

    QFile tmpFile(tempFilePath);
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open temp file:" << tmpFile;
        return QString("");
    }

    QTextStream in(&tmpFile);
    in << logFilePath;
    tmpFile.close();

    return tempFilePath;
}

void DLDBusHandler::releaseFilePathCacheFile(const QString &cacheFilePath)
{
    if (!cacheFilePath.isEmpty())
        QFile::remove(cacheFilePath);
}
