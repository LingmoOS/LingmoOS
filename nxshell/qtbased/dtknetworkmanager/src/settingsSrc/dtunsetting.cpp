// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtunsetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DTunSettingPrivate::DTunSettingPrivate()
    : m_pi(false)
    , m_vnetHdr(false)
    , m_multiQueue(false)
    , m_mode(DTunSetting::Mode::Tun)
    , m_name(NM_SETTING_TUN_SETTING_NAME)
{
}

DTunSetting::DTunSetting()
    : DNMSetting(DNMSetting::SettingType::Tun)
    , d_ptr(new DTunSettingPrivate())
{
}

DTunSetting::DTunSetting(const QSharedPointer<DTunSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DTunSettingPrivate())
{
    setGroup(other->group());
    setMode(other->mode());
    setMultiQueue(other->multiQueue());
    setOwner(other->owner());
    setPi(other->pi());
    setVnetHdr(other->vnetHdr());
}

QString DTunSetting::name() const
{
    Q_D(const DTunSetting);

    return d->m_name;
}

void DTunSetting::setGroup(const QString &group)
{
    Q_D(DTunSetting);

    d->m_group = group;
}

QString DTunSetting::group() const
{
    Q_D(const DTunSetting);

    return d->m_group;
}

void DTunSetting::setMode(DTunSetting::Mode mode)
{
    Q_D(DTunSetting);

    d->m_mode = mode;
}

DTunSetting::Mode DTunSetting::mode() const
{
    Q_D(const DTunSetting);

    return d->m_mode;
}

void DTunSetting::setMultiQueue(bool multiQueue)
{
    Q_D(DTunSetting);

    d->m_multiQueue = multiQueue;
}

bool DTunSetting::multiQueue() const
{
    Q_D(const DTunSetting);

    return d->m_multiQueue;
}

void DTunSetting::setOwner(const QString &owner)
{
    Q_D(DTunSetting);

    d->m_owner = owner;
}

QString DTunSetting::owner() const
{
    Q_D(const DTunSetting);

    return d->m_owner;
}

void DTunSetting::setPi(bool pi)
{
    Q_D(DTunSetting);

    d->m_pi = pi;
}

bool DTunSetting::pi() const
{
    Q_D(const DTunSetting);

    return d->m_pi;
}

void DTunSetting::setVnetHdr(bool vnetHdr)
{
    Q_D(DTunSetting);

    d->m_vnetHdr = vnetHdr;
}

bool DTunSetting::vnetHdr() const
{
    Q_D(const DTunSetting);

    return d->m_vnetHdr;
}

void DTunSetting::fromMap(const QVariantMap &setting)
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

QVariantMap DTunSetting::toMap() const
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

DNETWORKMANAGER_END_NAMESPACE
