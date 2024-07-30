/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ovspatchsetting.h"
#include "ovspatchsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_PATCH_SETTING_NAME "ovs-patch"
#define NM_SETTING_OVS_PATCH_PEER "peer"
#endif

NetworkManager::OvsPatchSettingPrivate::OvsPatchSettingPrivate()
    : name(NM_SETTING_OVS_PATCH_SETTING_NAME)
{
}

NetworkManager::OvsPatchSetting::OvsPatchSetting()
    : Setting(Setting::OvsPatch)
    , d_ptr(new OvsPatchSettingPrivate())
{
}

NetworkManager::OvsPatchSetting::OvsPatchSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new OvsPatchSettingPrivate())
{
    setPeer(other->peer());
}

NetworkManager::OvsPatchSetting::~OvsPatchSetting()
{
    delete d_ptr;
}

QString NetworkManager::OvsPatchSetting::name() const
{
    Q_D(const OvsPatchSetting);

    return d->name;
}

void NetworkManager::OvsPatchSetting::setPeer(const QString &peer)
{
    Q_D(OvsPatchSetting);

    d->peer = peer;
}

QString NetworkManager::OvsPatchSetting::peer() const
{
    Q_D(const OvsPatchSetting);

    return d->peer;
}

void NetworkManager::OvsPatchSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_OVS_PATCH_PEER))) {
        setPeer(setting.value(QLatin1String(NM_SETTING_OVS_PATCH_PEER)).toString());
    }
}

QVariantMap NetworkManager::OvsPatchSetting::toMap() const
{
    QVariantMap setting;

    if (!peer().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OVS_PATCH_PEER), peer());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::OvsPatchSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_OVS_PATCH_PEER << ": " << setting.peer() << '\n';

    return dbg.maybeSpace();
}
