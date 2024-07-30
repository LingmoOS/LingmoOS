/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "secretagent.h"
#include "passworddialog.h"

#include "plasma_nm_kded.h"

#include "configuration.h"

#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WireguardSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include <QDBusConnection>
#include <QDialog>
#include <QStringBuilder>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KWallet>
#include <KWindowSystem>
#include <KX11Extras>

SecretAgent::SecretAgent(QObject *parent)
    : NetworkManager::SecretAgent(QStringLiteral("org.kde.plasma.networkmanagement"), NetworkManager::SecretAgent::Capability::VpnHints, parent)
    , m_openWalletFailed(false)
    , m_wallet(nullptr)
    , m_dialog(nullptr)
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::serviceDisappeared, this, &SecretAgent::killDialogs);

    // We have to import secrets previously stored in plaintext files
    importSecretsFromPlainTextFiles();
}

SecretAgent::~SecretAgent() = default;

NMVariantMapMap SecretAgent::GetSecrets(const NMVariantMapMap &connection,
                                        const QDBusObjectPath &connection_path,
                                        const QString &setting_name,
                                        const QStringList &hints,
                                        uint flags)
{
    qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO;
    qCDebug(PLASMA_NM_KDED_LOG) << "Path:" << connection_path.path();
    qCDebug(PLASMA_NM_KDED_LOG) << "Setting name:" << setting_name;
    qCDebug(PLASMA_NM_KDED_LOG) << "Hints:" << hints;
    qCDebug(PLASMA_NM_KDED_LOG) << "Flags:" << flags;

    const QString callId = connection_path.path() % setting_name;
    for (const SecretsRequest &request : std::as_const(m_calls)) {
        if (request == callId) {
            qCWarning(PLASMA_NM_KDED_LOG) << "GetSecrets was called again! This should not happen, cancelling first call" << connection_path.path()
                                          << setting_name;
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

    return {};
}

void SecretAgent::SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO;
    qCDebug(PLASMA_NM_KDED_LOG) << "Path:" << connection_path.path();
    // qCDebug(PLASMA_NM_KDED_LOG) << "Setting:" << connection;

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

void SecretAgent::DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO;
    qCDebug(PLASMA_NM_KDED_LOG) << "Path:" << connection_path.path();
    // qCDebug(PLASMA_NM_KDED_LOG) << "Setting:" << connection;

    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::DeleteSecrets);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void SecretAgent::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
    qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO;
    qCDebug(PLASMA_NM_KDED_LOG) << "Path:" << connection_path.path();
    qCDebug(PLASMA_NM_KDED_LOG) << "Setting name:" << setting_name;

    QString callId = connection_path.path() % setting_name;
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls.at(i);
        if (request.type == SecretsRequest::GetSecrets && callId == request.callId) {
            if (m_dialog == request.dialog) {
                m_dialog = nullptr;
            }
            delete request.dialog;
            sendError(SecretAgent::AgentCanceled, QStringLiteral("Agent canceled the password dialog"), request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    processNext();
}

void SecretAgent::dialogAccepted()
{
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets && request.dialog == m_dialog) {
            NMStringMap tmpOpenconnectSecrets;
            NMVariantMapMap connection = request.dialog->secrets();
            if (connection.contains(QStringLiteral("vpn"))) {
                if (connection.value(QStringLiteral("vpn")).contains(QStringLiteral("tmp-secrets"))) {
                    QVariantMap vpnSetting = connection.value(QStringLiteral("vpn"));
                    tmpOpenconnectSecrets = qdbus_cast<NMStringMap>(vpnSetting.take(QStringLiteral("tmp-secrets")));
                    connection.insert(QStringLiteral("vpn"), vpnSetting);
                }
            }

            sendSecrets(connection, request.message);
            NetworkManager::ConnectionSettings::Ptr connectionSettings =
                NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(connection));
            NetworkManager::ConnectionSettings::Ptr completeConnectionSettings;
            NetworkManager::Connection::Ptr con = NetworkManager::findConnectionByUuid(connectionSettings->uuid());
            if (con) {
                completeConnectionSettings = con->settings();
            } else {
                completeConnectionSettings = connectionSettings;
            }
            if (request.saveSecretsWithoutReply && completeConnectionSettings->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
                bool requestOffline = true;
                if (completeConnectionSettings->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
                    NetworkManager::GsmSetting::Ptr gsmSetting =
                        completeConnectionSettings->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                    if (gsmSetting) {
                        if (gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved) //
                            || gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                            requestOffline = false;
                        } else if (gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotSaved) //
                                   || gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                            requestOffline = false;
                        }
                    }
                } else if (completeConnectionSettings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
                    NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting =
                        completeConnectionSettings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                    if (wirelessSecuritySetting
                        && ((wirelessSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEap)
                            || (wirelessSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192))) {
                        NetworkManager::Security8021xSetting::Ptr security8021xSetting =
                            completeConnectionSettings->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                        if (security8021xSetting) {
                            if (security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodFast) //
                                || security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodTtls) //
                                || security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodPeap)) {
                                if (security8021xSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved)
                                    || security8021xSetting->passwordFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                                    requestOffline = false;
                                }
                            }
                        }
                    }
                }

                if (requestOffline) {
                    SecretsRequest requestOffline(SecretsRequest::SaveSecrets);
                    requestOffline.connection = connection;
                    requestOffline.connection_path = request.connection_path;
                    requestOffline.saveSecretsWithoutReply = true;
                    m_calls << requestOffline;
                }
            } else if (request.saveSecretsWithoutReply && completeConnectionSettings->connectionType() == NetworkManager::ConnectionSettings::Vpn
                       && !tmpOpenconnectSecrets.isEmpty()) {
                NetworkManager::VpnSetting::Ptr vpnSetting =
                    completeConnectionSettings->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
                if (vpnSetting) {
                    NMStringMap data = vpnSetting->data();
                    NMStringMap secrets = vpnSetting->secrets();

                    // Load secrets from auth dialog which are returned back to NM
                    if (connection.value(QStringLiteral("vpn")).contains(QStringLiteral("secrets"))) {
                        secrets.insert(qdbus_cast<NMStringMap>(connection.value(QStringLiteral("vpn")).value(QStringLiteral("secrets"))));
                    }

                    // Load temporary secrets from auth dialog which are not returned to NM
                    for (const QString &key : tmpOpenconnectSecrets.keys()) {
                        if (secrets.contains(QStringLiteral("save_passwords")) && secrets.value(QStringLiteral("save_passwords")) == QLatin1String("yes")) {
                            data.insert(key + QLatin1String("-flags"), QString::number(NetworkManager::Setting::AgentOwned));
                        } else {
                            data.insert(key + QLatin1String("-flags"), QString::number(NetworkManager::Setting::NotSaved));
                        }
                        secrets.insert(key, tmpOpenconnectSecrets.value(key));
                    }

                    vpnSetting->setData(data);
                    vpnSetting->setSecrets(secrets);
                    if (!con) {
                        con = NetworkManager::findConnection(request.connection_path.path());
                    }

                    if (con) {
                        con->update(completeConnectionSettings->toMap());
                    }
                }
            }

            m_calls.removeAt(i);
            break;
        }
    }

    m_dialog->deleteLater();
    m_dialog = nullptr;

    processNext();
}

