// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGVIEWERSERVICE_H
#define LOGVIEWERSERVICE_H

#include <dgiomount.h>

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>
#include <QProcess>
#include <QTemporaryDir>

class QTextStream;
class DGioVolumeManager;
class LogViewerService : public QObject
    , protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.logviewer")
public:
    explicit LogViewerService(QObject *parent = nullptr);
    ~LogViewerService();

Q_SIGNALS:

public Q_SLOTS:
    Q_SCRIPTABLE QString readLog(const QString &filePath);
    // 获取指定行数范围的日志内容
    Q_SCRIPTABLE QStringList readLogLinesInRange(const QString &filePath, qint64 startLine, qint64 lineCount, bool bReverse);
    Q_SCRIPTABLE int exitCode();
    Q_SCRIPTABLE void quit();
    Q_SCRIPTABLE QStringList getFileInfo(const QString &file, bool unzip = true);
    Q_SCRIPTABLE QStringList getOtherFileInfo(const QString &file, bool unzip = true);
    Q_SCRIPTABLE bool exportLog(const QString &outDir, const QString &in, bool isFile);
    Q_SCRIPTABLE QString openLogStream(const QString &filePath);
    Q_SCRIPTABLE QString readLogInStream(const QString &token);
    Q_SCRIPTABLE bool isFileExist(const QString &filePath);
    Q_SCRIPTABLE quint64 getFileSize(const QString &filePath);
    Q_SCRIPTABLE qint64 getLineCount(const QString &filePath);
    // 仅能执行特定合法命令
    Q_SCRIPTABLE QString executeCmd(const QString &cmd);
    Q_SCRIPTABLE QStringList whiteListOutPaths();

public:
    // 获取用户家目录
    QStringList getHomePaths();
    // 获取外设挂载路径(包括smb路径)
    QStringList getExternalDevPaths();
    QList<QExplicitlySharedDataPointer<DGioMount>> getMounts_safe();

private:
    // 清理临时目录下一些缓存文件，如解压后dump文件等（前端可能没权限删除，因此统一放到后端清理）
    void clearTempFiles();

    // 通过缓存块方式快速定位到指定行行首
    qint64 findLineStartOffsetWithCaching(const QString &filePath, qint64 targetLine);

    qint64 readFileAndReturnIndex(const QString &filePath, qint64 startLine, QList<uint64_t>& lineIndexes, bool reverseOrder);

private:
    QTemporaryDir tmpDir;
    QProcess m_process;
    QString tmpDirPath;
    QMap<QString, QString> m_commands;
    QMap<QString, std::pair<QString, QTextStream*>> m_logMap;
    QMap<QString, QList<uint64_t>> m_logLineIndex;
    /**
     * @brief isValidInvoker 检验调研者是否是日志
     * @return
     */
    bool isValidInvoker();
};

#endif // LOGVIEWERSERVICE_H
