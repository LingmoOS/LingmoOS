// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYSTEMPROXYINTERFACE_H
#define DSYSTEMPROXYINTERFACE_H

#include "dnetworkmanager_global.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>
#include <QDBusPendingReply>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DSystemProxyInterface : public QObject
{
    Q_OBJECT
public:
    explicit DSystemProxyInterface(QObject *parent = nullptr);
    ~DSystemProxyInterface() override = default;

public Q_SLOTS:
    QDBusPendingReply<QString> getAutoProxy() const;
    QDBusPendingReply<QString, QString> getProxy(const QString &proxyType) const;
    QDBusPendingReply<QString> getProxyIgnoreHosts() const;
    QDBusPendingReply<QString> getProxyMethod() const;
    QDBusPendingReply<void> setAutoProxy(const QString &proxyAuto) const;
    QDBusPendingReply<void> setProxy(const QString &proxyType, const QString &host, const QString &port) const;
    QDBusPendingReply<void> setProxyIgnoreHosts(const QString &ignoreHosts) const;
    QDBusPendingReply<void> setProxyMethod(const QString &method) const;

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
