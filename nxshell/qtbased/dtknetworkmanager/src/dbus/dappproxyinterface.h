// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAPPPROXYINTERFACE_H
#define DAPPPROXYINTERFACE_H

#include "dnetworkmanager_global.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>
#include <QDBusPendingReply>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DAppProxyInterface : public QObject
{
    Q_OBJECT
public:
    explicit DAppProxyInterface(QObject *parent = nullptr);
    ~DAppProxyInterface() override = default;

    Q_PROPERTY(QString IP READ IP)
    Q_PROPERTY(QString password READ password)
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString user READ user)
    Q_PROPERTY(quint32 port READ port)

    QString IP() const;
    QString password() const;
    QString type() const;
    QString user() const;
    quint32 port() const;

public Q_SLOTS:
    QDBusPendingReply<void>
    set(const QString &type, const QString &ip, const quint32 port, const QString &user, const QString &password) const;

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
