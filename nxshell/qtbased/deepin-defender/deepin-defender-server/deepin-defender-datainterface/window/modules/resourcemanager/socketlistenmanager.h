// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SOCKETLISTENMANAGER_H
#define SOCKETLISTENMANAGER_H

#include "com_deepin_daemon_resourcemanager.h"
#include "com_deepin_dde_daemon_launcherd.h"
#include "monitornetflow_interface.h"
#include "datainterface_interface.h"

#include <QObject>
#include <QGSettings>

using ResourceManagerInter = com::deepin::daemon::ResourceManager;
using LauncherInter = com::deepin::dde::daemon::Launcher;
using MonitorNetFlow = com::deepin::defender::MonitorNetFlow;
using DataInterFace = com::deepin::defender::datainterface;

class SocketListenManager : public QObject
{
    Q_OBJECT
public:
    explicit SocketListenManager(QObject *parent = nullptr);
    ~SocketListenManager();

public Q_SLOTS:
    // 接收远程访问连接请求消息信号槽
    void onAppRemworkConnect(const QStringList &subscribers, const QString &msg);
    // 刷新远程访问状态
    void refreshRemControlStatus();

    // 设置/获取远程访问默认状态
    void setRemControlDefaultStatus(int);
    int getRemControlDefaultStatus();

    // 设置 联网/远程 开关状态
    void setRemControlSwitchStatus(bool isRemStatus);
    bool getRemControlSwitchStatus();
    // 远程访问列表 状态改变
    void setRemRequestReply(QString, QString, int);

    void doAppStartSlot(QStringList sAppList);
    void doAppEndSlot(QStringList sAppList);

private:
    // 通过pid获取进程的包名
    QString getPkgNameByPid(int pid);
    // 通过pid获取进程执行路径
    QString getProcExecPath(pid_t pid) const;
    // 从进程环境变量中获取环境信息
    QString getProcEnvProperties(pid_t pid, QString sEvnKey);
    // 通过执行路径获取包名
    QString getPkgNameByDesktopPath(QString sExecPath);

    QString getPPid(QString sPid);

private:
    QGSettings *m_gsRemControl; // 远程访问本地保存项
    QMap<QString, QString> m_mapPidMsg; // pid与msg容器

    ResourceManagerInter *m_resourceManagerInter;
    LauncherInter *m_launcherInter;
    MonitorNetFlow *m_monitornetflow;
    DataInterFace *m_dataInterFace;

    QMap<QString, QString> m_mapRemControlAppName; // 远程访问Apps名称容器
    QList<QString> m_remSystemApps; // 系统应用容器
    QList<QString> m_appsStartList;
    QList<QString> m_disableDialog; // 远程访问弹框禁止操作
    QList<QString> m_askDisableDialog; // 远程访问弹框禁止操作
    QList<QString> m_oneAllowDialog; // 远程访问弹框禁止操作

    bool m_isbRemControlSwitch;
    bool m_isbRegisterStatus;

    int m_nRemOption;
    QString m_uuid;
};

#endif // SOCKETLISTENMANAGER_H
