/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bondsetting.h"
#include "bondsetting_p.h"

#define NM_SETTING_BOND_INTERFACE_NAME "interface-name"

#include <QDebug>

NetworkManager::BondSettingPrivate::BondSettingPrivate()
    : name(NM_SETTING_BOND_SETTING_NAME)
{
}

NetworkManager::BondSetting::BondSetting()
    : Setting(Setting::Bond)
    , d_ptr(new BondSettingPrivate())
{
}

NetworkManager::BondSetting::BondSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new BondSettingPrivate())
{
    setInterfaceName(other->interfaceName());
    setOptions(other->options());
}

NetworkManager::BondSetting::~BondSetting()
{
    delete d_ptr;
}

QString NetworkManager::BondSetting::name() const
{
    Q_D(const BondSetting);

    return d->name;
}

void NetworkManager::BondSetting::setInterfaceName(const QString &name)
{
    Q_D(BondSetting);

    d->interfaceName = name;
}

QString NetworkManager::BondSetting::interfaceName() const
{
    Q_D(const BondSetting);

    return d->interfaceName;
}

void NetworkManager::BondSetting::addOption(const QString &option, const QString &value)
{
    Q_D(BondSetting);

    d->options.insert(option, value);
}

void NetworkManager::BondSetting::setOptions(const NMStringMap &options)
{
    Q_D(BondSetting);

    d->options = options;
}

NMStringMap NetworkManager::BondSetting::options() const
{
    Q_D(const BondSetting);

    return d->options;
}

void NetworkManager::BondSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_BOND_INTERFACE_NAME))) {
        setInterfaceName(setting.value(QLatin1String(NM_SETTING_BOND_INTERFACE_NAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BOND_OPTIONS))) {
        setOptions(qdbus_cast<NMStringMap>(setting.value(QLatin1String(NM_SETTING_BOND_OPTIONS))));
    }
}

QVariantMap NetworkManager::BondSetting::toMap() const
{
    QVariantMap setting;
    if (!interfaceName().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_BOND_INTERFACE_NAME), interfaceName());
    }
    if (!options().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_BOND_OPTIONS), QVariant::fromValue<NMStringMap>(options()));
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::BondSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_BOND_INTERFACE_NAME << ": " << setting.interfaceName() << '\n';
    dbg.nospace() << NM_SETTING_BOND_OPTIONS << ": " << setting.options() << '\n';

    return dbg.maybeSpace();
}
