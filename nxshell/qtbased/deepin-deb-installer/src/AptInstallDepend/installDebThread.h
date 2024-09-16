// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALLDEBTHREAD_H
#define INSTALLDEBTHREAD_H

// #include <QObject>
#include <QThread>
#include <QProcess>
#include <QFile>
#include <QDir>
#include "kprocess.h"

#define TEMPLATE_DIR "/tmp/DEBIAN_TMP"
#define TEMPLATE_PATH "/tmp/DEBIAN_TMP/templates"

class InstallDebThread : public QThread
{
    Q_OBJECT
public:
    InstallDebThread();
    virtual ~InstallDebThread();
    void setParam(const QStringList &tParam);
    void getDescription(const QString &debPath);
    void run();
    int m_resultFlag = -1;

public slots:
    void onFinished(int);
    void on_readoutput();

private:
    KProcess *m_proc;
    QStringList m_listParam;
    QList<QString> m_listDescribeData;

private:
    const QString m_tempLinkDir = "/tmp/LinkTemp/";
    // 使用软连接方式解决文件路径中存在空格的问题。
    QString SymbolicLink(const QString &previousName, const QString &packageName);
    QString link(const QString &linkPath, const QString &packageName);
    bool mkTempDir();
    bool rmTempDir();
};
#endif  // INSTALLDEBTHREAD_H
