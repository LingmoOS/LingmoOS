// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccesspoint.h"
#include "daccesspoint_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DAccessPointPrivate::DAccessPointPrivate(const quint64 apId, DAccessPoint *parent)
    : q_ptr(parent)
#ifdef USE_FAKE_INTERFACE
    , m_ap(new DAccessPointInterface("/com/deepin/FakeNetworkManager/AccessPoint/" + QByteArray::number(apId), this))
#else
    , m_ap(new DAccessPointInterface("/org/freedesktop/NetworkManager/AccessPoint/" + QByteArray::number(apId), this))
#endif
{
}

DAccessPoint::DAccessPoint(const quint64 apId, QObject *parent)
    : QObject(parent)
    , d_ptr(new DAccessPointPrivate(apId, this))
{
    Q_D(const DAccessPoint);

    connect(d->m_ap, &DAccessPointInterface::SsidChanged, this, &DAccessPoint::SSID);

    connect(d->m_ap, &DAccessPointInterface::StrengthChanged, this, &DAccessPoint::strength);

    connect(
        d->m_ap, &DAccessPointInterface::FlagsChanged, this, [this](const quint32 flags) { Q_EMIT this->flagsChanged({flags}); });

    connect(d->m_ap, &DAccessPointInterface::FrequencyChanged, this, &DAccessPoint::frequencyChanged);

    connect(d->m_ap, &DAccessPointInterface::RsnFlagsChanged, this, [this](const quint32 rsnFlags) {
        Q_EMIT this->rsnFlagsChanged({rsnFlags});
    });

    connect(d->m_ap, &DAccessPointInterface::WpaFlagsChanged, this, [this](const quint32 wpaFlags) {
        Q_EMIT this->wpaFlagsChanged({wpaFlags});
    });
}

DAccessPoint::~DAccessPoint() = default;

QString DAccessPoint::SSID() const
{
    Q_D(const DAccessPoint);
    return d->m_ap->SSID();
}
quint8 DAccessPoint::strength() const
{
    Q_D(const DAccessPoint);
    return d->m_ap->strength();
}
DAccessPoint::NMAP DAccessPoint::flags() const
{
    Q_D(const DAccessPoint);
    return {d->m_ap->flags()};
}
quint32 DAccessPoint::frequency() const
{
    Q_D(const DAccessPoint);
    return d->m_ap->frequency();
}
DAccessPoint::NMAPSec DAccessPoint::rsnFlags() const
{
    Q_D(const DAccessPoint);
    return {d->m_ap->rsnFlags()};
}
DAccessPoint::NMAPSec DAccessPoint::wpaFlags() const
{
    Q_D(const DAccessPoint);
    return {d->m_ap->wpaFlags()};
}

DNETWORKMANAGER_END_NAMESPACE
