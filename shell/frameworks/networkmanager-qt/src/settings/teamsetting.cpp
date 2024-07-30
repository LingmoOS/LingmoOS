/*
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "teamsetting.h"
#include "manager.h"
#include "teamsetting_p.h"

#define NM_SETTING_TEAM_INTERFACE_NAME "interface-name"

#include <QDebug>

NetworkManager::TeamSettingPrivate::TeamSettingPrivate()
    : name(NM_SETTING_TEAM_SETTING_NAME)
{
}

NetworkManager::TeamSetting::TeamSetting()
    : Setting(Setting::Team)
    , d_ptr(new TeamSettingPrivate())
{
}

NetworkManager::TeamSetting::TeamSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new TeamSettingPrivate())
{
    setInterfaceName(other->interfaceName());
    setConfig(other->config());
}

NetworkManager::TeamSetting::~TeamSetting()
{
    delete d_ptr;
}

QString NetworkManager::TeamSetting::name() const
{
    Q_D(const TeamSetting);

    return d->name;
}

void NetworkManager::TeamSetting::setInterfaceName(const QString &name)
{
    Q_D(TeamSetting);

    d->interfaceName = name;
}

QString NetworkManager::TeamSetting::interfaceName() const
{
    Q_D(const TeamSetting);

    return d->interfaceName;
}

QString NetworkManager::TeamSetting::config() const
{
    Q_D(const TeamSetting);
    return d->config;
}

void NetworkManager::TeamSetting::setConfig(const QString &config)
{
    Q_D(TeamSetting);
    d->config = config;
}

void NetworkManager::TeamSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_TEAM_INTERFACE_NAME))) {
        setInterfaceName(setting.value(QLatin1String(NM_SETTING_TEAM_INTERFACE_NAME)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_CONFIG))) {
        setConfig(setting.value(QLatin1String(NM_SETTING_TEAM_CONFIG)).toString());
    }
}

QVariantMap NetworkManager::TeamSetting::toMap() const
{
    QVariantMap setting;

    if (!interfaceName().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_INTERFACE_NAME), interfaceName());
    }
    if (!config().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_CONFIG), config());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::TeamSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_TEAM_INTERFACE_NAME << ": " << setting.interfaceName() << '\n';
    dbg.nospace() << NM_SETTING_TEAM_CONFIG << ": " << setting.config() << '\n';

    return dbg.maybeSpace();
}
