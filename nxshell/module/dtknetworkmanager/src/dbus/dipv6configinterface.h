// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV6CONFIGINTERFACE_H
#define DIPV6CONFIGINTERFACE_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>
#include <QDBusPendingReply>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DIPv6ConfigInterface : public QObject
{
    Q_OBJECT
public:
    explicit DIPv6ConfigInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DIPv6ConfigInterface() override = default;

    Q_PROPERTY(QList<Config> addressData READ addressData NOTIFY AddressDataChanged)
    Q_PROPERTY(QList<QByteArray> nameservers READ nameservers NOTIFY NameserversChanged)
    Q_PROPERTY(QString gateway READ gateway NOTIFY GatewayChanged)

    QList<Config> addressData() const;
    QList<QByteArray> nameservers() const;
    QString gateway() const;

Q_SIGNALS:
    void AddressDataChanged(const QList<Config> &addressData);
    void NameserversChanged(const QList<QByteArray> &nameservers);
    void GatewayChanged(const QString &gateway);

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
