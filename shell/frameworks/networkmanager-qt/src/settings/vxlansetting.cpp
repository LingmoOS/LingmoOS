/*
    SPDX-FileCopyrightText: 2018 Billy Laws <blaws05@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vxlansetting.h"
#include "vxlansetting_p.h"

#include <QDebug>

NetworkManager::VxlanSettingPrivate::VxlanSettingPrivate()
    : name(NM_SETTING_VXLAN_SETTING_NAME)
    , ageing(32)
    , destinationPort(8472)
    , id(0)
    , l2Miss(false)
    , l3Miss(false)
    , learning(true)
    , limit(0)
    , proxy(false)
    , rsc(false)
    , sourcePortMax(0)
    , sourcePortMin(0)
    , tos(0)
    , ttl(0)
{
}

NetworkManager::VxlanSetting::VxlanSetting()
    : Setting(Setting::Vxlan)
    , d_ptr(new VxlanSettingPrivate())
{
}

NetworkManager::VxlanSetting::VxlanSetting(const NetworkManager::VxlanSetting::Ptr &other)
    : Setting(other)
    , d_ptr(new VxlanSettingPrivate())
{
    setAgeing(other->ageing());
    setDestinationPort(other->destinationPort());
    setId(other->id());
    setL2Miss(other->l2Miss());
    setL3Miss(other->l3Miss());
    setLearning(other->learning());
    setLimit(other->limit());
    setLocal(other->local());
    setParent(other->parent());
    setProxy(other->proxy());
    setRemote(other->remote());
    setRsc(other->rsc());
    setSourcePortMax(other->sourcePortMax());
    setSourcePortMin(other->sourcePortMin());
    setTos(other->tos());
    setTtl(other->ttl());
}

NetworkManager::VxlanSetting::~VxlanSetting()
{
    delete d_ptr;
}

QString NetworkManager::VxlanSetting::name() const
{
    Q_D(const VxlanSetting);

    return d->name;
}

void NetworkManager::VxlanSetting::setAgeing(quint32 ageing)
{
    Q_D(VxlanSetting);

    d->ageing = ageing;
}

quint32 NetworkManager::VxlanSetting::ageing() const
{
    Q_D(const VxlanSetting);

    return d->ageing;
}

void NetworkManager::VxlanSetting::setDestinationPort(quint32 port)
{
    Q_D(VxlanSetting);

    d->destinationPort = port;
}

quint32 NetworkManager::VxlanSetting::destinationPort() const
{
    Q_D(const VxlanSetting);

    return d->destinationPort;
}

void NetworkManager::VxlanSetting::setId(quint32 id)
{
    Q_D(VxlanSetting);

    d->id = id;
}

quint32 NetworkManager::VxlanSetting::id() const
{
    Q_D(const VxlanSetting);

    return d->id;
}

void NetworkManager::VxlanSetting::setL2Miss(bool enable)
{
    Q_D(VxlanSetting);

    d->l2Miss = enable;
}

bool NetworkManager::VxlanSetting::l2Miss() const
{
    Q_D(const VxlanSetting);

    return d->l2Miss;
}

void NetworkManager::VxlanSetting::setL3Miss(bool enable)
{
    Q_D(VxlanSetting);

    d->l3Miss = enable;
}

bool NetworkManager::VxlanSetting::l3Miss() const
{
    Q_D(const VxlanSetting);

    return d->l3Miss;
}

void NetworkManager::VxlanSetting::setLearning(bool enable)
{
    Q_D(VxlanSetting);

    d->learning = enable;
}

bool NetworkManager::VxlanSetting::learning() const
{
    Q_D(const VxlanSetting);

    return d->learning;
}

void NetworkManager::VxlanSetting::setLimit(quint32 limit)
{
    Q_D(VxlanSetting);

    d->limit = limit;
}

quint32 NetworkManager::VxlanSetting::limit() const
{
    Q_D(const VxlanSetting);

    return d->limit;
}

void NetworkManager::VxlanSetting::setLocal(QString local)
{
    Q_D(VxlanSetting);

    d->local = local;
}

QString NetworkManager::VxlanSetting::local() const
{
    Q_D(const VxlanSetting);

    return d->local;
}

void NetworkManager::VxlanSetting::setParent(QString parent)
{
    Q_D(VxlanSetting);

    d->parent = parent;
}

QString NetworkManager::VxlanSetting::parent() const
{
    Q_D(const VxlanSetting);

    return d->parent;
}

void NetworkManager::VxlanSetting::setProxy(bool enable)
{
    Q_D(VxlanSetting);

    d->proxy = enable;
}

bool NetworkManager::VxlanSetting::proxy() const
{
    Q_D(const VxlanSetting);

    return d->proxy;
}

void NetworkManager::VxlanSetting::setRemote(QString remote)
{
    Q_D(VxlanSetting);

    d->remote = remote;
}

QString NetworkManager::VxlanSetting::remote() const
{
    Q_D(const VxlanSetting);

    return d->remote;
}

void NetworkManager::VxlanSetting::setRsc(bool enable)
{
    Q_D(VxlanSetting);

    d->rsc = enable;
}

bool NetworkManager::VxlanSetting::rsc() const
{
    Q_D(const VxlanSetting);

    return d->rsc;
}

void NetworkManager::VxlanSetting::setSourcePortMax(quint32 maxPort)
{
    Q_D(VxlanSetting);

    d->sourcePortMax = maxPort;
}

quint32 NetworkManager::VxlanSetting::sourcePortMax() const
{
    Q_D(const VxlanSetting);

    return d->sourcePortMax;
}

void NetworkManager::VxlanSetting::setSourcePortMin(quint32 minPort)
{
    Q_D(VxlanSetting);

    d->sourcePortMin = minPort;
}

quint32 NetworkManager::VxlanSetting::sourcePortMin() const
{
    Q_D(const VxlanSetting);

    return d->sourcePortMin;
}

void NetworkManager::VxlanSetting::setTos(quint32 tos)
{
    Q_D(VxlanSetting);

    d->tos = tos;
}

quint32 NetworkManager::VxlanSetting::tos() const
{
    Q_D(const VxlanSetting);

    return d->tos;
}

void NetworkManager::VxlanSetting::setTtl(quint32 ttl)
{
    Q_D(VxlanSetting);

    d->ttl = ttl;
}

quint32 NetworkManager::VxlanSetting::ttl() const
{
    Q_D(const VxlanSetting);

    return d->ttl;
}
void NetworkManager::VxlanSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_AGEING))) {
        setAgeing(setting.value(QLatin1String(NM_SETTING_VXLAN_AGEING)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_DESTINATION_PORT))) {
        setDestinationPort(setting.value(QLatin1String(NM_SETTING_VXLAN_DESTINATION_PORT)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_ID))) {
        setId(setting.value(QLatin1String(NM_SETTING_VXLAN_ID)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_L2_MISS))) {
        setL2Miss(setting.value(QLatin1String(NM_SETTING_VXLAN_L2_MISS)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_L3_MISS))) {
        setL3Miss(setting.value(QLatin1String(NM_SETTING_VXLAN_L3_MISS)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_LEARNING))) {
        setLearning(setting.value(QLatin1String(NM_SETTING_VXLAN_LEARNING)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_LIMIT))) {
        setLimit(setting.value(QLatin1String(NM_SETTING_VXLAN_LIMIT)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_LOCAL))) {
        setLocal(setting.value(QLatin1String(NM_SETTING_VXLAN_LOCAL)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_PARENT))) {
        setParent(setting.value(QLatin1String(NM_SETTING_VXLAN_PARENT)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_PROXY))) {
        setProxy(setting.value(QLatin1String(NM_SETTING_VXLAN_PROXY)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_REMOTE))) {
        setRemote(setting.value(QLatin1String(NM_SETTING_VXLAN_REMOTE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_RSC))) {
        setRsc(setting.value(QLatin1String(NM_SETTING_VXLAN_RSC)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MAX))) {
        setSourcePortMax(setting.value(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MAX)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MIN))) {
        setSourcePortMin(setting.value(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MIN)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_TOS))) {
        setTos(setting.value(QLatin1String(NM_SETTING_VXLAN_TOS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_VXLAN_TTL))) {
        setTtl(setting.value(QLatin1String(NM_SETTING_VXLAN_TTL)).toUInt());
    }
}

QVariantMap NetworkManager::VxlanSetting::toMap() const
{
    QVariantMap setting;

    if (ageing() != 32) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_AGEING), ageing());
    }

    if (destinationPort() != 8472) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_DESTINATION_PORT), destinationPort());
    }

    if (id()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_ID), id());
    }

    if (l2Miss()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_L2_MISS), l2Miss());
    }

    if (l3Miss()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_L3_MISS), l3Miss());
    }

    if (!learning()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_LEARNING), learning());
    }

    if (limit()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_LIMIT), limit());
    }

    if (!local().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_LOCAL), local());
    }

    if (!parent().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_PARENT), parent());
    }

    if (proxy()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_PROXY), proxy());
    }

    if (!remote().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_REMOTE), remote());
    }

    if (rsc()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_RSC), rsc());
    }

    if (sourcePortMax()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MAX), sourcePortMax());
    }

    if (sourcePortMin()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_SOURCE_PORT_MIN), sourcePortMin());
    }

    if (tos()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_TOS), tos());
    }

    if (ttl()) {
        setting.insert(QLatin1String(NM_SETTING_VXLAN_TTL), ttl());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::VxlanSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_VXLAN_AGEING << ": " << setting.ageing() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_DESTINATION_PORT << ": " << setting.destinationPort() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_ID << ": " << setting.id() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_L2_MISS << ": " << setting.l2Miss() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_L3_MISS << ": " << setting.l3Miss() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_LEARNING << ": " << setting.learning() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_LIMIT << ": " << setting.limit() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_LOCAL << ": " << setting.local() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_PARENT << ": " << setting.parent() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_PROXY << ": " << setting.proxy() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_REMOTE << ": " << setting.remote() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_RSC << ": " << setting.rsc() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_SOURCE_PORT_MAX << ": " << setting.sourcePortMax() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_SOURCE_PORT_MIN << ": " << setting.sourcePortMin() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_TOS << ": " << setting.tos() << '\n';
    dbg.nospace() << NM_SETTING_VXLAN_TTL << ": " << setting.ttl() << '\n';

    return dbg.maybeSpace();
}
