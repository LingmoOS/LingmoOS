// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYSTEMPROXY_H
#define DSYSTEMPROXY_H

#include "dnetworkmanagertypes.h"
#include <QUrl>
#include <DExpected>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DExpected;

class DSystemProxyPrivate;

class DSystemProxy : public QObject
{
    Q_OBJECT
public:
    explicit DSystemProxy(QObject *parent = nullptr);
    ~DSystemProxy() override;

public Q_SLOTS:
    DExpected<QString> autoProxy() const;
    DExpected<SystemProxyAddr> proxy(const ProxyType method) const;
    DExpected<QString> proxyIgnoreHosts() const;
    DExpected<SystemProxyMethod> proxyMethod() const;
    DExpected<void> setAutoProxy(const QUrl &pac) const;
    DExpected<void> setProxy(const ProxyType type, const SystemProxyAddr &addr) const;
    DExpected<void> setProxyIgnoreHosts(const QString &hosts) const;
    DExpected<void> setProxyMethod(const SystemProxyMethod method) const;

private:
    QScopedPointer<DSystemProxyPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DSystemProxy)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
