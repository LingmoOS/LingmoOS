// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defenderprocinfolist.h"

#include <QObject>

class GetProcInfoJob : public QObject
{
    Q_OBJECT
public:
    explicit GetProcInfoJob();

public:
    ~GetProcInfoJob();

Q_SIGNALS:
    //
    void sendInstalledAppInfoList(const DefenderProcInfoList &list);
    // 发送进程信息列表
    void sendProcInfos(const DefenderProcInfoList &list);

public slots:
    // 更新启动器应用信息列表
    void updateLauncherItemInfoList();
    // 更新进程信息列表
    void updateProInfoList(const DefenderProcInfoList &list);

private:
    // 获取系统语言
    QString getSysLanguage() const;
    // 通过pid获取进程执行路径
    QString getProcExecPath(pid_t pid) const;
    // 通过pid获取进程comm名称
    QString getProcCommName(pid_t pid) const;

    // 从进程环境变量中获取环境信息
    QString getProcEnvProperties(pid_t pid, QString sEvnKey);
    // 通过执行路径获取包名, 从/usr/share/deepin-app-store/cache.db数据库里直接读取包名
    QString getPkgNameByExecPath(QString sExecPath);
    // 通过执行路径获取进程信息
    DefenderProcInfo getProcInfoByExecPath(QString sExecPath);

    // 通过包名，从已安装列表中获取进程信息
    DefenderProcInfo getProcInfoByPkgName(QString sPkgName);

private:
    // 进程信息列表
    DefenderProcInfoList m_procInfoList;
    // 已安装应用信息列表
    DefenderProcInfoList m_installedAppInfoList;
};
