// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemsettings.h"

#include "../invokers/invokerfactory.h"

#include <QDebug>

namespace def {
SystemSettings::SystemSettings(QObject *parent)
    : QObject(parent)
    , m_netMonitorDBusInvoker(nullptr)
{
    m_netMonitorDBusInvoker =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.MonitorNetFlow",
                                                    "/com/deepin/pc/manager/MonitorNetFlow",
                                                    "com.deepin.pc.manager.MonitorNetFlow",
                                                    ConnectType::SYSTEM,
                                                    this);

    m_netMonitorDBusInvoker->Connect("SysSettingValueChanged",
                                     this,
                                     SLOT(onValueChanged(QString, QDBusVariant)));
}

SystemSettings::~SystemSettings() { }

bool SystemSettings::setValue(const QString &key, const QVariant &value)
{
    // 装载到QDBusVariant
    QDBusVariant dbusVar(value);
    // 装载到QVariant
    QVariant var;
    var.setValue(dbusVar);
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_netMonitorDBusInvoker, "SetSysSettingValue", key, var);
    GET_MESSAGE_VALUE(bool, ret, msg);
    return ret;
}

QVariant SystemSettings::getValue(const QString &key)
{
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_netMonitorDBusInvoker, "GetSysSettingValue", key);
    GET_MESSAGE_VALUE(QDBusVariant, valueDBusVar, msg);
    return valueDBusVar.variant();
}

void SystemSettings::onValueChanged(const QString &key, const QDBusVariant &value)
{
    Q_EMIT valueChanged(key, value.variant());
}
} // namespace def
