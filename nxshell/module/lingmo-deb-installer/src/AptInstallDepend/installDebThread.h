// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALLDEBTHREAD_H
#define INSTALLDEBTHREAD_H

#include <QThread>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCommandLineParser>

#include "kprocess.h"

#define TEMPLATE_DIR "/tmp/DEBIAN_TMP"
#define TEMPLATE_PATH "/tmp/DEBIAN_TMP/templates"

class InstallDebThread : public QThread
{
    Q_OBJECT

public:
    enum Command {
        Install = 1 << 1,  // common process
        Remove = 1 << 2,

        InstallWine = 1 << 3,
        InstallConfig = 1 << 4,  // DebConf install

        Compatible = 1 << 5,   // compatible mode
        Immutable = 1 << 6,    // immutable system
        LinglongUab = 1 << 7,  // linglong app(lingyaps)
    };
    Q_DECLARE_FLAGS(Commands, Command)
    Q_FLAG(Commands)

    InstallDebThread();
    virtual ~InstallDebThread();

    void setParam(const QStringList &arguments);
    void getDescription(const QString &debPath);

    void run();

    inline int retFlag() const { return m_resultFlag; }

public slots:
    void onFinished(int num, QProcess::ExitStatus exitStatus);
    void onReadoutput();

private:
    void installWine();
    void installConfig();
    void compatibleProcess();
    void immutableProcess();
    void uabProcessCli();

    // 使用软连接方式解决文件路径中存在空格的问题。
    QString SymbolicLink(const QString &previousName, const QString &packageName);
    QString link(const QString &linkPath, const QString &packageName);
    bool mkTempDir();
    bool rmTempDir();

    const QString m_tempLinkDir = "/tmp/LinkTemp/";

    QCommandLineParser m_parser;
    Commands m_cmds;
    QString m_rootfs;  // for comaptible mode : select rootfs

    KProcess *m_proc;
    QStringList m_listParam;
    QList<QString> m_listDescribeData;
    int m_resultFlag = -1;
};
#endif  // INSTALLDEBTHREAD_H
