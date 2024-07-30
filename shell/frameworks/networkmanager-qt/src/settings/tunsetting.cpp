/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tunsetting.h"
#include "tunsetting_p.h"

#include <QDebug>

NetworkManager::TunSettingPrivate::TunSettingPrivate()
    : name(NM_SETTING_TUN_SETTING_NAME)
    , mode(TunSetting::Tun)
    , multiQueue(false)
    , pi(false)
    , vnetHdr(false)
{
}

NetworkManager::TunSetting::TunSetting()
    : Setting(Setting::Tun)
    , d_ptr(new TunSettingPrivate())
{
}

NetworkManager::TunSetting::TunSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new TunSettingPrivate())
{
    setGroup(other->group());
    setMode(other->mode());
    setMultiQueue(other->multiQueue());
    setOwner(other->owner());
    setPi(other->pi());
    setVnetHdr(other->vnetHdr());
}

NetworkManager::TunSetting::~TunSetting()
{
    delete d_ptr;
}

QString NetworkManager::TunSetting::name() const
{
    Q_D(const TunSetting);

    return d->name;
}

void NetworkManager::TunSetting::setGroup(const QString &group)
{
    Q_D(TunSetting);

    d->group = group;
}

QString NetworkManager::TunSetting::group() const
{
    Q_D(const TunSetting);

    return d->group;
}

void NetworkManager::TunSetting::setMode(NetworkManager::TunSetting::Mode mode)
{
    Q_D(TunSetting);

    d->mode = mode;
}

NetworkManager::TunSetting::Mode NetworkManager::TunSetting::mode() const
{
    Q_D(const TunSetting);

    return d->mode;
}

void NetworkManager::TunSetting::setMultiQueue(bool multiQueue)
{
    Q_D(TunSetting);

    d->multiQueue = multiQueue;
}

bool NetworkManager::TunSetting::multiQueue() const
{
    Q_D(const TunSetting);

    return d->multiQueue;
}

void NetworkManager::TunSetting::setOwner(const QString &owner)
{
    Q_D(TunSetting);

    d->owner = owner;
}

QString NetworkManager::TunSetting::owner() const
{
    Q_D(const TunSetting);

    return d->owner;
}

void NetworkManager::TunSetting::setPi(bool pi)
{
    Q_D(TunSetting);

    d->pi = pi;
}

bool NetworkManager::TunSetting::pi() const
{
    Q_D(const TunSetting);

    return d->pi;
}

void NetworkManager::TunSetting::setVnetHdr(bool vnetHdr)
{
    Q_D(TunSetting);

    d->vnetHdr = vnetHdr;
}

bool NetworkManager::TunSetting::vnetHdr() const
{
    Q_D(const TunSetting);

    return d->vnetHdr;
}

void NetworkManager::TunSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_TUN_GROUP))) {
        setGroup(setting.value(QLatin1String(NM_SETTING_TUN_GROUP)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TUN_MODE))) {
        setMode((Mode)setting.value(QLatin1String(NM_SETTING_TUN_MODE)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TUN_MULTI_QUEUE))) {
        setMultiQueue(setting.value(QLatin1String(NM_SETTING_TUN_MULTI_QUEUE)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TUN_OWNER))) {
        setOwner(setting.value(QLatin1String(NM_SETTING_TUN_OWNER)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TUN_PI))) {
        setPi(setting.value(QLatin1String(NM_SETTING_TUN_PI)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TUN_VNET_HDR))) {
        setVnetHdr(setting.value(QLatin1String(NM_SETTING_TUN_VNET_HDR)).toBool());
    }
}

QVariantMap NetworkManager::TunSetting::toMap() const
{
    QVariantMap setting;

    if (!group().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_TUN_GROUP), group());
    }

    setting.insert(QLatin1String(NM_SETTING_TUN_MODE), (int)mode());
    setting.insert(QLatin1String(NM_SETTING_TUN_MULTI_QUEUE), multiQueue());

    if (!owner().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_TUN_OWNER), owner());
    }

    setting.insert(QLatin1String(NM_SETTING_TUN_PI), pi());
    setting.insert(QLatin1String(NM_SETTING_TUN_VNET_HDR), vnetHdr());

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::TunSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_TUN_GROUP << ": " << setting.group() << '\n';
    dbg.nospace() << NM_SETTING_TUN_MODE << ": " << setting.mode() << '\n';
    dbg.nospace() << NM_SETTING_TUN_MULTI_QUEUE << ": " << setting.multiQueue() << '\n';
    dbg.nospace() << NM_SETTING_TUN_OWNER << ": " << setting.owner() << '\n';
    dbg.nospace() << NM_SETTING_TUN_PI << ": " << setting.pi() << '\n';
    dbg.nospace() << NM_SETTING_TUN_VNET_HDR << ": " << setting.vnetHdr() << '\n';

    return dbg.maybeSpace();
}
