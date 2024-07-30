/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_SECRET_AGENT_H
#define PLASMA_NM_SECRET_AGENT_H

#include <NetworkManagerQt/SecretAgent>

namespace KWallet
{
class Wallet;
}

class PasswordDialog;

class SecretsRequest
{
public:
    enum Type {
        GetSecrets,
        SaveSecrets,
        DeleteSecrets,
    };
    explicit SecretsRequest(Type _type)
        : type(_type)
    {
    }
    inline bool operator==(const QString &other) const
    {
        return callId == other;
    }
    Type type;
    QString callId;
    NMVariantMapMap connection;
    QDBusObjectPath connection_path;
    QString setting_name;
    QStringList hints;
    NetworkManager::SecretAgent::GetSecretsFlags flags = NetworkManager::SecretAgent::None;
    /**
     * When a user connection is called on GetSecrets,
     * the secret agent is supposed to save the secrets
     * typed by user, when true proccessSaveSecrets
     * should skip the DBus reply.
     */
    bool saveSecretsWithoutReply = false;
    QDBusMessage message;
    PasswordDialog *dialog = nullptr;
};

class Q_DECL_EXPORT SecretAgent : public NetworkManager::SecretAgent
{
    Q_OBJECT
public:
    explicit SecretAgent(QObject *parent = nullptr);
    explicit SecretAgent(NetworkManager::SecretAgent::Capabilities capabilities, QObject *parent = nullptr);

    ~SecretAgent() override;

Q_SIGNALS:
    void secretsError(const QString &connectionPath, const QString &message) const;

public Q_SLOTS:
    NMVariantMapMap GetSecrets(const NMVariantMapMap &, const QDBusObjectPath &, const QString &, const QStringList &, uint) override;
    void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path) override;
    void DeleteSecrets(const NMVariantMapMap &, const QDBusObjectPath &) override;
    void CancelGetSecrets(const QDBusObjectPath &, const QString &) override;

private Q_SLOTS:
    void dialogAccepted();
    void dialogRejected();
    void killDialogs();
    void walletOpened(bool success);
    void walletClosed();

private:
    void processNext();
    /**
     * @brief processGetSecrets requests
     * @param request the request we are processing
     * @param ignoreWallet true if the code should avoid Wallet
     * normally if it failed to open
     * @return true if the item was processed
     */
    bool processGetSecrets(SecretsRequest &request) const;
    bool processSaveSecrets(SecretsRequest &request) const;
    bool processDeleteSecrets(SecretsRequest &request) const;
    /**
     * @brief useWallet checks if the KWallet system is enabled
     * and tries to open it async.
     * @return return true if the method should use the wallet,
     * the caller MUST always check if the wallet is opened.
     */
    bool useWallet() const;

    /**
     * @brief hasSecrets verifies if the desired connection has secrets to store
     * @param connection map with or without secrets
     * @return true if the connection has secrets, false otherwise
     */
    bool hasSecrets(const NMVariantMapMap &connection) const;
    void sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const;

    mutable bool m_openWalletFailed;
    mutable KWallet::Wallet *m_wallet;
    mutable PasswordDialog *m_dialog;
    QList<SecretsRequest> m_calls;

    void importSecretsFromPlainTextFiles();
};

#endif // PLASMA_NM_SECRET_AGENT_H
