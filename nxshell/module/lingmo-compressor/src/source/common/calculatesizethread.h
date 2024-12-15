// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CALCULATESIZETHREAD_H
#define CALCULATESIZETHREAD_H

#include "commonstruct.h"
#include <QThread>

/**
 * @brief The CalculateSizeThread class
 * 多线程递归计算文件(夹)大小
 */
class CalculateSizeThread: public QThread
{
    Q_OBJECT
public:
    explicit CalculateSizeThread(const QStringList &listfiles, const QString &strArchiveFullPath, const QList<FileEntry> &listAddEntry, const CompressOptions &stOptions, QObject *parent = nullptr);
    void set_thread_stop(bool thread_stop);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    void ConstructAddOptionsByThread(const QString &path);

Q_SIGNALS:
    void signalFinishCalculateSize(qint64 size, QString strArchiveFullPath, QList<FileEntry> listAddEntry, CompressOptions stOptions, QList<FileEntry> listAllEntry);
    void signalError(const QString &strError, const QString &strToolTip);

private:
    QStringList m_files;                // 添加的源文件数据（首层）
    QString m_strArchiveFullPath;       // 压缩包全路径（首层）
    QList<FileEntry> m_listAddEntry;    // 添加的entry数据
    CompressOptions m_stOptions;        // 压缩参数
    qint64 m_qTotalSize = 0;            // 文件总大小
    QList<FileEntry> m_listAllEntry;    // 所有文件数据
    bool m_thread_stop = false;         // 结束线程
};

#endif // CALCULATESIZETHREAD_H
