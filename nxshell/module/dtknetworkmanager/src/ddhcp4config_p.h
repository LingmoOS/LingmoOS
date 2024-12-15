// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDHCP4CONFIG_P_H
#define DDHCP4CONFIG_P_H

#include "dbus/ddhcp4configinterface.h"
#include "ddhcp4config.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DDHCP4ConfigPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DDHCP4ConfigPrivate(const quint64 id, DDHCP4Config *parent = nullptr);
    ~DDHCP4ConfigPrivate() override = default;

    DDHCP4Config *q_ptr{nullptr};
    DDHCP4ConfigInterface *m_dhcp4{nullptr};
    Q_DECLARE_PUBLIC(DDHCP4Config)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
