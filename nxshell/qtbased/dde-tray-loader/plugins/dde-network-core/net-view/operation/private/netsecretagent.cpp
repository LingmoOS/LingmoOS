// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netsecretagent.h"
#include "networkconst.h"

#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

#include <QDBusConnection>
#include <QJsonArray>
#include <QStringBuilder>
#include <QtDBus/qdbusmetatype.h>
#include <QTimer>

static const QString NetworkDialogApp = "dde-network-dialog"; // 网络列表执行文件

namespace dde {
namespace network {

NetSecretAgent::NetSecretAgent(PasswordCallbackFunc fun, bool greeter, QObject *parent)
    : NetworkManager::SecretAgent(QStringLiteral("com.deepin.system.network.SecretAgent"), parent)
    , NetSecretAgentInterface(fun)
{
    m_greeter = greeter;
    qCInfo(DNC) << "Register secret agent, com.deepin.system.network.SecretAgent mode: " << (m_greeter ? "greeter" : "lock");

    qDBusRegisterMetaType<NMVariantMapMap>();
}

bool NetSecretAgent::hasTask()
{
    return !m_calls.isEmpty();
}

void NetSecretAgent::inputPassword(const QString &key, const QVariantMap &password, bool input)
{
    onInputPassword(key, password.value(m_secret).toString(), password.value("identity").toString(), input);
}

NetSecretAgent::~NetSecretAgent() = default;

NMVariantMapMap NetSecretAgent::GetSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path, const QString &setting_name, const QStringList &hints, uint flags)
{
    qCDebug(DNC) << "Get secrets, path: " << connection_path.path() << ", setting name: " << setting_name << ", hints: " << hints << ", flags: " << flags;

    const QString callId = connection_path.path() % setting_name;
    for (const SecretsRequest &request : m_calls) {
        if (request == callId) {
            qCWarning(DNC) << "Get secrets was called again! This should not happen, cancelling first call, connection path: " << connection_path.path() << ", setting name: " << setting_name;
            CancelGetSecrets(connection_path, setting_name);
            break;
        }
    }

    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::GetSecrets);
    request.callId = callId;
    request.connection = connection;
    request.connection_path = connection_path;
    request.flags = static_cast<NetworkManager::SecretAgent::GetSecretsFlags>(flags);
    request.hints = hints;
    request.setting_name = setting_name;
    request.message = message();
    m_calls << request;

    processNext();
    QTimer::singleShot(GET_SECRETS_TIMEOUT, this, &NetSecretAgent::onGetSecretsTimeout);

    return {};
}

void NetSecretAgent::SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    setDelayedReply(true);
    SecretsRequest::Type type;
    if (hasSecrets(connection)) {
        type = SecretsRequest::SaveSecrets;
    } else {
        type = SecretsRequest::DeleteSecrets;
    }
    SecretsRequest request(type);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void NetSecretAgent::DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::DeleteSecrets);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void NetSecretAgent::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
    QString callId = connection_path.path() % setting_name;
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls.at(i);
        if (request.type == SecretsRequest::GetSecrets && callId == request.callId) {
            if (m_ssid == request.ssid) {
                qCDebug(DNC) << "Process finished (agent canceled)";
                cancelRequestPassword(QString(), m_ssid);
                m_ssid.clear();
            }
            sendError(SecretAgent::AgentCanceled, QStringLiteral("Agent canceled the password dialog"), request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    processNext();
}

void NetSecretAgent::onGetSecretsTimeout()
{
    qint64 time = QDateTime::currentDateTime().toMSecsSinceEpoch() + 1000 - GET_SECRETS_TIMEOUT;
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls.at(i);
        if (request.type == SecretsRequest::GetSecrets && request.createTime <= time) {
            if (m_ssid == request.ssid) {
                qCDebug(DNC) << "Process finished (Timeout)";
                cancelRequestPassword(QString(), m_ssid);
                m_ssid.clear();
            }
            m_calls.removeAt(i);
            break;
        }
    }
}

