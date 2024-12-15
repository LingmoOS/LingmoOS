// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENFILEWATCHER_H
#define OPENFILEWATCHER_H

#include "commonstruct.h"

#include <QObject>

class QFileSystemWatcher;

// 压缩包打开文件监控
class OpenFileWatcher: public QObject
{
    Q_OBJECT
public:
    explicit OpenFileWatcher(QObject *parent);
    ~OpenFileWatcher();

    /**
     * @brief reset     重置
     */
    void reset();

    /**
     * @brief setCurOpenFile    设置当前打开的文件
     * @param strCurOpenFile    文件全路径
     */
    void setCurOpenFile(const QString &strCurOpenFile);

    /**
     * @brief addCurOpenWatchFile  添加文件监控
     */
    void addCurOpenWatchFile();

    /**
     * @brief addWatchFile      添加监控文件
     * @param strFile       文件名
     */
    void addWatchFile(const QString &strFile);

    /**
     * @brief getFileHasModified    获取被监控的文件状态
     * @return
     */
    QMap<QString, bool> &getFileHasModified();

    /**
     * @brief setFilePassword   设置文件密码
     * @param strPassword       密码
     */
    void setCurFilePassword(const QString &strPassword);

    /**
     * @brief getFilePassword   获取文件密码
     */
    QMap<QString, QString> getFilePassword();

Q_SIGNALS:
    void fileChanged(const QString &path);

private:
    QFileSystemWatcher *m_pOpenFileWatcher;       // 对打开的文件监控
    QString m_strOpenFile;                 // 最后一次打开的文件（真实全路径 用来添加到文件监控中）
    QMap<QString, bool> m_mapFileHasModified;   // 文件是否更改（包内文件路径 - 状态）
    QMap<QString, QString> m_mapFilePassword;   // 文件加密密码（包内文件路径 - 密码）

};

#endif // OPENFILEWATCHER_H
