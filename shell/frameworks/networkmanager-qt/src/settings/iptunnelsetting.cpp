/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iptunnelsetting.h"
#include "iptunnelsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 12, 0)
#define NM_SETTING_IP_TUNNEL_FLAGS "flags"
#endif

NetworkManager::IpTunnelSettingPrivate::IpTunnelSettingPrivate()
    : name(NM_SETTING_IP_TUNNEL_SETTING_NAME)
    , mode(IpTunnelSetting::Unknown)
    , pathMtuDiscovery(true)
    , encapsulationLimit(0)
    , flags(IpTunnelSetting::None)
    , flowLabel(0)
    , mtu(0)
    , tos(0)
    , ttl(0)
{
}

NetworkManager::IpTunnelSetting::IpTunnelSetting()
    : Setting(Setting::IpTunnel)
    , d_ptr(new IpTunnelSettingPrivate())
{
}

NetworkManager::IpTunnelSetting::IpTunnelSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new IpTunnelSettingPrivate())
{
    setMode(other->mode());
    setPathMtuDiscovery(other->pathMtuDiscovery());
    setEncapsulationLimit(other->encapsulationLimit());
    setFlags(other->flags());
    setFlowLabel(other->flowLabel());
    setMtu(other->mtu());
    setTos(other->tos());
    setTtl(other->ttl());
    setInputKey(other->inputKey());
    setLocal(other->local());
    setOutputKey(other->outputKey());
    setParent(other->parent());
    setRemote(other->remote());
}

NetworkManager::IpTunnelSetting::~IpTunnelSetting()
{
    delete d_ptr;
}

QString NetworkManager::IpTunnelSetting::name() const
{
    Q_D(const IpTunnelSetting);

    return d->name;
}

void NetworkManager::IpTunnelSetting::setMode(NetworkManager::IpTunnelSetting::Mode mode)
{
    Q_D(IpTunnelSetting);

    d->mode = mode;
}

NetworkManager::IpTunnelSetting::Mode NetworkManager::IpTunnelSetting::mode() const
{
    Q_D(const IpTunnelSetting);

    return d->mode;
}

void NetworkManager::IpTunnelSetting::setPathMtuDiscovery(bool discovery)
{
    Q_D(IpTunnelSetting);

    d->pathMtuDiscovery = discovery;
}

bool NetworkManager::IpTunnelSetting::pathMtuDiscovery() const
{
    Q_D(const IpTunnelSetting);

    return d->pathMtuDiscovery;
}

void NetworkManager::IpTunnelSetting::setEncapsulationLimit(quint32 limit)
{
    Q_D(IpTunnelSetting);

    d->encapsulationLimit = limit;
}

quint32 NetworkManager::IpTunnelSetting::encapsulationLimit() const
{
    Q_D(const IpTunnelSetting);

    return d->encapsulationLimit;
}

void NetworkManager::IpTunnelSetting::setFlags(NetworkManager::IpTunnelSetting::Flags flags)
{
    Q_D(IpTunnelSetting);

    d->flags = flags;
}

NetworkManager::IpTunnelSetting::Flags NetworkManager::IpTunnelSetting::flags() const
{
    Q_D(const IpTunnelSetting);

    return d->flags;
}

void NetworkManager::IpTunnelSetting::setFlowLabel(quint32 label)
{
    Q_D(IpTunnelSetting);

    d->flowLabel = label;
}

quint32 NetworkManager::IpTunnelSetting::flowLabel() const
{
    Q_D(const IpTunnelSetting);

    return d->flowLabel;
}

void NetworkManager::IpTunnelSetting::setMtu(quint32 mtu)
{
    Q_D(IpTunnelSetting);

    d->mtu = mtu;
}

quint32 NetworkManager::IpTunnelSetting::mtu() const
{
    Q_D(const IpTunnelSetting);

    return d->mtu;
}

void NetworkManager::IpTunnelSetting::setTos(quint32 tos)
{
    Q_D(IpTunnelSetting);

    d->tos = tos;
}

quint32 NetworkManager::IpTunnelSetting::tos() const
{
    Q_D(const IpTunnelSetting);

    return d->tos;
}

void NetworkManager::IpTunnelSetting::setTtl(quint32 ttl)
{
    Q_D(IpTunnelSetting);

    d->ttl = ttl;
}

quint32 NetworkManager::IpTunnelSetting::ttl() const
{
    Q_D(const IpTunnelSetting);

    return d->ttl;
}

