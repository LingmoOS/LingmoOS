/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "matchsetting.h"
#include "matchsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 14, 0)
#define NM_SETTING_MATCH_SETTING_NAME "match"
#define NM_SETTING_MATCH_INTERFACE_NAME "interface-name"
#endif

NetworkManager::MatchSettingPrivate::MatchSettingPrivate()
    : name(NM_SETTING_MATCH_SETTING_NAME)
{
}

NetworkManager::MatchSetting::MatchSetting()
    : Setting(Setting::Match)
    , d_ptr(new MatchSettingPrivate())
{
}

NetworkManager::MatchSetting::MatchSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new MatchSettingPrivate())
{
    setInterfaceName(other->interfaceName());
}

NetworkManager::MatchSetting::~MatchSetting()
{
    delete d_ptr;
}

QString NetworkManager::MatchSetting::name() const
{
    Q_D(const MatchSetting);

    return d->name;
}

void NetworkManager::MatchSetting::setInterfaceName(const QStringList &name)
{
    Q_D(MatchSetting);

    d->interfaceName = name;
}

QStringList NetworkManager::MatchSetting::interfaceName() const
{
    Q_D(const MatchSetting);

    return d->interfaceName;
}

void NetworkManager::MatchSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_MATCH_INTERFACE_NAME))) {
        setInterfaceName(setting.value(QLatin1String(NM_SETTING_MATCH_INTERFACE_NAME)).toStringList());
    }
}

QVariantMap NetworkManager::MatchSetting::toMap() const
{
    QVariantMap setting;

    if (!interfaceName().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_MATCH_INTERFACE_NAME), interfaceName());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::MatchSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_MATCH_INTERFACE_NAME << ": " << setting.interfaceName() << '\n';

    return dbg.maybeSpace();
}
