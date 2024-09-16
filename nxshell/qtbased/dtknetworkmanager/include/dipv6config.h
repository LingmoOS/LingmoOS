// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV6CONFIG_H
#define DIPV6CONFIG_H

#include "dnetworkmanagertypes.h"
#include <QHostAddress>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIPv6ConfigPrivate;

class DIPv6Config : public QObject
{
    Q_OBJECT
public:
    explicit DIPv6Config(const quint64 id, QObject *parent = nullptr);
    ~DIPv6Config() override;

    Q_PROPERTY(QList<Config> addressData READ addressData NOTIFY addressDataChanged)
    Q_PROPERTY(QList<QHostAddress> nameservers READ nameservers NOTIFY nameserversChanged)
    Q_PROPERTY(QHostAddress gateway READ gateway NOTIFY gatewayChanged)

    QList<Config> addressData() const;
    QList<QHostAddress> nameservers() const;
    QHostAddress gateway() const;

Q_SIGNALS:
    void addressDataChanged(const QList<Config> &addrs) const;
    void nameserversChanged(const QList<QHostAddress> &nameserver) const;
    void gatewayChanged(const QHostAddress &gateway);

private:
    QScopedPointer<DIPv6ConfigPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DIPv6Config)
};

DNETWORKMANAGER_END_NAMESPACE
#endif
