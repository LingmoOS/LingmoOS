/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "teamportsetting.h"
#include "teamportsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_TEAM_PORT_SETTING_NAME "team-port"

#define NM_SETTING_TEAM_PORT_CONFIG "config"
#define NM_SETTING_TEAM_PORT_QUEUE_ID "queue-id"
#define NM_SETTING_TEAM_PORT_PRIO "prio"
#define NM_SETTING_TEAM_PORT_STICKY "sticky"
#define NM_SETTING_TEAM_PORT_LACP_PRIO "lacp-prio"
#define NM_SETTING_TEAM_PORT_LACP_KEY "lacp-key"
#define NM_SETTING_TEAM_PORT_LINK_WATCHERS "link-watchers"
#endif

NetworkManager::TeamPortSettingPrivate::TeamPortSettingPrivate()
    : name(NM_SETTING_TEAM_PORT_SETTING_NAME)
    , lacpKey(0)
    , lacpPrio(255)
    , prio(0)
    , queueId(-1)
    , sticky(false)
{
}

NetworkManager::TeamPortSetting::TeamPortSetting()
    : Setting(Setting::TeamPort)
    , d_ptr(new TeamPortSettingPrivate())
{
}

NetworkManager::TeamPortSetting::TeamPortSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new TeamPortSettingPrivate())
{
    config(other->config());
    lacpKey(other->lacpKey());
    lacpPrio(other->lacpPrio());
    prio(other->prio());
    queueId(other->queueId());
    sticky(other->sticky());
    setLinkWatchers(other->linkWatchers());
}

NetworkManager::TeamPortSetting::~TeamPortSetting()
{
    delete d_ptr;
}

QString NetworkManager::TeamPortSetting::name() const
{
    Q_D(const TeamPortSetting);

    return d->name;
}

void NetworkManager::TeamPortSetting::config(const QString &config)
{
    Q_D(TeamPortSetting);

    d->config = config;
}

QString NetworkManager::TeamPortSetting::config() const
{
    Q_D(const TeamPortSetting);

    return d->config;
}

void NetworkManager::TeamPortSetting::lacpKey(qint32 key)
{
    Q_D(TeamPortSetting);

    d->lacpKey = key;
}

qint32 NetworkManager::TeamPortSetting::lacpKey() const
{
    Q_D(const TeamPortSetting);

    return d->lacpKey;
}

void NetworkManager::TeamPortSetting::lacpPrio(qint32 priority)
{
    Q_D(TeamPortSetting);

    d->lacpPrio = priority;
}

qint32 NetworkManager::TeamPortSetting::lacpPrio() const
{
    Q_D(const TeamPortSetting);

    return d->lacpPrio;
}

void NetworkManager::TeamPortSetting::prio(qint32 prio)
{
    Q_D(TeamPortSetting);

    d->prio = prio;
}

qint32 NetworkManager::TeamPortSetting::prio() const
{
    Q_D(const TeamPortSetting);

    return d->prio;
}

void NetworkManager::TeamPortSetting::queueId(qint32 id)
{
    Q_D(TeamPortSetting);

    d->queueId = id;
}

qint32 NetworkManager::TeamPortSetting::queueId() const
{
    Q_D(const TeamPortSetting);

    return d->queueId;
}

void NetworkManager::TeamPortSetting::sticky(bool sticky)
{
    Q_D(TeamPortSetting);

    d->sticky = sticky;
}

bool NetworkManager::TeamPortSetting::sticky() const
{
    Q_D(const TeamPortSetting);

    return d->sticky;
}

void NetworkManager::TeamPortSetting::setLinkWatchers(const NMVariantMapList &linkWatchers)
{
    Q_D(TeamPortSetting);

    d->linkWatchers = linkWatchers;
}

NMVariantMapList NetworkManager::TeamPortSetting::linkWatchers() const
{
    Q_D(const TeamPortSetting);

    return d->linkWatchers;
}

void NetworkManager::TeamPortSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_CONFIG))) {
        config(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_CONFIG)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_LACP_KEY))) {
        lacpKey(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_LACP_KEY)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_LACP_PRIO))) {
        lacpPrio(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_LACP_PRIO)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_PRIO))) {
        prio(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_PRIO)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_QUEUE_ID))) {
        queueId(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_QUEUE_ID)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_STICKY))) {
        sticky(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_STICKY)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS))) {
        setLinkWatchers(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS))));
    }
}

QVariantMap NetworkManager::TeamPortSetting::toMap() const
{
    QVariantMap setting;

    if (!config().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_CONFIG), config());
    }

    if (lacpKey() != 0) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_LACP_KEY), lacpKey());
    }

    if (lacpPrio() != 255) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_LACP_PRIO), lacpPrio());
    }

    if (prio() != 0) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_PRIO), prio());
    }

    if (queueId() != -1) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_QUEUE_ID), queueId());
    }

    if (sticky()) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_STICKY), sticky());
    }

    if (!linkWatchers().empty()) {
        setting.insert(QLatin1String(NM_SETTING_TEAM_PORT_LINK_WATCHERS), QVariant::fromValue(linkWatchers()));
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::TeamPortSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_TEAM_PORT_CONFIG << ": " << setting.config() << '\n';
    dbg.nospace() << NM_SETTING_TEAM_PORT_LACP_KEY << ": " << setting.lacpKey() << '\n';
    dbg.nospace() << NM_SETTING_TEAM_PORT_LACP_PRIO << ": " << setting.lacpPrio() << '\n';
    dbg.nospace() << NM_SETTING_TEAM_PORT_PRIO << ": " << setting.prio() << '\n';
    dbg.nospace() << NM_SETTING_TEAM_PORT_QUEUE_ID << ": " << setting.queueId() << '\n';
    dbg.nospace() << NM_SETTING_TEAM_PORT_STICKY << ": " << setting.sticky() << '\n';

    dbg.nospace() << NM_SETTING_TEAM_PORT_LINK_WATCHERS << ": " << '\n';
    const NMVariantMapList variantMapList = setting.linkWatchers();
    for (const QVariantMap &linkWatcher : variantMapList) {
        QVariantMap::const_iterator i = linkWatcher.constBegin();
        while (i != linkWatcher.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }

    return dbg.maybeSpace();
}
