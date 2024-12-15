// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAPPPROXY_P_H
#define DAPPPROXY_P_H

#include "dappproxy.h"
#include "dbus/dappproxyinterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAppProxyPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DAppProxyPrivate(DAppProxy *parent = nullptr);
    ~DAppProxyPrivate() override = default;

    DAppProxy *q_ptr{nullptr};
    DAppProxyInterface *m_appProxy{nullptr};
    Q_DECLARE_PUBLIC(DAppProxy)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
