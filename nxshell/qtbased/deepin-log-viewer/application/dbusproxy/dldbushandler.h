// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DLDBUSHANDLER_H
#define DLDBUSHANDLER_H

#include "dldbusinterface.h"
#include <QObject>

class DLDBusHandler : public QObject
{
    Q_OBJECT
public:
    static DLDBusHandler *instance(QObject *parent = nullptr);
    ~DLDBusHandler();
    QString readLog(const QString &filePath);
    QStringList readLogLinesInRange(const QString &filePath, qint64 startLine = 0, qint64 lineCount = 500, bool bReverse = true);
    QStringList getFileInfo(const QString &flag, bool unzip = true);
    QStringList getOtherFileInfo(const QString &flag, bool unzip = true);
    int exitCode();
    void quit();
    bool exportLog(const QString &outDir, const QString &in, bool isFile);
    bool isFileExist(const QString &filePath);
    quint64 getFileSize(const QString &filePath);
    qint64 getLineCount(const QString &filePath);
    QString executeCmd(const QString &cmd);
    QString openLogStream(const QString &filePath);
    QString readLogInStream(const QString &token);
    QStringList whiteListOutPaths();

private:
    explicit DLDBusHandler(QObject *parent = nullptr);

private:
    static DLDBusHandler *m_statichandeler;
    DeepinLogviewerInterface *m_dbus;
    QStringList filePath;
};

#endif // DLDBUSHANDLER_H
