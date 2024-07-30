/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vlansetting.h"
#include "vlansetting_p.h"

// define the deprecated&dropped values
#define NM_SETTING_VLAN_INTERFACE_NAME "interface-name"

#include <QDebug>

NetworkManager::VlanSettingPrivate::VlanSettingPrivate()
    : name(NM_SETTING_VLAN_SETTING_NAME)
    , id(0)
    , flags(VlanSetting::None)
{
}

NetworkManager::VlanSetting::VlanSetting()
    : Setting(Setting::Vlan)
    , d_ptr(new VlanSettingPrivate())
{
}

NetworkManager::VlanSetting::VlanSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new VlanSettingPrivate())
{
    setInterfaceName(other->interfaceName());
    setParent(other->parent());
    setId(other->id());
    setFlags(other->flags());
    setIngressPriorityMap(other->ingressPriorityMap());
    setEgressPriorityMap(other->egressPriorityMap());
}

NetworkManager::VlanSetting::~VlanSetting()
{
    delete d_ptr;
}

QString NetworkManager::VlanSetting::name() const
{
    Q_D(const VlanSetting);

    return d->name;
}

void NetworkManager::VlanSetting::setInterfaceName(const QString &name)
{
    Q_D(VlanSetting);

    d->interfaceName = name;
}

QString NetworkManager::VlanSetting::interfaceName() const
{
    Q_D(const VlanSetting);

    return d->interfaceName;
}

void NetworkManager::VlanSetting::setParent(const QString &parent)
{
    Q_D(VlanSetting);

    d->parent = parent;
}

QString NetworkManager::VlanSetting::parent() const
{
    Q_D(const VlanSetting);

    return d->parent;
}

void NetworkManager::VlanSetting::setId(quint32 id)
{
    Q_D(VlanSetting);

    d->id = id;
}

quint32 NetworkManager::VlanSetting::id() const
{
    Q_D(const VlanSetting);

    return d->id;
}

void NetworkManager::VlanSetting::setFlags(NetworkManager::VlanSetting::Flags flags)
{
    Q_D(VlanSetting);

    d->flags = flags;
}

NetworkManager::VlanSetting::Flags NetworkManager::VlanSetting::flags() const
{
    Q_D(const VlanSetting);

    return d->flags;
}

void NetworkManager::VlanSetting::setIngressPriorityMap(const QStringList &map)
{
    Q_D(VlanSetting);

    d->ingressPriorityMap = map;
}

QStringList NetworkManager::VlanSetting::ingressPriorityMap() const
{
    Q_D(const VlanSetting);

    return d->ingressPriorityMap;
}

void NetworkManager::VlanSetting::setEgressPriorityMap(const QStringList &map)
{
    Q_D(VlanSetting);

    d->egressPriorityMap = map;
}

QStringList NetworkManager::VlanSetting::egressPriorityMap() const
{
    Q_D(const VlanSetting);

    return d->egressPriorityMap;
}

void NetworkManager::VlanSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_VLAN_INTERFACE_NAME))) {
        setInterfaceName(setting.value(QLatin1String(NM_SETTING_VLAN_INTERFACE_NAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VLAN_PARENT))) {
        setParent(setting.value(QLatin1String(NM_SETTING_VLAN_PARENT)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VLAN_ID))) {
        setId(setting.value(QLatin1String(NM_SETTING_VLAN_ID)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VLAN_FLAGS))) {
        setFlags((Flag)setting.value(QLatin1String(NM_SETTING_VLAN_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VLAN_INGRESS_PRIORITY_MAP))) {
        setIngressPriorityMap(setting.value(QLatin1String(NM_SETTING_VLAN_INGRESS_PRIORITY_MAP)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VLAN_EGRESS_PRIORITY_MAP))) {
        setEgressPriorityMap(setting.value(QLatin1String(NM_SETTING_VLAN_EGRESS_PRIORITY_MAP)).toStringList());
    }
}

QVariantMap NetworkManager::VlanSetting::toMap() const
{
    QVariantMap setting;

    if (!interfaceName().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VLAN_INTERFACE_NAME), interfaceName());
    }

    if (!parent().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VLAN_PARENT), parent());
    }

    if (id()) {
        setting.insert(QLatin1String(NM_SETTING_VLAN_ID), id());
    }

    if (flags() != None) {
        setting.insert(QLatin1String(NM_SETTING_VLAN_FLAGS), (int)flags());
    }

    if (!ingressPriorityMap().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VLAN_INGRESS_PRIORITY_MAP), ingressPriorityMap());
    }

    if (!egressPriorityMap().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VLAN_EGRESS_PRIORITY_MAP), egressPriorityMap());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::VlanSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_VLAN_INTERFACE_NAME << ": " << setting.interfaceName() << '\n';
    dbg.nospace() << NM_SETTING_VLAN_PARENT << ": " << setting.parent() << '\n';
    dbg.nospace() << NM_SETTING_VLAN_ID << ": " << setting.id() << '\n';
    dbg.nospace() << NM_SETTING_VLAN_FLAGS << ": " << setting.flags() << '\n';
    dbg.nospace() << NM_SETTING_VLAN_INGRESS_PRIORITY_MAP << ": " << setting.ingressPriorityMap() << '\n';
    dbg.nospace() << NM_SETTING_VLAN_EGRESS_PRIORITY_MAP << ": " << setting.egressPriorityMap() << '\n';

    return dbg.maybeSpace();
}
