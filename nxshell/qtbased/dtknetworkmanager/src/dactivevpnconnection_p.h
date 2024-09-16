// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACTIVEVPNCONNECTION_P_H
#define DACTIVEVPNCONNECTION_P_H

#include "dactiveconnection_p.h"
#include "dactivevpnconnection.h"
#include "dbus/dactivevpnconnectioninterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DActiveVpnConnectionPrivate : public DActiveConnectionPrivate
{
    Q_OBJECT
public:
    explicit DActiveVpnConnectionPrivate(const quint64 id, DActiveVpnConnection *parent);
    ~DActiveVpnConnectionPrivate() override = default;

    DActiveVpnConnectionInterface *m_vpn{nullptr};
    Q_DECLARE_PUBLIC(DActiveVpnConnection)
};

DNETWORKMANAGER_END_NAMESPACE
#endif
