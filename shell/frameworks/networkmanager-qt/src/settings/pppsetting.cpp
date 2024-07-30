/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pppsetting.h"
#include "pppsetting_p.h"

#include <QDebug>

NetworkManager::PppSettingPrivate::PppSettingPrivate()
    : name(NM_SETTING_PPP_SETTING_NAME)
    , noauth(true)
    , refuseEap(false)
    , refusePap(false)
    , refuseChap(false)
    , refuseMschap(false)
    , refuseMschapv2(false)
    , nobsdcomp(false)
    , nodeflate(false)
    , noVjComp(false)
    , requireMppe(false)
    , requireMppe128(false)
    , mppeStateful(false)
    , crtscts(false)
    , baud(0)
    , mru(0)
    , mtu(0)
    , lcpEchoFailure(0)
    , lcpEchoInterval(0)
{
}

NetworkManager::PppSetting::PppSetting()
    : Setting(Setting::Ppp)
    , d_ptr(new PppSettingPrivate())
{
}

NetworkManager::PppSetting::PppSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new PppSettingPrivate())
{
    setNoAuth(other->noAuth());
    setRefuseEap(other->refuseEap());
    setRefusePap(other->refusePap());
    setRefuseChap(other->refuseChap());
    setRefuseMschap(other->refuseMschap());
    setRefuseMschapv2(other->refuseMschapv2());
    setNoBsdComp(other->noBsdComp());
    setNoDeflate(other->noDeflate());
    setNoVjComp(other->noVjComp());
    setRequireMppe(other->requireMppe());
    setRequireMppe128(other->requireMppe128());
    setMppeStateful(other->mppeStateful());
    setCRtsCts(other->cRtsCts());
    setBaud(other->baud());
    setMru(other->mru());
    setMtu(other->mtu());
    setLcpEchoFailure(other->lcpEchoFailure());
    setLcpEchoInterval(other->lcpEchoInterval());
}

NetworkManager::PppSetting::~PppSetting()
{
    delete d_ptr;
}

QString NetworkManager::PppSetting::name() const
{
    Q_D(const PppSetting);

    return d->name;
}

void NetworkManager::PppSetting::setNoAuth(bool require)
{
    Q_D(PppSetting);

    d->noauth = require;
}

bool NetworkManager::PppSetting::noAuth() const
{
    Q_D(const PppSetting);

    return d->noauth;
}

void NetworkManager::PppSetting::setRefuseEap(bool refuse)
{
    Q_D(PppSetting);

    d->refuseEap = refuse;
}

bool NetworkManager::PppSetting::refuseEap() const
{
    Q_D(const PppSetting);

    return d->refuseEap;
}

void NetworkManager::PppSetting::setRefusePap(bool refuse)
{
    Q_D(PppSetting);

    d->refusePap = refuse;
}

bool NetworkManager::PppSetting::refusePap() const
{
    Q_D(const PppSetting);

    return d->refusePap;
}

void NetworkManager::PppSetting::setRefuseChap(bool refuse)
{
    Q_D(PppSetting);

    d->refuseChap = refuse;
}

bool NetworkManager::PppSetting::refuseChap() const
{
    Q_D(const PppSetting);

    return d->refuseChap;
}

void NetworkManager::PppSetting::setRefuseMschap(bool refuse)
{
    Q_D(PppSetting);

    d->refuseMschap = refuse;
}

bool NetworkManager::PppSetting::refuseMschap() const
{
    Q_D(const PppSetting);

    return d->refuseMschap;
}

void NetworkManager::PppSetting::setRefuseMschapv2(bool refuse)
{
    Q_D(PppSetting);

    d->refuseMschapv2 = refuse;
}

bool NetworkManager::PppSetting::refuseMschapv2() const
{
    Q_D(const PppSetting);

    return d->refuseMschapv2;
}

void NetworkManager::PppSetting::setNoBsdComp(bool require)
{
    Q_D(PppSetting);

    d->nobsdcomp = require;
}

bool NetworkManager::PppSetting::noBsdComp() const
{
    Q_D(const PppSetting);

    return d->nobsdcomp;
}

void NetworkManager::PppSetting::setNoDeflate(bool require)
{
    Q_D(PppSetting);

    d->nodeflate = require;
}

