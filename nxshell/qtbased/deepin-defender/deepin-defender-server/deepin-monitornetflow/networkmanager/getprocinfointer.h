// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defenderprocinfolist.h"

#include <QObject>

class GetProcInfoJob;

class GetProcInfoInter : public QObject
{
    Q_OBJECT
public:
    explicit GetProcInfoInter();

public:
    ~GetProcInfoInter();

    // 通过执行路径获取包名, 从/usr/share/deepin-app-store/cache.db数据库里直接读取包名
    QString getPkgNameByExecPath(const QString &execPath);
    // 通过包名，从已安装列表中获取进程信息
    DefenderProcInfo getProcInfoByPkgName(const QString &pkgName);

Q_SIGNALS:
    void sendPocNetFlowInfos(const DefenderProcInfoList &list);
    // 发送进程信息列表
    void sendProcInfos(const DefenderProcInfoList &list);
    // 已安装应用列表改变信号
    void installedAppListChanged();

public slots:
    void recInstalledAppInfos(const DefenderProcInfoList &list);
    void recProcInfos(const DefenderProcInfoList &list);

private:
private:
    // 进程信息列表
    DefenderProcInfoList m_procInfoList;
    // 已安装应用信息列表
    DefenderProcInfoList m_installedAppInfoList;
    GetProcInfoJob *m_getProcInfoJob;
    QThread *m_getProcInfoJobThread;
};
