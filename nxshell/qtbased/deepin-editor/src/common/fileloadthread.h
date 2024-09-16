// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILELOADTHREAD_H
#define FILELOADTHREAD_H

#include <QThread>

class FileLoadThread : public QThread
{
    Q_OBJECT
public:
    FileLoadThread(const QString &filepath, QObject *QObject = nullptr);
    ~FileLoadThread();

    void run();

signals:
    // 预处理信号，优先处理文件头，防止出现加载时间过长的情况
    void sigPreProcess(const QByteArray &encode, const QByteArray &content);
    void sigLoadFinished(const QByteArray &encode, const QByteArray &content, bool error = false);

private:
    QString m_strFilePath;
};

#endif
