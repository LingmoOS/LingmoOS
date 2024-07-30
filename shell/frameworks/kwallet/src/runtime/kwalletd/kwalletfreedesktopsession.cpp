/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletfreedesktopsession.h"

#include "kwalletfreedesktopsessionadaptor.h"
#include <QDBusConnection>

KWalletFreedesktopSession::KWalletFreedesktopSession(KWalletFreedesktopService *service,
                                                     std::unique_ptr<KWalletFreedesktopSessionAlgorithm> algorithm,
                                                     QString sessionPath,
                                                     const QDBusConnection &connection,
                                                     const QDBusMessage &message)
    : m_service(service)
    , m_algorithm(std::move(algorithm))
    , m_sessionPath(std::move(sessionPath))
    , m_serviceBusName(message.service())
{
    (void)new KWalletFreedesktopSessionAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_sessionPath, this);

    m_serviceWatcher.setConnection(connection);
    m_serviceWatcher.addWatchedService(m_serviceBusName);
    m_serviceWatcher.setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    connect(&m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KWalletFreedesktopSession::slotServiceOwnerChanged);
}

void KWalletFreedesktopSession::slotServiceOwnerChanged(const QString &, const QString &, const QString &)
{
    fdoService()->deleteSession(m_sessionPath);
}

void KWalletFreedesktopSession::Close()
{
    if (message().service() != m_serviceBusName) {
        sendErrorReply(QDBusError::ErrorType::UnknownObject, QStringLiteral("Can't find session ") + m_sessionPath);
    } else {
        fdoService()->deleteSession(m_sessionPath);
    }
}

QByteArray KWalletFreedesktopSession::negotiationOutput() const
{
    return m_algorithm->negotiationOutput();
}

bool KWalletFreedesktopSession::encrypt(const QDBusMessage &message, FreedesktopSecret &secret) const
{
    if (message.service() != m_serviceBusName) {
        return false;
    }

    return m_algorithm->encrypt(secret);
}

bool KWalletFreedesktopSession::decrypt(const QDBusMessage &message, FreedesktopSecret &secret) const
{
    if (message.service() != m_serviceBusName) {
        return false;
    }

    return m_algorithm->decrypt(secret);
}

KWalletFreedesktopService *KWalletFreedesktopSession::fdoService() const
{
    return m_service;
}

KWalletD *KWalletFreedesktopSession::backend() const
{
    return fdoService()->backend();
}

QDBusObjectPath KWalletFreedesktopSession::fdoObjectPath() const
{
    return QDBusObjectPath(m_sessionPath);
}

QByteArray KWalletFreedesktopSessionAlgorithmPlain::negotiationOutput() const
{
    return QByteArray();
}

bool KWalletFreedesktopSessionAlgorithmPlain::encrypt(FreedesktopSecret &secret) const
{
    secret.parameters = QByteArray();
    return true;
}

bool KWalletFreedesktopSessionAlgorithmPlain::decrypt(FreedesktopSecret &) const
{
    return true;
}

KWalletFreedesktopSessionAlgorithmDhAes::KWalletFreedesktopSessionAlgorithmDhAes(const QCA::PublicKey &publicKey, QCA::SymmetricKey symmetricKey)
    : m_publicKey(publicKey)
    , m_symmetricKey(std::move(symmetricKey))
{
}

QByteArray KWalletFreedesktopSessionAlgorithmDhAes::negotiationOutput() const
{
    return m_publicKey.toDH().y().toArray().toByteArray();
}

bool KWalletFreedesktopSessionAlgorithmDhAes::encrypt(FreedesktopSecret &secret) const
{
    auto initVector = QCA::InitializationVector(FDO_SECRETS_CIPHER_KEY_SIZE);
    auto cipher = QCA::Cipher(QStringLiteral("aes128"), QCA::Cipher::CBC, QCA::Cipher::PKCS7, QCA::Encode, m_symmetricKey, initVector);
    QCA::SecureArray result;
    result.append(cipher.update(QCA::MemoryRegion(secret.value)));
    if (cipher.ok()) {
        result.append(cipher.final());
        if (cipher.ok()) {
            secret.value = std::move(result);
            secret.parameters = initVector;
            return true;
        }
    }
    return false;
}

bool KWalletFreedesktopSessionAlgorithmDhAes::decrypt(FreedesktopSecret &secret) const
{
    auto cipher =
        QCA::Cipher(QStringLiteral("aes128"), QCA::Cipher::CBC, QCA::Cipher::PKCS7, QCA::Decode, m_symmetricKey, QCA::InitializationVector(secret.parameters));
    QCA::SecureArray result;
    result.append(cipher.update(QCA::MemoryRegion(secret.value)));
    if (cipher.ok()) {
        result.append(cipher.final());
        if (cipher.ok()) {
            secret.value = std::move(result);
            return true;
        }
    }
    return false;
}

#include "moc_kwalletfreedesktopsession.cpp"
