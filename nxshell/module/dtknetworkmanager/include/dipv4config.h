// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV4CONFIG_H
#define DIPV4CONFIG_H

#include "dnetworkmanagertypes.h"
#include <QHostAddress>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIPv4ConfigPrivate;

class DIPv4Config : public QObject
{
    Q_OBJECT
public:
    explicit DIPv4Config(const quint64 id, QObject *parent = nullptr);
    ~DIPv4Config() override;

    Q_PROPERTY(QList<Config> addressData READ addressData NOTIFY addressDataChanged)
    Q_PROPERTY(QList<Config> nameserverData READ nameserverData NOTIFY nameserverDataChanged)
    Q_PROPERTY(QHostAddress gateway READ gateway NOTIFY gatewayChanged)

    QList<Config> addressData() const;
    QList<Config> nameserverData() const;
    QHostAddress gateway() const;

Q_SIGNALS:
    void addressDataChanged(const QList<Config> &addrs) const;
    void nameserverDataChanged(const QList<Config> &nameserverData) const;
    void gatewayChanged(const QHostAddress &gateway);

private:
    QScopedPointer<DIPv4ConfigPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DIPv4Config)
};

DNETWORKMANAGER_END_NAMESPACE
#endif