void SecretAgent::dialogRejected()
{
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets && request.dialog == m_dialog) {
            sendError(SecretAgent::UserCanceled, QStringLiteral("User canceled the password dialog"), request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    m_dialog->deleteLater();
    m_dialog = nullptr;

    processNext();
}

void SecretAgent::killDialogs()
{
    int i = 0;
    while (i < m_calls.size()) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets) {
            delete request.dialog;
            m_calls.removeAt(i);
        }

        ++i;
    }
}

void SecretAgent::walletOpened(bool success)
{
    if (!success) {
        m_openWalletFailed = true;
        m_wallet->deleteLater();
        m_wallet = nullptr;
    } else {
        m_openWalletFailed = false;
    }

    processNext();
}

void SecretAgent::walletClosed()
{
    if (m_wallet) {
        m_wallet->deleteLater();
    }
    m_wallet = nullptr;
}

void SecretAgent::processNext()
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

bool SecretAgent::processGetSecrets(SecretsRequest &request) const
{
    if (m_dialog) {
        return false;
    }

    NetworkManager::ConnectionSettings::Ptr connectionSettings =
        NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(request.connection));
    NetworkManager::Setting::Ptr setting = connectionSettings->setting(request.setting_name);

    const bool requestNew = request.flags & RequestNew;
    const bool userRequested = request.flags & UserRequested;
    const bool allowInteraction = request.flags & AllowInteraction;
    const bool isVpn = (setting->type() == NetworkManager::Setting::Vpn);
    const bool isWireGuard = (setting->type() == NetworkManager::Setting::WireGuard);

    if (isVpn) {
        NetworkManager::VpnSetting::Ptr vpnSetting = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
        if (vpnSetting->serviceType() == QLatin1String("org.freedesktop.NetworkManager.ssh") && vpnSetting->data()["auth-type"] == QLatin1String("ssh-agent")) {
            QString authSock = qgetenv("SSH_AUTH_SOCK");
            qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO << "Sending SSH auth socket" << authSock;

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

    NMStringMap secretsMap;
    if (!requestNew && useWallet()) {
        if (m_wallet->isOpen()) {
            if (m_wallet->hasFolder(QStringLiteral("Network Management")) && m_wallet->setFolder(QStringLiteral("Network Management"))) {
                QString key = QLatin1Char('{') % connectionSettings->uuid() % QLatin1Char('}') % QLatin1Char(';') % request.setting_name;
                m_wallet->readMap(key, secretsMap);
            }
        } else {
            qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO << "Waiting for the wallet to open";
            return false;
        }
    }

    if (!secretsMap.isEmpty()) {
        setting->secretsFromStringMap(secretsMap);
        if (!(isVpn || isWireGuard) && setting->needSecrets(requestNew).isEmpty()) {
            // Enough secrets were retrieved from storage
            request.connection[request.setting_name] = setting->secretsToMap();
            sendSecrets(request.connection, request.message);
            return true;
        }
    }

    if (!Configuration::self().showPasswordDialog()) {
        sendError(SecretAgent::NoSecrets, QStringLiteral("Cannot authenticate"), request.message);
        Q_EMIT secretsError(request.connection_path.path(),
                            i18n("Authentication to %1 failed. Wrong password?", request.connection.value("connection").value("id").toString()));
        return true;
    } else if (isWireGuard && userRequested) { // Just return what we have
        NMVariantMapMap result;
        NetworkManager::WireGuardSetting::Ptr wireGuardSetting;
        wireGuardSetting = connectionSettings->setting(NetworkManager::Setting::WireGuard).dynamicCast<NetworkManager::WireGuardSetting>();
        // FIXME workaround when NM is asking for secrets which should be system-stored, if we send an empty map it
        // won't ask for additional secrets with AllowInteraction flag which would display the authentication dialog
        if (wireGuardSetting->secretsToMap().isEmpty()) {
            // Insert an empty secrets map as it was before I fixed it in NetworkManagerQt to make sure NM will ask again
            // with flags we need
            QVariantMap secretsMap;
            secretsMap.insert(QStringLiteral("secrets"), QVariant::fromValue<NMStringMap>(NMStringMap()));
            result.insert(QStringLiteral("wireguard"), secretsMap);
        } else {
            result.insert(QStringLiteral("wireguard"), wireGuardSetting->secretsToMap());
        }
        sendSecrets(result, request.message);
        return true;
    } else if (requestNew || (allowInteraction && !setting->needSecrets(requestNew).isEmpty()) || (allowInteraction && userRequested)
               || (isVpn && allowInteraction)) {
        m_dialog = new PasswordDialog(connectionSettings, request.flags, request.setting_name, request.hints);
        connect(m_dialog, &PasswordDialog::accepted, this, &SecretAgent::dialogAccepted);
        connect(m_dialog, &PasswordDialog::rejected, this, &SecretAgent::dialogRejected);

        if (m_dialog->hasError()) {
            sendError(m_dialog->error(), m_dialog->errorMessage(), request.message);
            delete m_dialog;
            m_dialog = nullptr;
            return true;
        } else {
            request.dialog = m_dialog;
            request.saveSecretsWithoutReply = !connectionSettings->permissions().isEmpty();
            m_dialog->show();
            if (KWindowSystem::isPlatformX11()) {
                KX11Extras::setState(m_dialog->winId(), NET::KeepAbove);
            }
            return false;
        }
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
        sendError(SecretAgent::InternalError, QStringLiteral("Plasma-nm did not know how to handle the request"), request.message);
        return true;
    }
}

bool SecretAgent::processSaveSecrets(SecretsRequest &request) const
{
    if (useWallet()) {
        if (m_wallet->isOpen()) {
            NetworkManager::ConnectionSettings connectionSettings(request.connection);

            if (!m_wallet->hasFolder(QStringLiteral("Network Management"))) {
                m_wallet->createFolder(QStringLiteral("Network Management"));
            }

            if (m_wallet->setFolder(QStringLiteral("Network Management"))) {
                for (const NetworkManager::Setting::Ptr &setting : connectionSettings.settings()) {
                    NMStringMap secretsMap = setting->secretsToStringMap();

                    if (!secretsMap.isEmpty()) {
                        QString entryName = QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % setting->name();
                        m_wallet->writeMap(entryName, secretsMap);
                    }
                }
            } else if (!request.saveSecretsWithoutReply) {
                sendError(SecretAgent::InternalError, QStringLiteral("Could not store secrets in the wallet."), request.message);
                return true;
            }
        } else {
            qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO << "Waiting for the wallet to open";
            return false;
        }
    }

    if (!request.saveSecretsWithoutReply) {
        QDBusMessage reply = request.message.createReply();
        if (!QDBusConnection::systemBus().send(reply)) {
            qCWarning(PLASMA_NM_KDED_LOG) << "Failed put save secrets reply into the queue";
        }
    }

    return true;
}

bool SecretAgent::processDeleteSecrets(SecretsRequest &request) const
{
    if (useWallet()) {
        if (m_wallet->isOpen()) {
            if (m_wallet->hasFolder(QStringLiteral("Network Management")) && m_wallet->setFolder(QStringLiteral("Network Management"))) {
                NetworkManager::ConnectionSettings connectionSettings(request.connection);
                for (const NetworkManager::Setting::Ptr &setting : connectionSettings.settings()) {
                    QString entryName = QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % setting->name();
                    for (const QString &entry : m_wallet->entryList()) {
                        if (entry.startsWith(entryName)) {
                            m_wallet->removeEntry(entryName);
                        }
                    }
                }
            }
        } else {
            qCDebug(PLASMA_NM_KDED_LOG) << Q_FUNC_INFO << "Waiting for the wallet to open";
            return false;
        }
    }

    QDBusMessage reply = request.message.createReply();
    if (!QDBusConnection::systemBus().send(reply)) {
        qCWarning(PLASMA_NM_KDED_LOG) << "Failed put delete secrets reply into the queue";
    }

    return true;
}

bool SecretAgent::useWallet() const
{
    if (m_wallet) {
        return true;
    }

    /* If opening of KWallet failed before, we should not try to open it again and
     * we should return false instead */
    if (m_openWalletFailed) {
        m_openWalletFailed = false;
        return false;
    }

    if (KWallet::Wallet::isEnabled()) {
        m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0, KWallet::Wallet::Asynchronous);
        if (m_wallet) {
            connect(m_wallet, &KWallet::Wallet::walletOpened, this, &SecretAgent::walletOpened);
            connect(m_wallet, &KWallet::Wallet::walletClosed, this, &SecretAgent::walletClosed);
            return true;
        } else {
            qCWarning(PLASMA_NM_KDED_LOG) << "Error opening kwallet.";
        }
    } else if (m_wallet) {
        m_wallet->deleteLater();
        m_wallet = nullptr;
    }

    return false;
}

