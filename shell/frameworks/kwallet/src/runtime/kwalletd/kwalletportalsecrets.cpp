/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletportalsecrets.h"

#include "kwalletportalsecretsadaptor.h"

#include <QRandomGenerator>

KWalletPortalSecrets::KWalletPortalSecrets(KWalletD *parent)
    : QObject(parent)
    , m_kwalletd(parent)
{
    (void)new KWalletPortalSecretsAdaptor(this);

    QDBusConnection::sessionBus().registerService(QStringLiteral("org.freedesktop.impl.portal.desktop.kwallet"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/portal/desktop"), this, QDBusConnection::ExportAdaptors);

    connect(m_kwalletd, &KWalletD::walletAsyncOpened, this, &KWalletPortalSecrets::walletOpened);
}

uint KWalletPortalSecrets::RetrieveSecret(const QDBusObjectPath &handle,
                                          const QString &app_id,
                                          const QDBusUnixFileDescriptor &fd,
                                          const QVariantMap &options,
                                          QVariantMap &results)
{
    Q_UNUSED(handle);
    Q_UNUSED(options);
    Q_UNUSED(results);

    setDelayedReply(true);

    const QString wallet = m_kwalletd->networkWallet();

    int transactionId = m_kwalletd->openAsync(wallet, 0, "xdg-desktop-portal", false, connection(), message());
    Request request{message(), fd.fileDescriptor(), app_id};
    m_pendingRequests.insert(transactionId, request);

    return 0;
}

void KWalletPortalSecrets::walletOpened(int transactionId, int walletHandle)
{
    if (!m_pendingRequests.contains(transactionId)) {
        // wallet open request was not from us
        return;
    }

    const Request request = m_pendingRequests.take(transactionId);

    bool exists = m_kwalletd->hasEntry(walletHandle, "xdg-desktop-portal", request.appId, "xdg-desktop-portal");

    QByteArray secret;

    if (exists) {
        secret = m_kwalletd->readEntry(walletHandle, "xdg-desktop-portal", request.appId, "xdg-desktop-portal");
    } else {
        secret = generateSecret();
        m_kwalletd->writeEntry(walletHandle, "xdg-desktop-portal", request.appId, secret, "xdg-desktop-portal");
        m_kwalletd->sync(walletHandle, "xdg-desktop-portal");
    }

    QFile outFile;
    outFile.open(request.fd, QIODevice::ReadWrite);

    outFile.write(secret);

    const auto replyList = QVariantList{{(uint)0}, {{QVariantMap{}}}};
    auto reply = request.message.createReply(replyList);
    QDBusConnection::sessionBus().send(reply);
}

QByteArray KWalletPortalSecrets::generateSecret() const
{
    const int secretSize = 64;
    QByteArray secret;
    secret.resize(secretSize);
    QRandomGenerator::securelySeeded().generate(secret.begin(), secret.end());

    return secret;
}
