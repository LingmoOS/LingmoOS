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

#ifndef __LINGMOLOG4QT_ROLLING_H__
#define __LINGMOLOG4QT_ROLLING_H__

#include "lingmo-logmacros.h"

#include <QThread>
#include <QReadWriteLock>

class LingmoUILog4qtRolling : public QThread
{
public:
    LingmoUILog4qtRolling(QString strBaseFilePath, unsigned uMaxFileCount = 0, quint64 uMaxFileSize = 0, 
        quint64 delayCheckFileTime = 0, QObject *parent = nullptr);
    virtual ~LingmoUILog4qtRolling();

    // 停止线程
    void stop();
    // 设置文件检查限制条件
    void setFileCheckLimit(unsigned uMaxFileCount = 0, quint64 uMaxFileSize = 0, quint64 delayCheckFileTime = 0);
    // 检查文件数量
    void checkLogFilesCount();
    // 检查文件大小
    void checkLogFilesSize();

protected:
    void run();

private:
    // 是否将线程退出
    bool m_isExit = true;
    // 间隔检查文件时间 单位（s）
    quint64 m_delayCheckFileTime = 0;
    // 当前基础文件路径
    QString m_strBaseFilePath;
    // 最大文件数限制
    unsigned m_uMaxFileCount = 0;
    // 最大文件大小限制
    quint64 m_uMaxFileSize = 0;
    // 数据保护锁
    QReadWriteLock m_lockReadWrite;
    // 配置是否修改
    bool m_isConfigChange = false;
};

#endif // __LINGMOLOG4QT_ROLLING_H__