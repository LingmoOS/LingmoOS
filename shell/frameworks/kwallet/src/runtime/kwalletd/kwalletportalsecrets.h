/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWALLETPORTALSECRETS_H
#define KWALLETPORTALSECRETS_H

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QObject>

#include "kwalletd.h"

class KWalletPortalSecrets : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    KWalletPortalSecrets(KWalletD *parent);

public Q_SLOTS:
    uint
    RetrieveSecret(const QDBusObjectPath &handle, const QString &app_id, const QDBusUnixFileDescriptor &fd, const QVariantMap &options, QVariantMap &results);

private Q_SLOTS:
    void walletOpened(int id, int handle);

private:
    struct Request {
        QDBusMessage message;
        int fd;
        QString appId;
    };

    QByteArray generateSecret() const;

    QHash<int, Request> m_pendingRequests;

    KWalletD *m_kwalletd;
};

#endif