bool SecretAgent::hasSecrets(const NMVariantMapMap &connection) const
{
    NetworkManager::ConnectionSettings connectionSettings(connection);
    for (const NetworkManager::Setting::Ptr &setting : connectionSettings.settings()) {
        if (!setting->secretsToMap().isEmpty()) {
            return true;
        }
    }

    return false;
}

void SecretAgent::sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const
{
    QDBusMessage reply;
    reply = message.createReply(QVariant::fromValue(secrets));
    if (!QDBusConnection::systemBus().send(reply)) {
        qCWarning(PLASMA_NM_KDED_LOG) << "Failed put the secret into the queue";
    }
}

void SecretAgent::importSecretsFromPlainTextFiles()
{
    KConfig config(QStringLiteral("plasma-networkmanagement"), KConfig::SimpleConfig);

    // No action is required when the list of secrets is empty
    if (!config.groupList().isEmpty()) {
        for (const QString &groupName : config.groupList()) {
            QString loadedUuid = groupName.split(QLatin1Char(';')).first().remove('{').remove('}');
            QString loadedSettingType = groupName.split(QLatin1Char(';')).last();
            NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(loadedUuid);
            if (connection) {
                NetworkManager::Setting::SecretFlags secretFlags =
                    KWallet::Wallet::isEnabled() ? NetworkManager::Setting::AgentOwned : NetworkManager::Setting::None;
                QMap<QString, QString> secrets = config.entryMap(groupName);
                NMVariantMapMap settings = connection->settings()->toMap();

                for (const QString &setting : settings.keys()) {
                    if (setting == QLatin1String("vpn")) {
                        NetworkManager::VpnSetting::Ptr vpnSetting =
                            connection->settings()->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
                        if (vpnSetting) {
                            // Add loaded secrets from the config file
                            vpnSetting->secretsFromStringMap(secrets);

                            NMStringMap vpnData = vpnSetting->data();
                            // Reset flags, we can't save secrets to our secret agent when KWallet is not enabled, because
                            // we dropped support for plaintext files, therefore they need to be stored to NetworkManager
                            for (const QString &key : vpnData.keys()) {
                                if (key.endsWith(QLatin1String("-flags"))) {
                                    vpnData.insert(key, QString::number((int)secretFlags));
                                }
                            }

                            vpnSetting->setData(vpnData);
                            settings.insert(setting, vpnSetting->toMap());
                            connection->update(settings);
                        }
                    } else {
                        if (setting == loadedSettingType) {
                            QVariantMap tmpSetting = settings.value(setting);
                            // Reset flags, we can't save secrets to our secret agent when KWallet is not enabled, because
                            // we dropped support for plaintext files, therefore they need to be stored to NetworkManager
                            for (const QString &key : tmpSetting.keys()) {
                                if (key.endsWith(QLatin1String("-flags"))) {
                                    tmpSetting.insert(key, (int)secretFlags);
                                }
                            }

                            // Add loaded secrets from the config file
                            QMap<QString, QString>::const_iterator it = secrets.constBegin();
                            QMap<QString, QString>::const_iterator end = secrets.constEnd();
                            for (; it != end; ++it) {
                                tmpSetting.insert(it.key(), it.value());
                            }

                            // Replace the old setting with the new one
                            settings.insert(setting, tmpSetting);
                            // Update the connection which re-saves secrets
                            connection->update(settings);
                        }
                    }
                }
            }

            // Remove the group
            KConfigGroup group(&config, groupName);
            group.deleteGroup();
        }
    }
}

#include "moc_secretagent.cpp"
