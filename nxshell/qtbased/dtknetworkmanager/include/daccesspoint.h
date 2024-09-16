// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCESSPOINT_H
#define DACCESSPOINT_H

#include "dnetworkmanagertypes.h"
#include <QFlags>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAccessPointPrivate;

class DAccessPoint : public QObject
{
    Q_OBJECT
public:
    Q_DECLARE_FLAGS(NMAP, NM80211ApFlags);
    Q_DECLARE_FLAGS(NMAPSec, NM80211ApSecurityFlags);

    explicit DAccessPoint(const quint64 apId, QObject *parent = nullptr);
    ~DAccessPoint() override;

    Q_PROPERTY(QString SSID READ SSID NOTIFY SSIDChanged)
    Q_PROPERTY(quint8 strength READ strength NOTIFY strengthChanged)
    Q_PROPERTY(NMAP flags READ flags NOTIFY flagsChanged)
    Q_PROPERTY(quint32 frequency READ frequency NOTIFY frequencyChanged)
    Q_PROPERTY(NMAPSec rsnFlags READ rsnFlags NOTIFY rsnFlagsChanged)
    Q_PROPERTY(NMAPSec wpaFlags READ wpaFlags NOTIFY wpaFlagsChanged)

    QString SSID() const;
    quint8 strength() const;
    NMAP flags() const;
    quint32 frequency() const;
    NMAPSec rsnFlags() const;
    NMAPSec wpaFlags() const;

Q_SIGNALS:

    void SSIDChanged(const QString &ssid);
    void strengthChanged(const quint8 strength);
    void frequencyChanged(const quint32 frequency);
    void flagsChanged(const NMAP &flags);
    void rsnFlagsChanged(const NMAPSec &rsnFlags);
    void wpaFlagsChanged(const NMAPSec &wpaFlags);

private:
    QScopedPointer<DAccessPointPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DAccessPoint)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DAccessPoint::NMAP)
Q_DECLARE_OPERATORS_FOR_FLAGS(DAccessPoint::NMAPSec)

DNETWORKMANAGER_END_NAMESPACE

#endif
