// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETSECRETAGENT_H
#define NETSECRETAGENT_H

#include "netsecretagentinterface.h"

#include <NetworkManagerQt/SecretAgent>

#include <QDateTime>

namespace dde {
namespace network {

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
        , flags(NetworkManager::SecretAgent::None)
        , saveSecretsWithoutReply(false)
        , createTime(QDateTime::currentDateTime().toMSecsSinceEpoch())
    {
    }

    inline bool operator==(const QString &other) const { return callId == other; }

    Type type;
    QString callId;
    NMVariantMapMap connection;
    QDBusObjectPath connection_path;
    QString setting_name;
    QStringList hints;
    NetworkManager::SecretAgent::GetSecretsFlags flags;
    /**
     * When a user connection is called on GetSecrets,
     * the secret agent is supposed to save the secrets
     * typed by user, when true proccessSaveSecrets
     * should skip the DBus reply.
     */
    bool saveSecretsWithoutReply;
    QDBusMessage message;
    QString ssid;
    qint64 createTime;
};

class NetSecretAgent : public NetworkManager::SecretAgent, public NetSecretAgentInterface
{
    Q_OBJECT
public:
    explicit NetSecretAgent(PasswordCallbackFunc fun, bool greeter = false, QObject *parent = nullptr);

    ~NetSecretAgent() override;

    bool hasTask() override;
    void inputPassword(const QString &key, const QVariantMap &password, bool input) override;

Q_SIGNALS:
    void secretsError(const QString &connectionPath, const QString &message);

public Q_SLOTS:
    NMVariantMapMap GetSecrets(const NMVariantMapMap &, const QDBusObjectPath &, const QString &, const QStringList &, uint) override;
    void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path) override;
    void DeleteSecrets(const NMVariantMapMap &, const QDBusObjectPath &) override;
    void CancelGetSecrets(const QDBusObjectPath &, const QString &) override;

    void onGetSecretsTimeout();
    void onInputPassword(const QString &key, const QString &password, const QString &identity, bool input);

private:
    void processNext();
    /**
     * @brief processGetSecrets requests
     * @param request the request we are processing
     * @param ignoreWallet true if the code should avoid Wallet
     * normally if it failed to open
     * @return true if the item was processed
     */
    bool processGetSecrets(SecretsRequest &request);
    bool processSaveSecrets(SecretsRequest &request) const;
    bool processDeleteSecrets(SecretsRequest &request) const;

    /**
     * @brief hasSecrets verifies if the desired connection has secrets to store
     * @param connection map with or without secrets
     * @return true if the connection has secrets, false otherwise
     */
    bool hasSecrets(const NMVariantMapMap &connection) const;
    void sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const;
    bool needConnectNetwork(const NMVariantMapMap &connectionMap) const;

    QString m_ssid;
    QString m_secret;
    QList<SecretsRequest> m_calls;
    bool m_greeter;
};

} // namespace network
} // namespace dde
#endif // NETSECRETAGENT_H
