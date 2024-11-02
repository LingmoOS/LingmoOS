/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "log-utils.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>

#define LOG_FILE_COUNT         2
#define MAX_LOG_FILE_SIZE      4194304
#define MAX_LOG_CHECK_INTERVAL 43200000

quint64 LogUtils::m_startUpTime = 0;
int LogUtils::m_logFileId = -1;
QString LogUtils::m_logFileName;
QString LogUtils::m_currentLogFile;
static QString logFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.log/lingmo-panel/";

void LogUtils::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    checkLogFile();

    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray currentTime = QTime::currentTime().toString().toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    FILE *log_file = fopen(m_currentLogFile.toLocal8Bit().constData(), "a+");

    switch (type) {
    case QtDebugMsg:
        if (!log_file) {
            break;
        }
        fprintf(log_file, "Debug: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(log_file? log_file: stdout, "Info: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(log_file? log_file: stderr, "Warning: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(log_file? log_file: stderr, "Critical: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(log_file? log_file: stderr, "Fatal: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    }

    if (log_file) {
        fclose(log_file);
    }
}

void LogUtils::initLogFile(const QString &fileName)
{
    QDir dir;
    if (!dir.exists(logFilePath)) {
        if (!dir.mkpath(logFilePath)) {
            qWarning() << "Unable to create" << logFilePath;
            return;
        }
    }
    m_logFileName = logFilePath + fileName + "-%1.log";

    for (int i = 0; i < LOG_FILE_COUNT; ++i) {
        m_currentLogFile = m_logFileName.arg(i);
        if (QFile::exists(m_currentLogFile)) {
            if (checkFileSize(m_currentLogFile)) {
                m_logFileId = i;
                break;
            }
        } else {
            QFile file(m_currentLogFile);
            file.open(QIODevice::WriteOnly);
            file.close();
        }
    }


    if (m_logFileId < 0) {
        m_logFileId = 0;
        m_currentLogFile = m_logFileName.arg(m_logFileId);
        clearFile(m_currentLogFile);
    }

    qInfo() << "Current log file:" << m_currentLogFile;
}

void LogUtils::checkLogFile()
{
    quint64 logTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    quint64 spacing = std::max(logTime, m_startUpTime) - std::min(logTime, m_startUpTime);

    if (spacing <= MAX_LOG_CHECK_INTERVAL || checkFileSize(m_currentLogFile)) {
        return;
    }

    m_logFileId = ((m_logFileId + 1) % LOG_FILE_COUNT);
    m_currentLogFile = m_logFileName.arg(m_logFileId);
    if (!checkFileSize(m_currentLogFile)) {
        clearFile(m_currentLogFile);
    }
}

bool LogUtils::checkFileSize(const QString &fileName)
{
    return QFile(fileName).size() < MAX_LOG_FILE_SIZE;
}

void LogUtils::clearFile(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write("");
    file.flush();
    file.close();
}
