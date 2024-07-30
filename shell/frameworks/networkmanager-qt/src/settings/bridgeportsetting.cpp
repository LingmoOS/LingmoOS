/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bridgeportsetting.h"
#include "bridgeportsetting_p.h"

#include <QDebug>

NetworkManager::BridgePortSettingPrivate::BridgePortSettingPrivate()
    : name(NM_SETTING_BRIDGE_PORT_SETTING_NAME)
    , priority(32)
    , pathCost(100)
    , hairpinMode(false)
{
}

NetworkManager::BridgePortSetting::BridgePortSetting()
    : Setting(Setting::BridgePort)
    , d_ptr(new BridgePortSettingPrivate())
{
}

NetworkManager::BridgePortSetting::BridgePortSetting(const NetworkManager::BridgePortSetting::Ptr &other)
    : Setting(other)
    , d_ptr(new BridgePortSettingPrivate())
{
    setPriority(other->priority());
    setPathCost(other->pathCost());
    setHairpinMode(other->hairpinMode());
}

NetworkManager::BridgePortSetting::~BridgePortSetting()
{
    delete d_ptr;
}

QString NetworkManager::BridgePortSetting::name() const
{
    Q_D(const BridgePortSetting);

    return d->name;
}

void NetworkManager::BridgePortSetting::setPriority(quint32 priority)
{
    Q_D(BridgePortSetting);

    d->priority = priority;
}

quint32 NetworkManager::BridgePortSetting::priority() const
{
    Q_D(const BridgePortSetting);

    return d->priority;
}

void NetworkManager::BridgePortSetting::setPathCost(quint32 cost)
{
    Q_D(BridgePortSetting);

    d->pathCost = cost;
}

quint32 NetworkManager::BridgePortSetting::pathCost() const
{
    Q_D(const BridgePortSetting);

    return d->pathCost;
}

void NetworkManager::BridgePortSetting::setHairpinMode(bool enable)
{
    Q_D(BridgePortSetting);

    d->hairpinMode = enable;
}

bool NetworkManager::BridgePortSetting::hairpinMode() const
{
    Q_D(const BridgePortSetting);

    return d->hairpinMode;
}

void NetworkManager::BridgePortSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_PORT_PRIORITY))) {
        setPriority(setting.value(QLatin1String(NM_SETTING_BRIDGE_PORT_PRIORITY)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_PORT_PATH_COST))) {
        setPathCost(setting.value(QLatin1String(NM_SETTING_BRIDGE_PORT_PATH_COST)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_PORT_HAIRPIN_MODE))) {
        setHairpinMode(setting.value(QLatin1String(NM_SETTING_BRIDGE_PORT_HAIRPIN_MODE)).toBool());
    }
}

QVariantMap NetworkManager::BridgePortSetting::toMap() const
{
    QVariantMap setting;

    if (priority() != 32) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_PORT_PRIORITY), priority());
    }

    if (pathCost() != 100) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_PORT_PATH_COST), pathCost());
    }

    if (hairpinMode()) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_PORT_PATH_COST), hairpinMode());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::BridgePortSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_BRIDGE_PORT_PRIORITY << ": " << setting.priority() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_PORT_PATH_COST << ": " << setting.pathCost() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_PORT_PATH_COST << ": " << setting.hairpinMode() << '\n';

    return dbg.maybeSpace();
}