void NetworkManager::IpTunnelSetting::setInputKey(const QString &key)
{
    Q_D(IpTunnelSetting);

    d->inputKey = key;
}

QString NetworkManager::IpTunnelSetting::inputKey() const
{
    Q_D(const IpTunnelSetting);

    return d->inputKey;
}

void NetworkManager::IpTunnelSetting::setLocal(const QString &local)
{
    Q_D(IpTunnelSetting);

    d->local = local;
}

QString NetworkManager::IpTunnelSetting::local() const
{
    Q_D(const IpTunnelSetting);

    return d->local;
}

void NetworkManager::IpTunnelSetting::setParent(const QString &parent)
{
    Q_D(IpTunnelSetting);

    d->parent = parent;
}

QString NetworkManager::IpTunnelSetting::parent() const
{
    Q_D(const IpTunnelSetting);

    return d->parent;
}

void NetworkManager::IpTunnelSetting::setOutputKey(const QString &key)
{
    Q_D(IpTunnelSetting);

    d->outputKey = key;
}

QString NetworkManager::IpTunnelSetting::outputKey() const
{
    Q_D(const IpTunnelSetting);

    return d->outputKey;
}

void NetworkManager::IpTunnelSetting::setRemote(const QString &remote)
{
    Q_D(IpTunnelSetting);

    d->remote = remote;
}

QString NetworkManager::IpTunnelSetting::remote() const
{
    Q_D(const IpTunnelSetting);

    return d->remote;
}

void NetworkManager::IpTunnelSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_MODE))) {
        setMode((Mode)setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_MODE)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_PATH_MTU_DISCOVERY))) {
        setPathMtuDiscovery(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_PATH_MTU_DISCOVERY)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_ENCAPSULATION_LIMIT))) {
        setEncapsulationLimit(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_ENCAPSULATION_LIMIT)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_FLAGS))) {
        setFlags((Flag)setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_FLOW_LABEL))) {
        setFlowLabel(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_FLOW_LABEL)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_MTU))) {
        setMtu(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_MTU)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_TOS))) {
        setTos(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_TOS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_TTL))) {
        setTtl(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_TTL)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_INPUT_KEY))) {
        setInputKey(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_INPUT_KEY)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_LOCAL))) {
        setLocal(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_LOCAL)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_PARENT))) {
        setParent(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_PARENT)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_OUTPUT_KEY))) {
        setOutputKey(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_OUTPUT_KEY)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_IP_TUNNEL_REMOTE))) {
        setRemote(setting.value(QLatin1String(NM_SETTING_IP_TUNNEL_REMOTE)).toString());
    }
}

QVariantMap NetworkManager::IpTunnelSetting::toMap() const
{
    QVariantMap setting;

    setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_PATH_MTU_DISCOVERY), pathMtuDiscovery());

    if (mode() != Unknown) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_MODE), (int)mode());
    }

    if (encapsulationLimit() > 0) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_ENCAPSULATION_LIMIT), encapsulationLimit());
    }

    if (flags() > None) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_FLAGS), (int)flags());
    }

    if (flowLabel() > 0) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_FLOW_LABEL), flowLabel());
    }

    if (mtu() > 0) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_MTU), mtu());
    }

    if (tos() > 0) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_TOS), tos());
    }

    if (ttl() > 0) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_TTL), ttl());
    }

    if (!inputKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_INPUT_KEY), inputKey());
    }

    if (!local().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_LOCAL), local());
    }

    if (!parent().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_PARENT), parent());
    }

    if (!outputKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_OUTPUT_KEY), outputKey());
    }

    if (!remote().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_IP_TUNNEL_REMOTE), remote());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::IpTunnelSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_IP_TUNNEL_MODE << ": " << setting.mode() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_PATH_MTU_DISCOVERY << ": " << setting.pathMtuDiscovery() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_ENCAPSULATION_LIMIT << ": " << setting.encapsulationLimit() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_FLAGS << ": " << setting.flags() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_FLOW_LABEL << ": " << setting.flowLabel() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_MTU << ": " << setting.mtu() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_TOS << ": " << setting.tos() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_TTL << ": " << setting.ttl() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_INPUT_KEY << ": " << setting.inputKey() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_LOCAL << ": " << setting.local() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_PARENT << ": " << setting.parent() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_OUTPUT_KEY << ": " << setting.outputKey() << '\n';
    dbg.nospace() << NM_SETTING_IP_TUNNEL_REMOTE << ": " << setting.remote() << '\n';

    return dbg.maybeSpace();
}
