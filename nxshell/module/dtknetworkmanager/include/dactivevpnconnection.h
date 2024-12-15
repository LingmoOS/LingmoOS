// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACTIVEVPNCONNECTION_H
#define DACTIVEVPNCONNECTION_H

#include "dactiveconnection.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DActiveVpnConnectionPrivate;

class DActiveVpnConnection : public DActiveConnection
{
    Q_OBJECT
public:
    explicit DActiveVpnConnection(const quint64 activeConnId, QObject *parent = nullptr);
    ~DActiveVpnConnection() override = default;

    Q_PROPERTY(NMVpnConnectionState vpnState READ vpnState NOTIFY VpnStateChanged)
    Q_PROPERTY(QString banner READ banner NOTIFY bannerChanged)

    NMVpnConnectionState vpnState() const;
    QString banner() const;

Q_SIGNALS:
    void VpnStateChanged(const NMVpnConnectionState state, const NMActiveConnectionStateReason reason);
    void bannerChanged(const QString &banner);

private:
    Q_DECLARE_PRIVATE(DActiveVpnConnection)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
