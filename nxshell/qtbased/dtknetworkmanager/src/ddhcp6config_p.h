// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDHCP6CONFIG_P_H
#define DDHCP6CONFIG_P_H

#include "dbus/ddhcp6configinterface.h"
#include "ddhcp6config.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DDHCP6ConfigPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DDHCP6ConfigPrivate(const quint64 id, DDHCP6Config *parent = nullptr);
    ~DDHCP6ConfigPrivate() override = default;

    DDHCP6Config *q_ptr{nullptr};
    DDHCP6ConfigInterface *m_dhcp6{nullptr};
    Q_DECLARE_PUBLIC(DDHCP6Config)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
