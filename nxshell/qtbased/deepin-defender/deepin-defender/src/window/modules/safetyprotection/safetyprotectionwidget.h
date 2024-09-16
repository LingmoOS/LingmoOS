// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAFETYPROTECTIONWIDGET_H
#define SAFETYPROTECTIONWIDGET_H

#include "../src/widgets/safetyprotectionitem.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSpacerItem>
#include <QVBoxLayout>

class DBusInvokerInterface;
class SettingsInvokerInterface;
class InvokerFactoryInterface;

class SafetyProtectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SafetyProtectionWidget(InvokerFactoryInterface *invokerFactory, QWidget *parent = nullptr);
    ~SafetyProtectionWidget();

private:
    // 初始化界面
    void initUI();

    // 初始化信号
    void initSignalSLot();

    // 初始化安全防护每一项开关状态
    void initSwitchStatus();

private Q_SLOTS:
    // USB存储设备保护状态改变
    void doUsbItemStatusChange();

    // 改变远程端口登陆状态
    void doRemRegisterStatusChange();
    // 当ssh状态改变时
    void recSSHStatus(bool status);

    // 远程访问控制状态改变
    void doRemControlStatusChange();

    // 联网控制状态改变
    void doNetControlStatusChange();

    // 流量详情状态改变
    void doDataUsageStatusChange();

    // USB存储设备保护标题点击槽
    void doUsbTextClicked();
    // 远程访问控制标题点击槽
    void doRemControlTextClicked();
    // 联网管控标题点击槽
    void doNetControlTextClicked();
    // 流量详情标题点击槽
    void doDataUsageTextClicked();
    // 安全日志
    void writeSecurityLog(const QString &) const;

private:
    SafetyProtectionItem *m_usbItem;
    SafetyProtectionItem *m_remRegisterItem;
    SafetyProtectionItem *m_remControlItem;
    SafetyProtectionItem *m_netControlItem;
    SafetyProtectionItem *m_dataUsageItem;

    SettingsInvokerInterface *m_gsetting; // 安全中心gsetting配置
    DBusInvokerInterface *m_monitorInterFaceServer; // 流量监控dbus类
    DBusInvokerInterface *m_dataInterFaceServer; // 安全中心数据服务接口
    DBusInvokerInterface *m_hmiScreen; // 主界面服务
};

#endif // SAFETYPROTECTIONWIDGET_H
