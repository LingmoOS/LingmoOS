/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tcsetting.h"
#include "tcsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_TC_CONFIG_SETTING_NAME "tc"

#define NM_SETTING_TC_CONFIG_QDISCS "qdiscs"
#define NM_SETTING_TC_CONFIG_TFILTERS "tfilters"
#endif

NetworkManager::TcSettingPrivate::TcSettingPrivate()
    : name(NM_SETTING_TC_CONFIG_SETTING_NAME)
{
}

NetworkManager::TcSetting::TcSetting()
    : Setting(Setting::Tc)
    , d_ptr(new TcSettingPrivate())
{
}

NetworkManager::TcSetting::TcSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new TcSettingPrivate())
{
    setQdiscs(other->qdiscs());
    setTfilters(other->tfilters());
}

NetworkManager::TcSetting::~TcSetting()
{
    delete d_ptr;
}

QString NetworkManager::TcSetting::name() const
{
    Q_D(const TcSetting);

    return d->name;
}

void NetworkManager::TcSetting::setQdiscs(const NMVariantMapList &qdiscs)
{
    Q_D(TcSetting);

    d->qdiscs = qdiscs;
}

NMVariantMapList NetworkManager::TcSetting::qdiscs() const
{
    Q_D(const TcSetting);

    return d->qdiscs;
}

void NetworkManager::TcSetting::setTfilters(const NMVariantMapList &tfilters)
{
    Q_D(TcSetting);

    d->tfilters = tfilters;
}

NMVariantMapList NetworkManager::TcSetting::tfilters() const
{
    Q_D(const TcSetting);

    return d->tfilters;
}

void NetworkManager::TcSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_TC_CONFIG_QDISCS))) {
        setQdiscs(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NM_SETTING_TC_CONFIG_QDISCS))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_TC_CONFIG_TFILTERS))) {
        setTfilters(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NM_SETTING_TC_CONFIG_TFILTERS))));
    }
}

QVariantMap NetworkManager::TcSetting::toMap() const
{
    QVariantMap setting;

    if (!qdiscs().empty()) {
        setting.insert(QLatin1String(NM_SETTING_TC_CONFIG_QDISCS), QVariant::fromValue(qdiscs()));
    }

    if (!tfilters().empty()) {
        setting.insert(QLatin1String(NM_SETTING_TC_CONFIG_TFILTERS), QVariant::fromValue(tfilters()));
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::TcSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_TC_CONFIG_QDISCS << ": " << '\n';
    const NMVariantMapList qdiscList = setting.qdiscs();
    for (const QVariantMap &qdisc : qdiscList) {
        QVariantMap::const_iterator i = qdisc.constBegin();
        while (i != qdisc.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }
    dbg.nospace() << NM_SETTING_TC_CONFIG_TFILTERS << ": " << '\n';
    const NMVariantMapList tfiltersList = setting.tfilters();
    for (const QVariantMap &tfilter : tfiltersList) {
        QVariantMap::const_iterator i = tfilter.constBegin();
        while (i != tfilter.constEnd()) {
            dbg.nospace() << i.key() << ": " << i.value() << '\n';
        }
    }

    return dbg.maybeSpace();
}
