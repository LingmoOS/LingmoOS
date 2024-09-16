// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "sysupdatermodel.h"
#include "window/modules/common/common.h"
#include "window/namespace.h"

#include <QDateTime>

class QGSettings;
class DBusInvokerInterface;
class SettingsInvokerInterface;

class HomePageModel : public QObject
{
    Q_OBJECT
public:
    explicit HomePageModel(QObject *parent = nullptr);

public:
    // 设置上次体检分数
    void setSafetyScore(const int &score);
    // 获取上次体检分数
    int GetSafetyScore();
    // 设置上次体检时间
    void SetLastCheckTime(QDateTime dateTime);
    // 获取上次体检时间
    QDateTime GetLastCheckTime();

    // 设置是否处于自启动app数检测中
    inline void SetAutoStartAppChecking(bool checking) { m_autoStartAppChecking = checking; }

    // 是否处于自启动app数检测中
    inline bool autoStartAppChecking() { return m_autoStartAppChecking; }

    // 打开自启动管理页
    void openStartupControlPage();
    // 检查自启动
    void checkAutoStart();
    // 是否忽略检测自启动项
    bool ignoreAutoStartChecking() const;

    // 设置是否忽略检测自启动项
    void setIgnoreAutoStartChecking(bool ignore);

    //// ssh检测
    // 异步获取ssh状态
    void asyncGetSSHStatus() const;

    // 异步设置ssh状态
    void asyncSetSSHStatus(bool status);

    //// 系统更新检测
    // 设置是否处于系统版本检测中
    inline void SetSysVersionChecking(bool checking) { m_sysVersionChecking = checking; }

    // 是否处于系统版本检测中
    inline bool sysVersionChecking() const { return m_sysVersionChecking; }

    // 打开系统更新界面
    void openSysUpdatePage();

    // 设置是否处于垃圾文件检测中
    inline void SetTrashChecking(bool checking) { m_trashChecking = checking; }

    // 磁盘管理器 是否已安装
    bool isDiskManagerInterValid();
    // 获取磁盘状态
    bool getDiskStatus();

    // 设置是否处于磁盘检测中
    inline void SetDiskChecking(bool checking) { m_diskChecking = checking; }

    // 设置是否处于正在检测开发者模式
    inline void SetDevelopModeChecking(bool checking) { m_developModeChecking = checking; }

    // 是否处于开发者模式
    bool IsDeveloperMode();
    // 是否忽略检测开发者模式
    bool ignoreDevModeChecking() const;
    // 设置是否忽略检测开发者模式
    void setIgnoreDevModeChecking(bool ignore);

public Q_SLOTS:
    // 更新自启动应用个数
    void updateStartUpAppCount(bool bAdd, const QString &sID);

    // 垃圾文件扫描完成时
    void onTrashScanFinished(double sum);

    // 添加安全日志
    void addSecurityLog(QString sInfo);

Q_SIGNALS:
    // 通知打开体检首页
    void notifyShowHomepage();

    // 当自启动检查结束时
    void checkAutoStartFinished(int count);
    // 自启动应用数量改变时
    void autoStartAppCountChanged(int count);

    // 发送ssh状态
    void sendSSHStatus(bool status);

    // 通知去检查系统更新
    void NotifyCheckSysUpdate();
    // 通知去停止检查系统更新
    void notifyStopCheckSysUpdate();
    void notifyTimeroutSysUpdate(const QString &status);
    // 检查系统更新完成
    void CheckSysUpdateFinished(bool canUpdate);

    // 请求开始垃圾文件扫描
    void requestStartTrashScan();
    // 垃圾文件扫描完成时
    void trashScanFinished(const double sum);
    // 请求清理选中的垃圾文件
    void requestCleanSelectTrash();
    // 清理选中的垃圾文件完成时
    void cleanSelectTrashFinished();

private:
    // 获取允许自启动应用个数
    int getAutoStartAppCount() const;

private:
    // gsetting对象
    SettingsInvokerInterface *m_gSettingsInvokerInter;
    // 安全中心数据接口服务对象
    DBusInvokerInterface *m_defenderDataInvokerInter;
    // 控制中心dbus服务
    DBusInvokerInterface *m_controlCenterInvokerInter;
    // 安全中心界面dbus服务
    DBusInvokerInterface *m_defenderWindowInvokerInter;
    // 同步助手dbus服务，用于查询是否处于开发者模式
    DBusInvokerInterface *m_helperInvokerInter;
    // 磁盘管理dbus服务
    DBusInvokerInterface *m_diskManagerInvokerInter;
    // 流量监控系统dbus服务
    DBusInvokerInterface *m_netFlowMonitorInvokerInter;
    DBusInvokerInterface *m_langSelectInvokerInter;
    // 自启动服务
    DBusInvokerInterface *m_securityCenterInter;

    // 系统更新数据处理对象
    SysUpdaterModel *m_sysUpdaterModel;
    // 是否正在进行检查自启动应用数
    bool m_autoStartAppChecking;
    // 是否正在进行检查系统版本
    bool m_sysVersionChecking;
    // 是否正在进行检测垃圾文件
    bool m_trashChecking;
    // 是否正在进行磁盘检测
    bool m_diskChecking;
    // 是否正在进行检查开发者模式
    bool m_developModeChecking;
};
