/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ovsportsetting.h"
#include "ovsportsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_PORT_SETTING_NAME "ovs-port"
#define NM_SETTING_OVS_PORT_VLAN_MODE "vlan-mode"
#define NM_SETTING_OVS_PORT_TAG "tag"
#define NM_SETTING_OVS_PORT_LACP "lacp"
#define NM_SETTING_OVS_PORT_BOND_MODE "bond-mode"
#define NM_SETTING_OVS_PORT_BOND_UPDELAY "bond-updelay"
#define NM_SETTING_OVS_PORT_BOND_DOWNDELAY "bond-downdelay"
#endif

NetworkManager::OvsPortSettingPrivate::OvsPortSettingPrivate()
    : name(NM_SETTING_OVS_PORT_SETTING_NAME)
    , bondDowndelay(0)
    , bondUpdelay(0)
    , tag(0)
{
}

NetworkManager::OvsPortSetting::OvsPortSetting()
    : Setting(Setting::OvsPort)
    , d_ptr(new OvsPortSettingPrivate())
{
}

NetworkManager::OvsPortSetting::OvsPortSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new OvsPortSettingPrivate())
{
    setBondDowndelay(other->bondDowndelay());
    setBondUpdelay(other->bondUpdelay());
    setTag(other->tag());
    setBondMode(other->bondMode());
    setLacp(other->lacp());
    setVlanMode(other->vlanMode());
}

NetworkManager::OvsPortSetting::~OvsPortSetting()
{
    delete d_ptr;
}

QString NetworkManager::OvsPortSetting::name() const
{
    Q_D(const OvsPortSetting);

    return d->name;
}

void NetworkManager::OvsPortSetting::setBondDowndelay(quint32 delay)
{
    Q_D(OvsPortSetting);

    d->bondDowndelay = delay;
}

quint32 NetworkManager::OvsPortSetting::bondDowndelay() const
{
    Q_D(const OvsPortSetting);

    return d->bondDowndelay;
}

void NetworkManager::OvsPortSetting::setBondUpdelay(quint32 delay)
{
    Q_D(OvsPortSetting);

    d->bondUpdelay = delay;
}

quint32 NetworkManager::OvsPortSetting::bondUpdelay() const
{
    Q_D(const OvsPortSetting);

    return d->bondUpdelay;
}

void NetworkManager::OvsPortSetting::setTag(quint32 tag)
{
    Q_D(OvsPortSetting);

    d->tag = tag;
}

quint32 NetworkManager::OvsPortSetting::tag() const
{
    Q_D(const OvsPortSetting);

    return d->tag;
}

void NetworkManager::OvsPortSetting::setBondMode(const QString &mode)
{
    Q_D(OvsPortSetting);

    d->bondMode = mode;
}

QString NetworkManager::OvsPortSetting::bondMode() const
{
    Q_D(const OvsPortSetting);

    return d->bondMode;
}

void NetworkManager::OvsPortSetting::setLacp(const QString &lacp)
{
    Q_D(OvsPortSetting);

    d->lacp = lacp;
}

QString NetworkManager::OvsPortSetting::lacp() const
{
    Q_D(const OvsPortSetting);

    return d->lacp;
}

void NetworkManager::OvsPortSetting::setVlanMode(const QString &mode)
{
    Q_D(OvsPortSetting);

    d->vlanMode = mode;
}

QString NetworkManager::OvsPortSetting::vlanMode() const
{
    Q_D(const OvsPortSetting);

    return d->vlanMode;
}

void NetworkManager::OvsPortSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_OVS_PORT_BOND_UPDELAY))) {
        setBondUpdelay(setting.value(QLatin1String(NM_SETTING_OVS_PORT_BOND_UPDELAY)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OVS_PORT_BOND_DOWNDELAY))) {
        setBondDowndelay(setting.value(QLatin1String(NM_SETTING_OVS_PORT_BOND_DOWNDELAY)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OVS_PORT_TAG))) {
        setTag(setting.value(QLatin1String(NM_SETTING_OVS_PORT_TAG)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OVS_PORT_BOND_MODE))) {
        setBondMode(setting.value(QLatin1String(NM_SETTING_OVS_PORT_BOND_MODE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OVS_PORT_LACP))) {
        setLacp(setting.value(QLatin1String(NM_SETTING_OVS_PORT_LACP)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OVS_PORT_VLAN_MODE))) {
        setVlanMode(setting.value(QLatin1String(NM_SETTING_OVS_PORT_VLAN_MODE)).toString());
    }
}

QVariantMap NetworkManager::OvsPortSetting::toMap() const
{
    QVariantMap setting;

    if (bondUpdelay() > 0) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PORT_BOND_UPDELAY), bondUpdelay());
    }

    if (bondDowndelay() > 0) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PORT_BOND_DOWNDELAY), bondDowndelay());
    }

    if (tag() > 0) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PORT_TAG), tag());
    }

    if (!bondMode().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PORT_BOND_MODE), bondMode());
    }

    if (!lacp().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PORT_LACP), lacp());
    }

    if (!vlanMode().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PORT_VLAN_MODE), vlanMode());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::OvsPortSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_OVS_PORT_BOND_UPDELAY << ": " << setting.bondUpdelay() << '\n';
    dbg.nospace() << NM_SETTING_OVS_PORT_BOND_DOWNDELAY << ": " << setting.bondDowndelay() << '\n';
    dbg.nospace() << NM_SETTING_OVS_PORT_TAG << ": " << setting.tag() << '\n';
    dbg.nospace() << NM_SETTING_OVS_PORT_BOND_MODE << ": " << setting.bondMode() << '\n';
    dbg.nospace() << NM_SETTING_OVS_PORT_LACP << ": " << setting.lacp() << '\n';
    dbg.nospace() << NM_SETTING_OVS_PORT_VLAN_MODE << ": " << setting.vlanMode() << '\n';

    return dbg.maybeSpace();
}
