/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bridgesetting.h"
#include "bridgesetting_p.h"

#define NM_SETTING_BRIDGE_INTERFACE_NAME "interface-name"

#include <QDebug>

NetworkManager::BridgeSettingPrivate::BridgeSettingPrivate()
    : name(NM_SETTING_BRIDGE_SETTING_NAME)
    , multicastSnooping(true)
    , stp(true)
    , priority(128)
    , forwardDelay(15)
    , helloTime(2)
    , maxAge(20)
    , agingTime(300)
{
}

NetworkManager::BridgeSetting::BridgeSetting()
    : Setting(Setting::Bridge)
    , d_ptr(new BridgeSettingPrivate())
{
}

NetworkManager::BridgeSetting::BridgeSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new BridgeSettingPrivate())
{
    setInterfaceName(other->interfaceName());
    setStp(other->stp());
    setPriority(other->priority());
    setForwardDelay(other->forwardDelay());
    setHelloTime(other->helloTime());
    setMaxAge(other->maxAge());
    setAgingTime(other->agingTime());
    setMulticastSnooping(other->multicastSnooping());
    setMacAddress(other->macAddress());
}

NetworkManager::BridgeSetting::~BridgeSetting()
{
    delete d_ptr;
}

QString NetworkManager::BridgeSetting::name() const
{
    Q_D(const BridgeSetting);

    return d->name;
}

void NetworkManager::BridgeSetting::setInterfaceName(const QString &name)
{
    Q_D(BridgeSetting);

    d->interfaceName = name;
}

QString NetworkManager::BridgeSetting::interfaceName() const
{
    Q_D(const BridgeSetting);

    return d->interfaceName;
}

void NetworkManager::BridgeSetting::setStp(bool enabled)
{
    Q_D(BridgeSetting);

    d->stp = enabled;
}

bool NetworkManager::BridgeSetting::stp() const
{
    Q_D(const BridgeSetting);

    return d->stp;
}

void NetworkManager::BridgeSetting::setPriority(quint32 priority)
{
    Q_D(BridgeSetting);

    d->priority = priority;
}

quint32 NetworkManager::BridgeSetting::priority() const
{
    Q_D(const BridgeSetting);

    return d->priority;
}

void NetworkManager::BridgeSetting::setForwardDelay(quint32 delay)
{
    Q_D(BridgeSetting);

    d->forwardDelay = delay;
}

quint32 NetworkManager::BridgeSetting::forwardDelay() const
{
    Q_D(const BridgeSetting);

    return d->forwardDelay;
}

void NetworkManager::BridgeSetting::setHelloTime(quint32 time)
{
    Q_D(BridgeSetting);

    d->helloTime = time;
}

quint32 NetworkManager::BridgeSetting::helloTime() const
{
    Q_D(const BridgeSetting);

    return d->helloTime;
}

void NetworkManager::BridgeSetting::setMaxAge(quint32 age)
{
    Q_D(BridgeSetting);

    d->maxAge = age;
}

quint32 NetworkManager::BridgeSetting::maxAge() const
{
    Q_D(const BridgeSetting);

    return d->maxAge;
}

void NetworkManager::BridgeSetting::setAgingTime(quint32 time)
{
    Q_D(BridgeSetting);

    d->agingTime = time;
}

quint32 NetworkManager::BridgeSetting::agingTime() const
{
    Q_D(const BridgeSetting);

    return d->agingTime;
}

void NetworkManager::BridgeSetting::setMulticastSnooping(bool snooping)
{
    Q_D(BridgeSetting);

    d->multicastSnooping = snooping;
}

bool NetworkManager::BridgeSetting::multicastSnooping() const
{
    Q_D(const BridgeSetting);

    return d->multicastSnooping;
}

void NetworkManager::BridgeSetting::setMacAddress(const QByteArray &address)
{
    Q_D(BridgeSetting);

    d->macAddress = address;
}

QByteArray NetworkManager::BridgeSetting::macAddress() const
{
    Q_D(const BridgeSetting);

    return d->macAddress;
}

void NetworkManager::BridgeSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_INTERFACE_NAME))) {
        setInterfaceName(setting.value(QLatin1String(NM_SETTING_BRIDGE_INTERFACE_NAME)).toString());
    }
    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_STP))) {
        setStp(setting.value(QLatin1String(NM_SETTING_BRIDGE_STP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_PRIORITY))) {
        setPriority(setting.value(QLatin1String(NM_SETTING_BRIDGE_PRIORITY)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_FORWARD_DELAY))) {
        setForwardDelay(setting.value(QLatin1String(NM_SETTING_BRIDGE_FORWARD_DELAY)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_HELLO_TIME))) {
        setHelloTime(setting.value(QLatin1String(NM_SETTING_BRIDGE_HELLO_TIME)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_MAX_AGE))) {
        setMaxAge(setting.value(QLatin1String(NM_SETTING_BRIDGE_MAX_AGE)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_AGEING_TIME))) {
        setAgingTime(setting.value(QLatin1String(NM_SETTING_BRIDGE_AGEING_TIME)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_MULTICAST_SNOOPING))) {
        setMulticastSnooping(setting.value(QLatin1String(NM_SETTING_BRIDGE_MULTICAST_SNOOPING)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BRIDGE_MAC_ADDRESS))) {
        setMacAddress(setting.value(QLatin1String(NM_SETTING_BRIDGE_MAC_ADDRESS)).toByteArray());
    }
}

QVariantMap NetworkManager::BridgeSetting::toMap() const
{
    QVariantMap setting;

    if (!interfaceName().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_INTERFACE_NAME), interfaceName());
    }
    if (!stp()) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_STP), stp());
    }

    if (priority() != 128) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_PRIORITY), priority());
    }

    if (forwardDelay() != 15) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_FORWARD_DELAY), forwardDelay());
    }

    if (helloTime() != 2) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_HELLO_TIME), helloTime());
    }

    if (maxAge() != 20) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_MAX_AGE), maxAge());
    }

    if (agingTime() != 300) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_AGEING_TIME), agingTime());
    }

    if (!multicastSnooping()) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_MULTICAST_SNOOPING), multicastSnooping());
    }

    if (!macAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_BRIDGE_MAC_ADDRESS), macAddress());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::BridgeSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_BRIDGE_INTERFACE_NAME << ": " << setting.interfaceName() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_STP << ": " << setting.stp() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_PRIORITY << ": " << setting.priority() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_FORWARD_DELAY << ": " << setting.forwardDelay() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_HELLO_TIME << ": " << setting.helloTime() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_MAX_AGE << ": " << setting.maxAge() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_AGEING_TIME << ": " << setting.agingTime() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_MULTICAST_SNOOPING << ": " << setting.multicastSnooping() << '\n';
    dbg.nospace() << NM_SETTING_BRIDGE_MAC_ADDRESS << ": " << setting.macAddress() << '\n';

    return dbg.maybeSpace();
}
