/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wimaxsetting.h"
#include "wimaxsetting_p.h"

#include <QDebug>

NetworkManager::WimaxSettingPrivate::WimaxSettingPrivate()
    : name(NM_SETTING_WIMAX_SETTING_NAME)
{
}

NetworkManager::WimaxSetting::WimaxSetting()
    : Setting(Setting::Wimax)
    , d_ptr(new WimaxSettingPrivate())
{
}

NetworkManager::WimaxSetting::WimaxSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new WimaxSettingPrivate())
{
    setNetworkName(other->networkName());
    setMacAddress(other->macAddress());
}

NetworkManager::WimaxSetting::~WimaxSetting()
{
    delete d_ptr;
}

QString NetworkManager::WimaxSetting::name() const
{
    Q_D(const WimaxSetting);

    return d->name;
}

void NetworkManager::WimaxSetting::setNetworkName(const QString &name)
{
    Q_D(WimaxSetting);

    d->networkName = name;
}

QString NetworkManager::WimaxSetting::networkName() const
{
    Q_D(const WimaxSetting);

    return d->networkName;
}

void NetworkManager::WimaxSetting::setMacAddress(const QByteArray &address)
{
    Q_D(WimaxSetting);

    d->macAddress = address;
}

QByteArray NetworkManager::WimaxSetting::macAddress() const
{
    Q_D(const WimaxSetting);

    return d->macAddress;
}

void NetworkManager::WimaxSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_WIMAX_NETWORK_NAME))) {
        setNetworkName(setting.value(QLatin1String(NM_SETTING_WIMAX_NETWORK_NAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIMAX_MAC_ADDRESS))) {
        setMacAddress(setting.value(QLatin1String(NM_SETTING_WIMAX_MAC_ADDRESS)).toByteArray());
    }
}

QVariantMap NetworkManager::WimaxSetting::toMap() const
{
    QVariantMap setting;

    if (!networkName().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIMAX_NETWORK_NAME), networkName());
    }

    if (!macAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIMAX_MAC_ADDRESS), macAddress());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::WimaxSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_WIMAX_NETWORK_NAME << ": " << setting.networkName() << '\n';
    dbg.nospace() << NM_SETTING_WIMAX_MAC_ADDRESS << ": " << setting.macAddress() << '\n';

    return dbg.maybeSpace();
}
