// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020} Uniontech Technology Co., Ltd.
 *
 * @file log.h
 *
 * @brief 日志 接口类
 *
 * @date 2020-05-26 11:55
 *
 * Author: denglinglong  <denglinglong@uniontech.com>
 *
 * Maintainer: denglinglong  <denglinglong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOG_H
#define LOG_H

#include <QMutex>
#include <QString>
#include <QStorageInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>

#define DEFALT_REMAIN_SIZE 100 * 1024 * 1024
#define MAXLOGSIZE 10 * 1024 * 1024
#define DEFALT_REMAIN_TIME 7

static QMutex s_logMutex;
static QString s_logPath;
static QString _logDir;
static QFile _logFile;

/**
 *@brief 设置日志目录
 *@param dir 目录地址
 *@return
 */
void setLogDir(const QString &dir);

/**
 *@brief 检查磁盘空间是否大于10MB
 *@param
 *@return
 */
void CheckFreeDisk();

/**
 *@brief 检查日志时间是否大于7天
 *@param
 *@return
 */
void CheckLogTime();

/**
 *@brief 创建新的日志
 *@param
 *@return
 */
void CreateNewLog();

/**
 *@brief 检查磁盘大小
 *@param
 *@return
 */
bool CheckRotateSize();

/**
 *@brief 设置日志路径
 *@param path 路径地址
 *@return
 */
void setLogPath(const QString &path);

/**
 *@brief 设置日志级别
 *@param level QtDebugMsg;QtWarningMsg;QtCriticalMsg;QtFatalMsg;
 *@return
 */
void setLogLevel(int level);

/**
 *@brief 用户日志信息
 *@param
 *@return
 */
void customLogMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
#endif // LOG_H
