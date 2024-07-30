/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ovsinterfacesetting.h"
#include "ovsinterfacesetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_INTERFACE_SETTING_NAME "ovs-interface"
#define NM_SETTING_OVS_INTERFACE_TYPE "type"
#endif

NetworkManager::OvsInterfaceSettingPrivate::OvsInterfaceSettingPrivate()
    : name(NM_SETTING_OVS_INTERFACE_SETTING_NAME)
{
}

NetworkManager::OvsInterfaceSetting::OvsInterfaceSetting()
    : Setting(Setting::OvsInterface)
    , d_ptr(new OvsInterfaceSettingPrivate())
{
}

NetworkManager::OvsInterfaceSetting::OvsInterfaceSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new OvsInterfaceSettingPrivate())
{
    setInterfaceType(other->interfaceType());
}

NetworkManager::OvsInterfaceSetting::~OvsInterfaceSetting()
{
    delete d_ptr;
}

QString NetworkManager::OvsInterfaceSetting::name() const
{
    Q_D(const OvsInterfaceSetting);

    return d->name;
}

void NetworkManager::OvsInterfaceSetting::setInterfaceType(const QString &type)
{
    Q_D(OvsInterfaceSetting);

    d->type = type;
}

QString NetworkManager::OvsInterfaceSetting::interfaceType() const
{
    Q_D(const OvsInterfaceSetting);

    return d->type;
}

void NetworkManager::OvsInterfaceSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_OVS_INTERFACE_TYPE))) {
        setInterfaceType(setting.value(QLatin1String(NM_SETTING_OVS_INTERFACE_TYPE)).toString());
    }
}

QVariantMap NetworkManager::OvsInterfaceSetting::toMap() const
{
    QVariantMap setting;

    if (!interfaceType().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OVS_INTERFACE_TYPE), interfaceType());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::OvsInterfaceSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_OVS_INTERFACE_TYPE << ": " << setting.type() << '\n';

    return dbg.maybeSpace();
}