bool NetworkManager::PppSetting::noDeflate() const
{
    Q_D(const PppSetting);

    return d->nodeflate;
}

void NetworkManager::PppSetting::setNoVjComp(bool require)
{
    Q_D(PppSetting);

    d->noVjComp = require;
}

bool NetworkManager::PppSetting::noVjComp() const
{
    Q_D(const PppSetting);

    return d->noVjComp;
}

void NetworkManager::PppSetting::setRequireMppe(bool require)
{
    Q_D(PppSetting);

    d->requireMppe = require;
}

bool NetworkManager::PppSetting::requireMppe() const
{
    Q_D(const PppSetting);

    return d->requireMppe;
}

void NetworkManager::PppSetting::setRequireMppe128(bool require)
{
    Q_D(PppSetting);

    d->requireMppe128 = require;
}

bool NetworkManager::PppSetting::requireMppe128() const
{
    Q_D(const PppSetting);

    return d->requireMppe128;
}

void NetworkManager::PppSetting::setMppeStateful(bool used)
{
    Q_D(PppSetting);

    d->mppeStateful = used;
}

bool NetworkManager::PppSetting::mppeStateful() const
{
    Q_D(const PppSetting);

    return d->mppeStateful;
}

void NetworkManager::PppSetting::setCRtsCts(bool control)
{
    Q_D(PppSetting);

    d->crtscts = control;
}

bool NetworkManager::PppSetting::cRtsCts() const
{
    Q_D(const PppSetting);

    return d->crtscts;
}

void NetworkManager::PppSetting::setBaud(quint32 baud)
{
    Q_D(PppSetting);

    d->baud = baud;
}

quint32 NetworkManager::PppSetting::baud() const
{
    Q_D(const PppSetting);

    return d->baud;
}

void NetworkManager::PppSetting::setMru(quint32 mru)
{
    Q_D(PppSetting);

    d->mru = mru;
}

quint32 NetworkManager::PppSetting::mru() const
{
    Q_D(const PppSetting);

    return d->mru;
}

void NetworkManager::PppSetting::setMtu(quint32 mtu)
{
    Q_D(PppSetting);

    d->mtu = mtu;
}

quint32 NetworkManager::PppSetting::mtu() const
{
    Q_D(const PppSetting);

    return d->mtu;
}

void NetworkManager::PppSetting::setLcpEchoFailure(quint32 number)
{
    Q_D(PppSetting);

    d->lcpEchoFailure = number;
}

quint32 NetworkManager::PppSetting::lcpEchoFailure() const
{
    Q_D(const PppSetting);

    return d->lcpEchoFailure;
}

void NetworkManager::PppSetting::setLcpEchoInterval(quint32 interval)
{
    Q_D(PppSetting);

    d->lcpEchoInterval = interval;
}

quint32 NetworkManager::PppSetting::lcpEchoInterval() const
{
    Q_D(const PppSetting);

    return d->lcpEchoInterval;
}