void NetSecretAgent::onInputPassword(const QString &key, const QString &password, const QString &identity, bool input)
{
    QString ssid = key;
    if (ssid.isEmpty())
        ssid = m_ssid;

    for (auto &&it = m_calls.begin(); it != m_calls.end();) {
        SecretsRequest &request = *it;
        if (request.type == SecretsRequest::GetSecrets && request.ssid == key) {
            if (input) {
                QJsonObject resultJsonObj;
                QJsonArray secretsJsonArray;
                secretsJsonArray.append(password);
                resultJsonObj.insert("secrets", secretsJsonArray);

                NetworkManager::ConnectionSettings::Ptr connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(request.connection));
                NetworkManager::Setting::Ptr setting = connectionSettings->setting(request.setting_name);
                auto needSecrets = setting->needSecrets(request.flags & RequestNew);
                if (!password.isEmpty() && !needSecrets.isEmpty()) {
                    QVariantMap result;
                    result.insert("identity", identity);
                    result.insert(needSecrets.first(), password);
                    request.connection[request.setting_name] = result;
                    sendSecrets(request.connection, request.message);
                }
            } else {
                sendError(SecretAgent::UserCanceled, QStringLiteral("user canceled"), request.message);
            }
            if (m_ssid == key)
                m_ssid.clear();
            it = m_calls.erase(it);
        } else {
            ++it;
        }
    }
}

