// Copyright (C) 2022 ~ 2023 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netcheckmodel.h"
#include "window/modules/common/common.h"
#include "window/modules/common/gsettingkey.h"
#include "window/modules/common/invokers/invokerfactory.h"

NetCheckModel::NetCheckModel(QObject *parent)
    : QObject(parent)
    , m_netCheckDBusInvokerInter(nullptr)
    , m_netWorkDBusInvokerInter(nullptr)
    , m_gsetting(nullptr)
{
    // 初始化安全中心gsetting配置
    m_gsetting = InvokerFactory::GetInstance().CreateSettings(DEEPIN_PC_MANAGER_GSETTING_PATH, QByteArray(), this);

    m_netCheckDBusInvokerInter = InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.netcheck",
                                                                             "/com/deepin/pc/manager/netcheck",
                                                                             "com.deepin.pc.manager.netcheck",
                                                                             ConnectType::SYSTEM, this);
    m_netCheckDBusInvokerInter->Connect("NotifyCheckNetDevice", this, SLOT(acceptCheckNetDevice(int)));
    m_netCheckDBusInvokerInter->Connect("NotifyCheckNetConnSetting", this, SLOT(acceptCheckNetConnSetting(int)));
    m_netCheckDBusInvokerInter->Connect("NotifyCheckNetDHCP", this, SLOT(acceptCheckNetDHCP(int)));
    m_netCheckDBusInvokerInter->Connect("NotifyCheckNetDNS", this, SLOT(acceptCheckNetDNS(int)));
    m_netCheckDBusInvokerInter->Connect("NotifyCheckNetHost", this, SLOT(acceptCheckNetHost(int)));
    m_netCheckDBusInvokerInter->Connect("NotifyCheckNetConn", this, SLOT(acceptCheckNetConn(int)));

    m_netWorkDBusInvokerInter = InvokerFactory::GetInstance().CreateInvoker("com.deepin.daemon.Network",
                                                                            "/com/deepin/daemon/Network",
                                                                            "com.deepin.daemon.Network",
                                                                            ConnectType::SESSION, this);
}

NetCheckModel::~NetCheckModel()
{
}

// 初始化数据
void NetCheckModel::initData()
{
}

void NetCheckModel::startNetCheck(QString domainText)
{
    Q_EMIT sendNetCheckStatus(true);
    m_domainText = domainText;
    Q_EMIT notifyNetCheckReset();

    startCheckNetDevice();
}

// 获取/设置网络检测类型和内网域名/ip
int NetCheckModel::getNetCheckType()
{
    return m_gsetting->GetValue(NET_CHECK_TYPE).toInt();
}
void NetCheckModel::setNetCheckType(int type)
{
    m_gsetting->SetValue(NET_CHECK_TYPE, type);
}

QString NetCheckModel::getNetCheckIntranet()
{
    return m_gsetting->GetValue(NET_CHECK_INTRANET_DOMAIN).toString();
}
void NetCheckModel::setNetCheckIntranet(QString domain)
{
    m_gsetting->SetValue(NET_CHECK_INTRANET_DOMAIN, domain);
}

// 开始检测网络设备
void NetCheckModel::startCheckNetDevice()
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "StartCheckNetDevice");
}
// 开始检测网络设置
void NetCheckModel::startCheckNetSetting()
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "StartCheckNetConnSetting");
}
// 开始检测DHCP
void NetCheckModel::startCheckNetDHCP()
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "StartCheckNetDHCP");
}
// 开始检测DNS
void NetCheckModel::startCheckNetDNS()
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "StartCheckNetDNS");
}
// 开始检测host文件
void NetCheckModel::startCheckNetHostFile()
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "StartCheckNetHost");
}
// 开始检测网络连接访问
void NetCheckModel::startCheckNetConn()
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "StartCheckNetConn", m_domainText);
}

// 接收网络硬件检测结果
void NetCheckModel::acceptCheckNetDevice(int result)
{
    Q_EMIT notifyNetDeviceCheck(result);
}
// 接收网络连接配置检测结果
void NetCheckModel::acceptCheckNetConnSetting(int result)
{
    Q_EMIT notifyNetSettingCheck(result);
}
// 接收DHCP检测结果
void NetCheckModel::acceptCheckNetDHCP(int result)
{
    Q_EMIT notifyNetDHCPCheck(result);
}
// 接收DNS检测结果
void NetCheckModel::acceptCheckNetDNS(int result)
{
    Q_EMIT notifyNetDNSCheck(result);
}
// 接收host检测结果
void NetCheckModel::acceptCheckNetHost(int result)
{
    Q_EMIT notifyNetHostCheck(result);
}
// 接收网络连接访问检测结果
void NetCheckModel::acceptCheckNetConn(int result)
{
    Q_EMIT sendNetCheckStatus(false);
    Q_EMIT notifyNetConnCheck(result);
}

bool NetCheckModel::checkVPNOrProxyOpen()
{
    bool flag = false;

    QDBusMessage msgVPN = DBUS_BLOCK_INVOKE(m_netWorkDBusInvokerInter, DBUS_PROPERTY_INVOKER_NAME, "VpnEnabled");
    QVariant varVPN = Utils::checkAndTransQDbusVarIntoQVar(msgVPN.arguments().first());
    if (varVPN.toBool()) {
        flag = true;
    }

    QDBusMessage msgProxy = DBUS_BLOCK_INVOKE(m_netWorkDBusInvokerInter, "GetProxyMethod");
    GET_MESSAGE_VALUE(QString, status, msgProxy);
    if ("none" != status) {
        flag = true;
    }

    return flag;
}

bool NetCheckModel::getNetCheckStatus()
{
    bool status = false;
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_netCheckDBusInvokerInter, "GetNetCheckStatus");
    GET_MESSAGE_VALUE(bool, result, msg);
    status = result;

    return status;
}
void NetCheckModel::setNetCheckStatus(bool status)
{
    DBUS_NOBLOCK_INVOKE(m_netCheckDBusInvokerInter, "SetNetCheckStatus", status);
    Q_EMIT sendNetCheckStatus(false);
}