void NetworkManager::PppSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_PPP_NOAUTH))) {
        setNoAuth(setting.value(QLatin1String(NM_SETTING_PPP_NOAUTH)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REFUSE_EAP))) {
        setRefuseEap(setting.value(QLatin1String(NM_SETTING_PPP_REFUSE_EAP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REFUSE_PAP))) {
        setRefusePap(setting.value(QLatin1String(NM_SETTING_PPP_REFUSE_PAP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REFUSE_CHAP))) {
        setRefuseChap(setting.value(QLatin1String(NM_SETTING_PPP_REFUSE_CHAP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAP))) {
        setRefuseMschap(setting.value(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAPV2))) {
        setRefuseMschapv2(setting.value(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAPV2)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_NOBSDCOMP))) {
        setNoBsdComp(setting.value(QLatin1String(NM_SETTING_PPP_NOBSDCOMP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_NODEFLATE))) {
        setNoDeflate(setting.value(QLatin1String(NM_SETTING_PPP_NODEFLATE)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_NO_VJ_COMP))) {
        setNoVjComp(setting.value(QLatin1String(NM_SETTING_PPP_NO_VJ_COMP)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE))) {
        setRequireMppe(setting.value(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE_128))) {
        setRequireMppe128(setting.value(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE_128)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_MPPE_STATEFUL))) {
        setMppeStateful(setting.value(QLatin1String(NM_SETTING_PPP_MPPE_STATEFUL)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_CRTSCTS))) {
        setCRtsCts(setting.value(QLatin1String(NM_SETTING_PPP_CRTSCTS)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_BAUD))) {
        setBaud(setting.value(QLatin1String(NM_SETTING_PPP_BAUD)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_MRU))) {
        setMru(setting.value(QLatin1String(NM_SETTING_PPP_MRU)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_MTU))) {
        setMtu(setting.value(QLatin1String(NM_SETTING_PPP_MTU)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_LCP_ECHO_FAILURE))) {
        setLcpEchoFailure(setting.value(QLatin1String(NM_SETTING_PPP_LCP_ECHO_FAILURE)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_PPP_LCP_ECHO_INTERVAL))) {
        setLcpEchoInterval(setting.value(QLatin1String(NM_SETTING_PPP_LCP_ECHO_INTERVAL)).toUInt());
    }
}

QVariantMap NetworkManager::PppSetting::toMap() const
{
    QVariantMap setting;

    if (!noAuth()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_NOAUTH), noAuth());
    }

    if (refuseEap()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REFUSE_EAP), refuseEap());
    }

    if (refusePap()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REFUSE_PAP), refusePap());
    }

    if (refuseChap()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REFUSE_CHAP), refuseChap());
    }

    if (refuseMschap()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAP), refuseMschap());
    }

    if (refuseMschapv2()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAPV2), refuseMschapv2());
    }

    if (noBsdComp()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_NOBSDCOMP), noBsdComp());
    }

    if (noDeflate()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_NODEFLATE), noDeflate());
    }

    if (noVjComp()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_NO_VJ_COMP), noVjComp());
    }

    if (requireMppe()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE), requireMppe());
    }

    if (requireMppe128()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE_128), requireMppe128());
    }

    if (mppeStateful()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_MPPE_STATEFUL), mppeStateful());
    }

    if (cRtsCts()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_CRTSCTS), cRtsCts());
    }

    if (baud()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_BAUD), baud());
    }

    if (mru()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_MRU), mru());
    }

    if (mtu()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_MTU), mtu());
    }

    if (lcpEchoFailure()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_LCP_ECHO_FAILURE), lcpEchoFailure());
    }

    if (lcpEchoInterval()) {
        setting.insert(QLatin1String(NM_SETTING_PPP_LCP_ECHO_INTERVAL), lcpEchoInterval());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::PppSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_PPP_NOAUTH << ": " << setting.noAuth() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REFUSE_EAP << ": " << setting.refuseEap() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REFUSE_PAP << ": " << setting.refusePap() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REFUSE_CHAP << ": " << setting.refuseChap() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REFUSE_MSCHAP << ": " << setting.refuseMschap() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REFUSE_MSCHAPV2 << ": " << setting.refuseMschapv2() << '\n';
    dbg.nospace() << NM_SETTING_PPP_NOBSDCOMP << ": " << setting.noBsdComp() << '\n';
    dbg.nospace() << NM_SETTING_PPP_NODEFLATE << ": " << setting.noDeflate() << '\n';
    dbg.nospace() << NM_SETTING_PPP_NO_VJ_COMP << ": " << setting.noVjComp() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REQUIRE_MPPE << ": " << setting.requireMppe() << '\n';
    dbg.nospace() << NM_SETTING_PPP_REQUIRE_MPPE_128 << ": " << setting.requireMppe128() << '\n';
    dbg.nospace() << NM_SETTING_PPP_MPPE_STATEFUL << ": " << setting.mppeStateful() << '\n';
    dbg.nospace() << NM_SETTING_PPP_CRTSCTS << ": " << setting.cRtsCts() << '\n';
    dbg.nospace() << NM_SETTING_PPP_BAUD << ": " << setting.baud() << '\n';
    dbg.nospace() << NM_SETTING_PPP_MRU << ": " << setting.mru() << '\n';
    dbg.nospace() << NM_SETTING_PPP_MTU << ": " << setting.mtu() << '\n';
    dbg.nospace() << NM_SETTING_PPP_LCP_ECHO_FAILURE << ": " << setting.lcpEchoFailure() << '\n';
    dbg.nospace() << NM_SETTING_PPP_LCP_ECHO_INTERVAL << ": " << setting.lcpEchoInterval() << '\n';

    return dbg.maybeSpace();
}