void NetSecretAgent::processNext()
{
    int i = 0;
    while (i < m_calls.size()) {
        SecretsRequest &request = m_calls[i];
        switch (request.type) {
        case SecretsRequest::GetSecrets:
            if (processGetSecrets(request)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        case SecretsRequest::SaveSecrets:
            if (processSaveSecrets(request)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        case SecretsRequest::DeleteSecrets:
            if (processDeleteSecrets(request)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        }
        ++i;
    }
}

bool NetSecretAgent::processGetSecrets(SecretsRequest &request)
{
    if (!m_ssid.isEmpty()) {
        return false;
    }

    NetworkManager::ConnectionSettings::Ptr connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(request.connection));
    NetworkManager::Setting::Ptr setting = connectionSettings->setting(request.setting_name);

    const bool requestNew = request.flags & RequestNew;
    const bool userRequested = request.flags & UserRequested;
    const bool allowInteraction = request.flags & AllowInteraction;
    const bool isVpn = (setting->type() == NetworkManager::Setting::Vpn);

    if (isVpn) {
        NetworkManager::VpnSetting::Ptr vpnSetting = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
        if (vpnSetting->serviceType() == QLatin1String("org.freedesktop.NetworkManager.ssh") && vpnSetting->data()["auth-type"] == QLatin1String("ssh-agent")) {
            QString authSock = qgetenv("SSH_AUTH_SOCK");
            qCDebug(DNC) << "Sending SSH auth socket: " << authSock;

            if (authSock.isEmpty()) {
                sendError(SecretAgent::NoSecrets, QStringLiteral("SSH_AUTH_SOCK not present"), request.message);
            } else {
                NMStringMap secrets;
                secrets.insert(QStringLiteral("ssh-auth-sock"), authSock);

                QVariantMap secretData;
                secretData.insert(QStringLiteral("secrets"), QVariant::fromValue<NMStringMap>(secrets));
                request.connection[request.setting_name] = secretData;
                sendSecrets(request.connection, request.message);
            }
            return true;
        }
    }

    if (!m_greeter) {
        NMStringMap secretsMap;
        if (!requestNew) {
            // 需要去解锁密码环，取密码
            return false;
        }

        if (!secretsMap.isEmpty()) {
            setting->secretsFromStringMap(secretsMap);
            if (!(isVpn) && setting->needSecrets(requestNew).isEmpty()) {
                // Enough secrets were retrieved from storage
                request.connection[request.setting_name] = setting->secretsToMap();
                sendSecrets(request.connection, request.message);
                return true;
            }
        }
    }

    if (requestNew || (allowInteraction && !setting->needSecrets(requestNew).isEmpty()) || (allowInteraction && userRequested) || (isVpn && allowInteraction)) {
        qCDebug(DNC) << "Process request secrets";

        // 只处理无线的
        if (connectionSettings->connectionType() != NetworkManager::ConnectionSettings::Wireless) {
            sendError(SecretAgent::InternalError, QStringLiteral("dss can only handle wireless"), request.message);
            return true;
        }

        if (!userRequested && !allowInteraction) {
            sendError(SecretAgent::AgentCanceled, QStringLiteral("dss only support user quest"), request.message);
            return true;
        }

        QString devPath;
        NetworkManager::ActiveConnection::List actives = NetworkManager::activeConnections();
        for (auto &active : actives) {
            if (active->connection()->path() == request.connection_path.path() && !active->devices().isEmpty()) {
                devPath = active->devices().first();
                break;
            }
        }
        m_ssid = connectionSettings->id();
        request.ssid = m_ssid;
        qCDebug(DNC) << "Request password, device path: " << devPath << ", connection setting id: " << connectionSettings->id();
        const auto &requestSetting = request.connection.value(request.setting_name);
        QString identity = requestSetting.value("identity").toString();

        QVariantMap param;
        QStringList secrets;
        QMap<QString, QString> prop;
        if (!identity.isEmpty()) {
            secrets.append("identity");
            prop.insert("identity", identity);
        }
        m_secret = setting->needSecrets(requestNew).first();
        secrets.append(m_secret);
        param.insert("secrets", secrets);
        if (!prop.isEmpty()) {
            param.insert("prop", QVariant::fromValue(prop));
        }
        requestPassword(devPath, connectionSettings->id(), param);
        return false;
    } else if (isVpn && userRequested) { // just return what we have
        NMVariantMapMap result;
        NetworkManager::VpnSetting::Ptr vpnSetting;
        vpnSetting = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
        // FIXME workaround when NM is asking for secrets which should be system-stored, if we send an empty map it
        // won't ask for additional secrets with AllowInteraction flag which would display the authentication dialog
        if (vpnSetting->secretsToMap().isEmpty()) {
            // Insert an empty secrets map as it was before I fixed it in NetworkManagerQt to make sure NM will ask again
            // with flags we need
            QVariantMap secretsMap;
            secretsMap.insert(QStringLiteral("secrets"), QVariant::fromValue<NMStringMap>(NMStringMap()));
            result.insert(QStringLiteral("vpn"), secretsMap);
        } else {
            result.insert(QStringLiteral("vpn"), vpnSetting->secretsToMap());
        }
        sendSecrets(result, request.message);
        return true;
    } else if (setting->needSecrets().isEmpty()) {
        NMVariantMapMap result;
        result.insert(setting->name(), setting->secretsToMap());
        sendSecrets(result, request.message);
        return true;
    } else {
        sendError(SecretAgent::InternalError, QStringLiteral("dss did not know how to handle the request"), request.message);
        return true;
    }
}

bool NetSecretAgent::processSaveSecrets(SecretsRequest &request) const
{
    if (!request.saveSecretsWithoutReply) {
        QDBusMessage reply = request.message.createReply();
        if (!QDBusConnection::systemBus().send(reply)) {
            qCWarning(DNC) << "Failed put save secrets reply into the queue, reply: " << reply;
        }
    }

    return true;
}

bool NetSecretAgent::processDeleteSecrets(SecretsRequest &request) const
{
    QDBusMessage reply = request.message.createReply();
    if (!QDBusConnection::systemBus().send(reply)) {
        qCWarning(DNC) << "Failed put delete secrets reply into the queue, reply: " << reply;
    }

    return true;
}

bool NetSecretAgent::hasSecrets(const NMVariantMapMap &connection) const
{
    NetworkManager::ConnectionSettings connectionSettings(connection);
    for (const NetworkManager::Setting::Ptr &setting : connectionSettings.settings()) {
        if (!setting->secretsToMap().isEmpty()) {
            return true;
        }
    }

    return false;
}

void NetSecretAgent::sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const
{
    QDBusMessage reply;
    reply = message.createReply(QVariant::fromValue(secrets));
    if (!QDBusConnection::systemBus().send(reply)) {
        qCWarning(DNC) << "Failed put the secret into the queue, reply: " << reply;
    }
}
} // namespace network
} // namespace dde
