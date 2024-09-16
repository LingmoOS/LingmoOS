// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"
#include "defenderprocinfolist.h"
#include "csingleteon.h"
#include "../src/window/modules/common/common.h"

#include <QObject>
#include <QMap>

#define Work_Speed_Debug 0
#define Work_Flow_Debug 0

class DBusInvokerInterface;
class SettingsInvokerInterface;
class InvokerFactoryInterface;

DEF_NAMESPACE_BEGIN
class DataUsageModel : public QObject
{
    Q_OBJECT
public:
    explicit DataUsageModel(InvokerFactoryInterface *invokerFactory, QObject *parent = nullptr);

    // 将speedwidget中的DBUS、GSETTINGS调用移到此处
    QString GetNetControlAppStatus() const;
    bool GetDataUsageSwitchStatus() const;
    void SetNetAppStatusChange(const QString &, int) const;
    void AddSecurityLog(const QString &) const;

    DefenderProcInfoList getProcInfoList() const;
    // 异步方式获取某个app流量数据
    void asyncGetAppFlowList(const QString &pkgName, const TimeRangeType &timeRangeType);
    // 异步方式获取所有app流量数据
    void asyncGetAllAppFlowList(const TimeRangeType &timeRangeType);

Q_SIGNALS:
    void sendProcInfoList(const DefenderProcInfoList &procInfos);
    // 向应用流量详情对话框实例发送显示的数据列表
    void sendAppFlowList(const QString &pkgName, const int timeRangeType, const DefenderProcInfoList &flowLst);
    // 发送来自服务的流量排名应用数据
    void sendAllAppFlowList(const int timeRangeType, const DefenderProcInfoList &flowLst);

public slots:
    void recProcInfoList(const DefenderProcInfoList &procInfos);

private:
    DBusInvokerInterface *m_monitorInterFaceServer;
    DBusInvokerInterface *m_dataInterFaceServer;
    SettingsInvokerInterface *m_gsNetControl;

    DefenderProcInfoList m_procInfoList;
};
DEF_NAMESPACE_END
