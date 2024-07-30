/*
    SPDX-FileCopyrightText: 2013-2016 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013, 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connectioneditorbase.h"

#include "plasma_nm_editor.h"
#include "settings/bondwidget.h"
#include "settings/bridgewidget.h"
#include "settings/btwidget.h"
#include "settings/cdmawidget.h"
#include "settings/connectionwidget.h"
#include "settings/gsmwidget.h"
#include "settings/infinibandwidget.h"
#include "settings/ipv4widget.h"
#include "settings/ipv6widget.h"
#include "settings/pppoewidget.h"
#include "settings/pppwidget.h"
#include "settings/teamwidget.h"
#include "settings/vlanwidget.h"
#include "settings/wificonnectionwidget.h"
#include "settings/wifisecurity.h"
#include "settings/wiredconnectionwidget.h"
#include "settings/wiredsecurity.h"
#include "settings/wireguardinterfacewidget.h"
#include "uiutils.h"
#include "vpnuiplugin.h"

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/AdslSetting>
#include <NetworkManagerQt/CdmaSetting>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/PppoeSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include <KLocalizedString>
#include <KNotification>
#include <KUser>

ConnectionEditorBase::ConnectionEditorBase(const NetworkManager::ConnectionSettings::Ptr &connection, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_initialized(false)
    , m_valid(false)
    , m_pendingReplies(0)
    , m_connection(connection)
{
}

ConnectionEditorBase::ConnectionEditorBase(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
}

ConnectionEditorBase::~ConnectionEditorBase()
{
    m_connection.clear();
}

void ConnectionEditorBase::setConnection(const NetworkManager::ConnectionSettings::Ptr &connection)
{
    // Set connection settings
    m_connection.clear();
    m_connection = connection;
    m_initialized = false;
    m_wifiSecurity = nullptr;

    // Reset UI setting widgets
    delete m_connectionWidget;
    m_connectionWidget = nullptr;
    qDeleteAll(m_settingWidgets);
    m_settingWidgets.clear();

    initialize();
}

NMVariantMapMap ConnectionEditorBase::setting() const
{
    NMVariantMapMap settings = m_connectionWidget->setting();

    for (SettingWidget *widget : m_settingWidgets) {
        const QString type = widget->type();
        if (type != NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x)
            && type != NetworkManager::Setting::typeAsString(NetworkManager::Setting::WirelessSecurity)) {
            settings.insert(type, widget->setting());
        }

        // add 802.1x security if needed
        QVariantMap security8021x;
        if (type == NetworkManager::Setting::typeAsString(NetworkManager::Setting::WirelessSecurity)) {
            auto wifiSecurity = static_cast<WifiSecurity *>(widget);
            if (wifiSecurity->enabled()) {
                settings.insert(type, wifiSecurity->setting());
            }
            if (wifiSecurity->enabled8021x()) {
                security8021x = static_cast<WifiSecurity *>(widget)->setting8021x();
                settings.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x), security8021x);
            }
        } else if (type == NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x)) {
            auto wiredSecurity = static_cast<WiredSecurity *>(widget);
            if (wiredSecurity->enabled8021x()) {
                security8021x = static_cast<WiredSecurity *>(widget)->setting();
                settings.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x), security8021x);
            }
        }
    }

    // Set properties which are not returned from setting widgets
    NetworkManager::ConnectionSettings::Ptr connectionSettings =
        NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(m_connection->connectionType()));

    connectionSettings->fromMap(settings);
    connectionSettings->setId(connectionName());
    for (SettingWidget *widget : m_settingWidgets) {
        // WireGuard also defines the connection settings interface name
        if (widget->type() == NetworkManager::Setting::typeAsString(NetworkManager::Setting::WireGuard)) {
            auto wireGuard = static_cast<WireGuardInterfaceWidget *>(widget);
            connectionSettings->setInterfaceName(wireGuard->interfaceName());
        }
    }
    connectionSettings->setUuid(m_connection->uuid());

    if (connectionSettings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
        NetworkManager::WirelessSecuritySetting::Ptr securitySetting =
            connectionSettings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
        NetworkManager::WirelessSetting::Ptr wirelessSetting =
            connectionSettings->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();

        if (securitySetting && wirelessSetting) {
            if (securitySetting->keyMgmt() != NetworkManager::WirelessSecuritySetting::WirelessSecuritySetting::Unknown) {
                wirelessSetting->setSecurity(QStringLiteral("802-11-wireless-security"));
            }

            if (securitySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::SAE
                && wirelessSetting->mode() == NetworkManager::WirelessSetting::Adhoc) {
                // Ad-Hoc settings as specified by the supplicant
                // Proto
                QList<NetworkManager::WirelessSecuritySetting::WpaProtocolVersion> protoVersions = securitySetting->proto();
                protoVersions << NetworkManager::WirelessSecuritySetting::Rsn;
                securitySetting->setProto(protoVersions);
                // Pairwise
                QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities> pairwiseEncrypts = securitySetting->pairwise();
                pairwiseEncrypts << NetworkManager::WirelessSecuritySetting::Ccmp;
                securitySetting->setPairwise(pairwiseEncrypts);
                // Group
                QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities> groupEncrypts = securitySetting->group();
                groupEncrypts << NetworkManager::WirelessSecuritySetting::Ccmp;
                securitySetting->setGroup(groupEncrypts);
            }
        }
    }
    return connectionSettings->toMap();
}

bool ConnectionEditorBase::isInitialized() const
{
    return m_initialized;
}

bool ConnectionEditorBase::isValid() const
{
    return m_valid;
}

void ConnectionEditorBase::addConnectionWidget(ConnectionWidget *widget, const QString &text)
{
    m_connectionWidget = widget;

    connect(widget, &ConnectionWidget::settingChanged, this, &ConnectionEditorBase::settingChanged);

    addWidget(widget, text);
}

void ConnectionEditorBase::addSettingWidget(SettingWidget *widget, const QString &text)
{
    m_settingWidgets << widget;

    connect(widget, &SettingWidget::settingChanged, this, &ConnectionEditorBase::settingChanged);

    addWidget(widget, text);
}

void ConnectionEditorBase::initialize()
{
    const bool emptyConnection = m_connection->id().isEmpty();
    const NetworkManager::ConnectionSettings::ConnectionType type = m_connection->connectionType();

    if (emptyConnection) {
        UiUtils::setConnectionDefaultPermissions(m_connection);
    }

    // General configuration common to all connection types
    auto connectionWidget = new ConnectionWidget(m_connection);
    addConnectionWidget(connectionWidget, i18nc("General", "General configuration"));
    connect(connectionWidget, &ConnectionWidget::allUsersChanged, this, &ConnectionEditorBase::onAllUsersChanged);

    // Add the rest of widgets
    QString serviceType;
    if (type == NetworkManager::ConnectionSettings::Wired) {
        auto wiredWidget = new WiredConnectionWidget(m_connection->setting(NetworkManager::Setting::Wired), this);
        addSettingWidget(wiredWidget, i18n("Wired"));
        auto wiredSecurity =
            new WiredSecurity(m_connection->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>(), this);
        addSettingWidget(wiredSecurity, i18n("802.1x Security"));
    } else if (type == NetworkManager::ConnectionSettings::Wireless) {
        auto wifiWidget = new WifiConnectionWidget(m_connection->setting(NetworkManager::Setting::Wireless), this);
        addSettingWidget(wifiWidget, i18n("Wi-Fi"));
        auto wifiSecurity = new WifiSecurity(m_connection->setting(NetworkManager::Setting::WirelessSecurity),
                                             m_connection->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>(),
                                             this);
        addSettingWidget(wifiSecurity, i18n("Wi-Fi Security"));
        connect(wifiWidget, QOverload<const QString &>::of(&WifiConnectionWidget::ssidChanged), wifiSecurity, &WifiSecurity::onSsidChanged);
        m_wifiSecurity = wifiSecurity;
    } else if (type == NetworkManager::ConnectionSettings::Pppoe) { // DSL
        auto pppoeWidget = new PppoeWidget(m_connection->setting(NetworkManager::Setting::Pppoe), this);
        addSettingWidget(pppoeWidget, i18n("DSL"));
        auto wiredWidget = new WiredConnectionWidget(m_connection->setting(NetworkManager::Setting::Wired), this);
        addSettingWidget(wiredWidget, i18n("Wired"));
    } else if (type == NetworkManager::ConnectionSettings::Gsm) { // GSM
        auto gsmWidget = new GsmWidget(m_connection->setting(NetworkManager::Setting::Gsm), this);
        addSettingWidget(gsmWidget, i18n("Mobile Broadband (%1)", m_connection->typeAsString(m_connection->connectionType())));
    } else if (type == NetworkManager::ConnectionSettings::Cdma) { // CDMA
        auto cdmaWidget = new CdmaWidget(m_connection->setting(NetworkManager::Setting::Cdma), this);
        addSettingWidget(cdmaWidget, i18n("Mobile Broadband (%1)", m_connection->typeAsString(m_connection->connectionType())));
    } else if (type == NetworkManager::ConnectionSettings::Bluetooth) { // Bluetooth
        auto btWidget = new BtWidget(m_connection->setting(NetworkManager::Setting::Bluetooth), this);
        addSettingWidget(btWidget, i18n("Bluetooth"));
        NetworkManager::BluetoothSetting::Ptr btSetting =
            m_connection->setting(NetworkManager::Setting::Bluetooth).staticCast<NetworkManager::BluetoothSetting>();
        if (btSetting->profileType() == NetworkManager::BluetoothSetting::Dun) {
            auto gsmWidget = new GsmWidget(m_connection->setting(NetworkManager::Setting::Gsm), this);
            addSettingWidget(gsmWidget, i18n("GSM"));
            auto pppWidget = new PPPWidget(m_connection->setting(NetworkManager::Setting::Ppp), this);
            addSettingWidget(pppWidget, i18n("PPP"));
        }
    } else if (type == NetworkManager::ConnectionSettings::Infiniband) { // Infiniband
        auto infinibandWidget = new InfinibandWidget(m_connection->setting(NetworkManager::Setting::Infiniband), this);
        addSettingWidget(infinibandWidget, i18n("Infiniband"));
    } else if (type == NetworkManager::ConnectionSettings::Bond) { // Bond
        auto bondWidget = new BondWidget(m_connection->uuid(), m_connection->id(), m_connection->setting(NetworkManager::Setting::Bond), this);
        addSettingWidget(bondWidget, i18n("Bond"));
    } else if (type == NetworkManager::ConnectionSettings::Bridge) { // Bridge
        auto bridgeWidget = new BridgeWidget(m_connection->uuid(), m_connection->id(), m_connection->setting(NetworkManager::Setting::Bridge), this);
        addSettingWidget(bridgeWidget, i18n("Bridge"));
    } else if (type == NetworkManager::ConnectionSettings::Vlan) { // Vlan
        auto vlanWidget = new VlanWidget(m_connection->setting(NetworkManager::Setting::Vlan), this);
        addSettingWidget(vlanWidget, i18n("Vlan"));
    } else if (type == NetworkManager::ConnectionSettings::Team) { // Team
        auto teamWidget = new TeamWidget(m_connection->uuid(), m_connection->id(), m_connection->setting(NetworkManager::Setting::Team), this);
        addSettingWidget(teamWidget, i18n("Team"));
    } else if (type == NetworkManager::ConnectionSettings::WireGuard) { // WireGuard
        auto wireGuardInterfaceWidget = new WireGuardInterfaceWidget(m_connection, this);
        addSettingWidget(wireGuardInterfaceWidget, i18n("WireGuard Interface"));
    } else if (type == NetworkManager::ConnectionSettings::Vpn) { // VPN
        NetworkManager::VpnSetting::Ptr vpnSetting = m_connection->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
        if (!vpnSetting) {
            qCWarning(PLASMA_NM_EDITOR_LOG) << "Missing VPN setting!";
        } else {
            serviceType = vpnSetting->serviceType();

            const auto result = VpnUiPlugin::loadPluginForType(this, serviceType);

            if (result) {
                const QString shortName = serviceType.section('.', -1);
                SettingWidget *vpnWidget = result.plugin->widget(vpnSetting, this);
                addSettingWidget(vpnWidget, i18n("VPN (%1)", shortName));
            } else {
                qCWarning(PLASMA_NM_EDITOR_LOG) << "Could not instantiate VPN UI plugin" << result.errorText;
            }
        }
    }

    // PPP widget
    if (type == NetworkManager::ConnectionSettings::Pppoe || type == NetworkManager::ConnectionSettings::Cdma
        || type == NetworkManager::ConnectionSettings::Gsm) {
        auto pppWidget = new PPPWidget(m_connection->setting(NetworkManager::Setting::Ppp), this);
        addSettingWidget(pppWidget, i18n("PPP"));
    }

    // IPv4 widget
    if (!m_connection->isSlave()) {
        auto ipv4Widget = new IPv4Widget(m_connection->setting(NetworkManager::Setting::Ipv4), this);
        addSettingWidget(ipv4Widget, i18n("IPv4"));
    }

    // IPv6 widget
    if ((type == NetworkManager::ConnectionSettings::Wired //
         || type == NetworkManager::ConnectionSettings::Wireless //
         || type == NetworkManager::ConnectionSettings::Infiniband //
         || type == NetworkManager::ConnectionSettings::Team //
         || type == NetworkManager::ConnectionSettings::Cdma //
         || type == NetworkManager::ConnectionSettings::Gsm //
         || type == NetworkManager::ConnectionSettings::Bond //
         || type == NetworkManager::ConnectionSettings::Bridge //
         || type == NetworkManager::ConnectionSettings::Vlan //
         || type == NetworkManager::ConnectionSettings::WireGuard //
         || type == NetworkManager::ConnectionSettings::Loopback //
         || (type == NetworkManager::ConnectionSettings::Vpn && serviceType == QLatin1String("org.freedesktop.NetworkManager.openvpn")))
        && !m_connection->isSlave()) {
        auto ipv6Widget = new IPv6Widget(m_connection->setting(NetworkManager::Setting::Ipv6), this);
        addSettingWidget(ipv6Widget, i18n("IPv6"));
    }

    // Re-check validation
    bool valid = true;
    for (SettingWidget *widget : std::as_const(m_settingWidgets)) {
        valid = valid && widget->isValid();
        connect(widget, &SettingWidget::validChanged, this, &ConnectionEditorBase::validChanged);
    }

    m_valid = valid;
    Q_EMIT validityChanged(valid);

    KAcceleratorManager::manage(this);

    // If the connection is not empty (not new) we want to load its secrets
    if (!emptyConnection) {
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(m_connection->uuid());
        if (connection) {
            QStringList requiredSecrets;
            QString settingName;
            QVariantMap setting;
            QDBusPendingReply<NMVariantMapMap> reply;

            if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Adsl) {
                NetworkManager::AdslSetting::Ptr adslSetting =
                    connection->settings()->setting(NetworkManager::Setting::Adsl).staticCast<NetworkManager::AdslSetting>();
                if (adslSetting && !adslSetting->needSecrets().isEmpty()) {
                    requiredSecrets = adslSetting->needSecrets();
                    setting = adslSetting->toMap();
                    settingName = QStringLiteral("adsl");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Bluetooth) {
                NetworkManager::GsmSetting::Ptr gsmSetting =
                    connection->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && !gsmSetting->needSecrets().isEmpty()) {
                    requiredSecrets = gsmSetting->needSecrets();
                    setting = gsmSetting->toMap();
                    settingName = QStringLiteral("gsm");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Cdma) {
                NetworkManager::CdmaSetting::Ptr cdmaSetting =
                    connection->settings()->setting(NetworkManager::Setting::Cdma).staticCast<NetworkManager::CdmaSetting>();
                if (cdmaSetting && !cdmaSetting->needSecrets().isEmpty()) {
                    requiredSecrets = cdmaSetting->needSecrets();
                    setting = cdmaSetting->toMap();
                    settingName = QStringLiteral("cdma");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
                NetworkManager::GsmSetting::Ptr gsmSetting =
                    connection->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && !gsmSetting->needSecrets().isEmpty()) {
                    requiredSecrets = gsmSetting->needSecrets();
                    setting = gsmSetting->toMap();
                    settingName = QStringLiteral("gsm");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Pppoe) {
                NetworkManager::PppoeSetting::Ptr pppoeSetting =
                    connection->settings()->setting(NetworkManager::Setting::Pppoe).staticCast<NetworkManager::PppoeSetting>();
                if (pppoeSetting && !pppoeSetting->needSecrets().isEmpty()) {
                    requiredSecrets = pppoeSetting->needSecrets();
                    setting = pppoeSetting->toMap();
                    settingName = QStringLiteral("pppoe");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Wired) {
                NetworkManager::Security8021xSetting::Ptr securitySetting =
                    connection->settings()->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                if (securitySetting && !securitySetting->needSecrets().isEmpty()) {
                    requiredSecrets = securitySetting->needSecrets();
                    setting = securitySetting->toMap();
                    settingName = QStringLiteral("802-1x");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::WireGuard) {
                NetworkManager::WireGuardSetting::Ptr securitySetting =
                    connection->settings()->setting(NetworkManager::Setting::WireGuard).staticCast<NetworkManager::WireGuardSetting>();
                if (securitySetting && !securitySetting->needSecrets().isEmpty()) {
                    requiredSecrets = securitySetting->needSecrets();
                    setting = securitySetting->toMap();
                    settingName = QStringLiteral("wireguard");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
                NetworkManager::WirelessSecuritySetting::Ptr wifiSecuritySetting =
                    connection->settings()->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                if (wifiSecuritySetting
                    && (wifiSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEap
                        || wifiSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192
                        || (wifiSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WirelessSecuritySetting::Ieee8021x
                            && wifiSecuritySetting->authAlg() != NetworkManager::WirelessSecuritySetting::Leap))) {
                    NetworkManager::Security8021xSetting::Ptr securitySetting =
                        connection->settings()->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                    if (securitySetting && !securitySetting->needSecrets().isEmpty()) {
                        requiredSecrets = securitySetting->needSecrets();
                        setting = securitySetting->toMap();
                        settingName = QStringLiteral("802-1x");

                        if (requiredSecrets.contains(NM_SETTING_802_1X_PASSWORD_RAW)) {
                            requiredSecrets.removeAll(NM_SETTING_802_1X_PASSWORD_RAW);
                        }
                    }
                } else {
                    if (!wifiSecuritySetting->needSecrets().isEmpty()) {
                        requiredSecrets = wifiSecuritySetting->needSecrets();
                        setting = wifiSecuritySetting->toMap();
                        settingName = QStringLiteral("802-11-wireless-security");
                    }
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
                settingName = QStringLiteral("vpn");
            }

            if (!requiredSecrets.isEmpty() || m_connection->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
                bool requestSecrets = false;
                if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
                    NetworkManager::VpnSetting::Ptr vpnSetting =
                        connection->settings()->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
                    for (const QString &key : vpnSetting->data().keys()) {
                        if (key.endsWith(QStringLiteral("-flags"))) {
                            NetworkManager::Setting::SecretFlagType secretFlag = (NetworkManager::Setting::SecretFlagType)vpnSetting->data().value(key).toInt();
                            if (secretFlag == NetworkManager::Setting::None || secretFlag == NetworkManager::Setting::AgentOwned) {
                                requestSecrets = true;
                            }
                        }
                    }
                } else {
                    for (const QString &secret : std::as_const(requiredSecrets)) {
                        if (setting.contains(secret + QLatin1String("-flags"))) {
                            NetworkManager::Setting::SecretFlagType secretFlag =
                                (NetworkManager::Setting::SecretFlagType)setting.value(secret + QLatin1String("-flags")).toInt();
                            if (secretFlag == NetworkManager::Setting::None || secretFlag == NetworkManager::Setting::AgentOwned) {
                                requestSecrets = true;
                            }
                        } else {
                            requestSecrets = true;
                        }
                    }
                }

                if (requestSecrets) {
                    m_pendingReplies++;
                    reply = connection->secrets(settingName);
                    auto watcher = new QDBusPendingCallWatcher(reply, this);
                    watcher->setProperty("connection", connection->name());
                    watcher->setProperty("settingName", settingName);
                    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ConnectionEditorBase::replyFinished);
                    m_valid = false;
                    Q_EMIT validityChanged(false);
                    return;
                }
            }
        }
    }

    // We should be now fully initialized as we don't wait for secrets
    if (m_pendingReplies == 0) {
        m_initialized = true;
    }
}

void ConnectionEditorBase::replyFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<NMVariantMapMap> reply = *watcher;
    const QString settingName = watcher->property("settingName").toString();
    if (reply.isValid()) {
        NMVariantMapMap secrets = reply.argumentAt<0>();
        for (const QString &key : secrets.keys()) {
            if (key == settingName) {
                NetworkManager::Setting::Ptr setting = m_connection->setting(NetworkManager::Setting::typeFromString(key));
                if (setting) {
                    setting->secretsFromMap(secrets.value(key));
                    for (SettingWidget *widget : std::as_const(m_settingWidgets)) {
                        const QString type = widget->type();
                        if (type == settingName
                            || (settingName == NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x)
                                && type == NetworkManager::Setting::typeAsString(NetworkManager::Setting::WirelessSecurity))) {
                            widget->loadSecrets(setting);
                        }
                    }
                }
            }
        }
    } else {
        auto notification = new KNotification(QStringLiteral("FailedToGetSecrets"), KNotification::CloseOnTimeout);
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setTitle(i18n("Failed to get secrets for %1", watcher->property("connection").toString()));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    }

    watcher->deleteLater();
    validChanged(true);

    // We should be now fully with secrets
    m_pendingReplies--;
    m_initialized = true;
}

void ConnectionEditorBase::validChanged(bool valid)
{
    if (!valid) {
        m_valid = false;
        Q_EMIT validityChanged(false);
        return;
    } else {
        for (SettingWidget *widget : std::as_const(m_settingWidgets)) {
            if (!widget->isValid()) {
                m_valid = false;
                Q_EMIT validityChanged(false);
                return;
            }
        }
    }

    m_valid = true;
    Q_EMIT validityChanged(true);
}

void ConnectionEditorBase::onAllUsersChanged()
{
    if (!m_wifiSecurity) {
        return;
    }

    auto allUsers = m_connectionWidget->allUsers();
    m_wifiSecurity->setStoreSecretsSystemWide(allUsers);
}

#include "moc_connectioneditorbase.cpp"
