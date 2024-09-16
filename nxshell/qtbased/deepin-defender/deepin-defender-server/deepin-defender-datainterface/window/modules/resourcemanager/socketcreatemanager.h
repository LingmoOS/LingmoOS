// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SOCKETCREATEMANAGER_H
#define SOCKETCREATEMANAGER_H

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

class SocketCreateManager : public QObject
{
    Q_OBJECT
public:
    explicit SocketCreateManager(QObject *parent = nullptr);
    ~SocketCreateManager();

public Q_SLOTS:
    // 接收联网管控连接请求消息信号槽
    void onAppNetworkConnect(const QStringList &subscribers, const QString &msg);
    // 刷新联网管控状态
    void refreshNetControlStatus();

    // 设置 联网/远程 开关状态
    void setNetControlSwitchStatus(bool isNetStatus);
    bool getNetControlSwitchStatus();
    // 得到联网管控所有APP信息
    void getNetControlAppInfos();
    // 联网管控列表 联网状态改变
    void setNetControlStatusChange(QString, int);
    // 联网管控列表 联网状态改变
    void setNetRequestReply(QString, QString, int);

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

    // 接收包名解析数据库插入数据完成信号槽
    void initNetControlLocalInfo();

    // 将数据集中数据保存到本地
    void setGsLocalData(QMap<QString, QString>);
    // 清除本地配置
    void reset();

    QString getPPid(QString sPid);

Q_SIGNALS:
    void sendNetAppsChange(QList<DefenderProcInfo>);

private:
    QGSettings *m_gsNetControl; // 联网管控本地保存项
    QMap<QString, QString> m_mapNetControl; // 联网管控App容器
    QMap<QString, QString> m_mapNetControlInfo; // 联网管控App临时容器
    QMap<QString, QString> m_mapNetControlAppName; // 联网管控Apps名称容
    QMap<QString, QString> m_mapPidMsg; // pid与msg容器

    ResourceManagerInter *m_resourceManagerInter;
    LauncherInter *m_launcherInter;
    MonitorNetFlow *m_monitornetflow;
    DataInterFace *m_dataInterFace;

    QList<QString> m_appsStartList;
    QList<QString> m_disableDialog; // 联网管控弹框禁止操作
    QList<QString> m_askDisableDialog; // 联网管控弹框禁止操作
    QList<QString> m_oneAllowDialog; // 联网管控弹框禁止操作

    bool m_isbNetControlSwitch;
    bool m_isbRegisterStatus;
    QString m_uuid;
};

#endif // SOCKETCREATEMANAGER_H
