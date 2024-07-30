/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETFREEDESKTOPSESSION_H_
#define _KWALLETFREEDESKTOPSESSION_H_

#include "kwalletfreedesktopservice.h"
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusServiceWatcher>
#include <QtCrypto>

#define FDO_SECRETS_SESSION_PATH FDO_SECRETS_SERVICE_OBJECT "/session/"

class KWalletD;

class KWalletFreedesktopSessionAlgorithm
{
public:
    virtual ~KWalletFreedesktopSessionAlgorithm() = default;
    virtual QByteArray negotiationOutput() const = 0;
    virtual bool encrypt(FreedesktopSecret &secret) const = 0;
    virtual bool decrypt(FreedesktopSecret &secret) const = 0;
};

class KWalletFreedesktopSession : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    KWalletFreedesktopSession(class KWalletFreedesktopService *parent,
                              std::unique_ptr<KWalletFreedesktopSessionAlgorithm> algorithm,
                              QString sessionPath,
                              const QDBusConnection &connection,
                              const QDBusMessage &message);

    KWalletFreedesktopSession(const KWalletFreedesktopSession &) = delete;
    KWalletFreedesktopSession &operator=(const KWalletFreedesktopSession &) = delete;

    KWalletFreedesktopSession(KWalletFreedesktopSession &&) = delete;
    KWalletFreedesktopSession &operator=(KWalletFreedesktopSession &&) = delete;

    KWalletFreedesktopService *fdoService() const;
    KWalletD *backend() const;
    QDBusObjectPath fdoObjectPath() const;

    QByteArray negotiationOutput() const;
    bool encrypt(const QDBusMessage &message, FreedesktopSecret &secret) const;
    bool decrypt(const QDBusMessage &message, FreedesktopSecret &secret) const;

private Q_SLOTS:
    void slotServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    class KWalletFreedesktopService *m_service;
    std::unique_ptr<KWalletFreedesktopSessionAlgorithm> m_algorithm;
    QString m_sessionPath;
    QString m_serviceBusName;
    QDBusServiceWatcher m_serviceWatcher;

    /* Freedesktop API */

    /* org.freedesktop.Secret.Session methods */
public Q_SLOTS:
    void Close();
};

class KWalletFreedesktopSessionAlgorithmPlain : public KWalletFreedesktopSessionAlgorithm
{
public:
    QByteArray negotiationOutput() const override;
    bool encrypt(FreedesktopSecret &secret) const override;
    bool decrypt(FreedesktopSecret &secret) const override;
};

class KWalletFreedesktopSessionAlgorithmDhAes : public KWalletFreedesktopSessionAlgorithm
{
public:
    KWalletFreedesktopSessionAlgorithmDhAes(const QCA::PublicKey &publicKey, QCA::SymmetricKey symmetricKey);

    QByteArray negotiationOutput() const override;
    bool encrypt(FreedesktopSecret &secret) const override;
    bool decrypt(FreedesktopSecret &secret) const override;

private:
    QCA::PublicKey m_publicKey;
    QCA::SymmetricKey m_symmetricKey;
};

#endif
