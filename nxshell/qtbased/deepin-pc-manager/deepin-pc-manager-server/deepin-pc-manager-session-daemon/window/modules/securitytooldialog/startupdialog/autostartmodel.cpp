// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "autostartmodel.h"

#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerfactory.h"

#include <qglobal.h>

AutoStartModel::AutoStartModel(QObject *parent)
    : QObject(parent)
    , m_dataInterfaceInvoker(nullptr)
{
    init();
    initSignalSLot();
}

AutoStartModel::~AutoStartModel() { }

// 更新所有应用自启动状态
bool AutoStartModel::updateAppsAutoStartStatus() const
{
    // 更新所有应用自动动状态
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterfaceInvoker, "startLauncherManage");
    GET_MESSAGE_VALUE(bool, succeed, msg);
    return succeed;
}

// 获取所有自启动应用的信息字符串
QString AutoStartModel::getAllAutoStartAppsInfoStr() const
{
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterfaceInvoker, "getAppsInfoEnable");
    GET_MESSAGE_VALUE(QString, autoStartAppsInfoStr, msg);
    return autoStartAppsInfoStr;
}

// 获取所有非自启动应用的信息字符串
QString AutoStartModel::getAllNonautoStartAppsInfoStr() const
{
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterfaceInvoker, "getAppsInfoDisable");
    GET_MESSAGE_VALUE(QString, nonautoStartAppsInfoStr, msg);
    return nonautoStartAppsInfoStr;
}

// 应用是否自启动
bool AutoStartModel::isAppAutoStart(const QString &path) const
{
    // 判断该应用状态（是否是自启动）
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterfaceInvoker, "isAutostart", path);
    GET_MESSAGE_VALUE(bool, autoStart, msg);
    return autoStart;
}

// 设置应用自启动
bool AutoStartModel::setAppAutoStart(int status, const QString &path) const
{
    // 执行自启动开启/关闭操作
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_dataInterfaceInvoker, "exeAutostart", status, path);
    GET_MESSAGE_VALUE(bool, autoStart, msg);
    return autoStart;
}

// 添加日志
void AutoStartModel::addSecurityLog(int type, const QString &log) const
{
    Q_UNUSED(type);
    Q_UNUSED(log);
}

void AutoStartModel::init()
{
    m_dataInterfaceInvoker =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.session.daemon",
                                                    "/com/deepin/pc/manager/session/daemon",
                                                    "com.deepin.pc.manager.session.daemon",
                                                    ConnectType::SESSION,
                                                    this);
}

void AutoStartModel::initSignalSLot()
{
    // 后台自启动数据刷新信号连接
    m_dataInterfaceInvoker->Connect("AccessRefreshData",
                                    this,
                                    SLOT(itemStatusChanged(bool, QString)));
}
