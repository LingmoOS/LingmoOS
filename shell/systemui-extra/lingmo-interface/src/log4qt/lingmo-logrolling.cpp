/*
 * Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmo-logrolling.h"
#include "lingmo-log4qt.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QProcess>

LingmoUILog4qtRolling::LingmoUILog4qtRolling(QString strBaseFilePath, unsigned uMaxFileCount, quint64 uMaxFileSize, 
        quint64 delayCheckFileTime, QObject *parent) :
    QThread(parent),
    m_isExit(false),
    m_delayCheckFileTime(delayCheckFileTime),
    m_strBaseFilePath(strBaseFilePath),
    m_uMaxFileCount(uMaxFileCount),
    m_uMaxFileSize(uMaxFileSize*1024*1024)  // 单位 M
{
}

LingmoUILog4qtRolling::~LingmoUILog4qtRolling()
{

}

// 线程过程
void LingmoUILog4qtRolling::run()
{
    // 时间间隔参数异常
    if (m_delayCheckFileTime == 0) {
        return ;
    }

    // 定时检查日志文件和大小
    while (!m_isExit && m_delayCheckFileTime != 0) {
        m_lockReadWrite.lockForRead();
        if (m_uMaxFileCount > 0) {
            checkLogFilesCount();
        }
        if (m_uMaxFileSize > 0) {
            checkLogFilesSize();
        }
        quint64 willDelayCheckFileTime = m_delayCheckFileTime*1000;
        m_lockReadWrite.unlock();
        // 细化sleep片段
        while (!m_isExit && willDelayCheckFileTime >= 50 && !m_isConfigChange) {
            QThread::msleep(50);
            willDelayCheckFileTime -= 50;
        }
        m_lockReadWrite.lockForWrite();
        m_isConfigChange = false;
        m_lockReadWrite.unlock();
        if (m_isExit) {
            break;
        }
    }
}

// 设置文件检查限制条件
void LingmoUILog4qtRolling::setFileCheckLimit(unsigned uMaxFileCount, quint64 uMaxFileSize, quint64 delayCheckFileTime)
{
    m_lockReadWrite.lockForWrite();
    m_uMaxFileCount = uMaxFileCount;
    m_uMaxFileSize = uMaxFileSize*1024*1024; // 单位 M
    m_delayCheckFileTime = delayCheckFileTime;
    m_isConfigChange = true;
    m_lockReadWrite.unlock();
}

// 停止线程
void LingmoUILog4qtRolling::stop()
{
    m_isExit = true;
}

// 检查文件数量
void LingmoUILog4qtRolling::checkLogFilesCount()
{
    if (m_strBaseFilePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(m_strBaseFilePath);
    if (!fileInfo.exists()) {
        return;
    }
    QDir dir(fileInfo.path());
    if (!dir.exists()) {
        return ;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QStringList filters;
    filters<<fileInfo.fileName()+"*";
    dir.setNameFilters(filters);        // 过滤名称
    dir.setSorting(QDir::Time);         // 按修改时间排序 由新到旧
    QFileInfoList fileInfoList = dir.entryInfoList();
    int nFileCount = fileInfoList.count();
    if (nFileCount <= 0 || nFileCount <= (int)m_uMaxFileCount) {
        return;
    }
    int willRmFileCount = nFileCount - m_uMaxFileCount;
    for (int i = nFileCount-1; i >= 0; i--) {
        if (willRmFileCount <= 0) {
            break;
        }
        QString filePath = fileInfoList[i].absoluteFilePath();
        KyDebug()<<"Will Remove File:"<<filePath;
        if (filePath != m_strBaseFilePath) {
            QFile file(filePath);
            file.remove();  // 删除文件
            willRmFileCount --;
        }
    }
}

// 检查文件大小
void LingmoUILog4qtRolling::checkLogFilesSize()
{
    if (m_strBaseFilePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(m_strBaseFilePath);
    if (!fileInfo.exists()) {
        return;
    }

    QDir dir(fileInfo.path());
    if (!dir.exists()) {
        return ;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QStringList filters;
    filters<<fileInfo.fileName()+"*";   // 过滤名称
    dir.setNameFilters(filters);        // 按修改时间排序 由新到旧
    dir.setSorting(QDir::Time);
    QFileInfoList fileInfoList = dir.entryInfoList();
    quint64 uTotalFileSize = 0;
    for (int i = 0; i < fileInfoList.count(); i++) {
        uTotalFileSize += fileInfoList[i].size();
    }
    if (uTotalFileSize <= 0 || uTotalFileSize <= m_uMaxFileSize) {
        return;
    }
    qint64 willRmFileSize = uTotalFileSize - m_uMaxFileSize;
    for (int i = fileInfoList.count()-1; i >= 0; i--) {
        if (willRmFileSize <= 0) {
            break;
        }
        QString filePath = fileInfoList[i].absoluteFilePath();
        KyDebug()<<"Will Remove File:"<<filePath;
        if (filePath != m_strBaseFilePath) {
            QFile file(filePath);
            file.remove();  // 删除文件
            willRmFileSize -= fileInfoList[i].size();
        } else {    // 清空文件内容
            //KyDebug()<<"清空文件内容："<<filePath;
            QProcess process;
            process.start("sh", QStringList()<<"-c"<<QString("> %1").arg(filePath));
            process.waitForFinished(100);
        }
    }
}