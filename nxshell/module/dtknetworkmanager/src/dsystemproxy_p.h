// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYSTEMPROXY_P_H
#define DSYSTEMPROXY_P_H

#include "dbus/dsystemproxyinterface.h"
#include "dsystemproxy.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DSystemProxyPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DSystemProxyPrivate(DSystemProxy *parent = nullptr);
    ~DSystemProxyPrivate() override = default;

    DSystemProxy *q_ptr{nullptr};
    DSystemProxyInterface *m_sysProxy{nullptr};
    Q_DECLARE_PUBLIC(DSystemProxy)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
