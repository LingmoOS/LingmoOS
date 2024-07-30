/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "handler.h"
#include "configuration.h"
#include "connectioneditordialog.h"
#include "plasma_nm_libs.h"
#include "uiutils.h"

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>

#include <libnm/nm-vpn-plugin-info.h>

#include <ModemManagerQt/Manager>
#include <ModemManagerQt/ModemDevice>

#include <QDBusError>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QFile>
#include <QIcon>
#include <QStringBuilder>

#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KNotification>
#include <KOSRelease>
#include <KPluginMetaData>
#include <KProcess>
#include <KUser>
#include <KWallet>
#include <KWindowSystem>
#include <KX11Extras>

#include <QCoroCore>
#include <QCoroDBus>
#include <nm-client.h>

#define AGENT_SERVICE "org.kde.kded6"
#define AGENT_PATH "/modules/networkmanagement"
#define AGENT_IFACE "org.kde.plasmanetworkmanagement"

// 10 seconds
#define NM_REQUESTSCAN_LIMIT_RATE 10000

Handler::Handler(QObject *parent)
    : QObject(parent)
    , m_tmpWirelessEnabled(NetworkManager::isWirelessEnabled())
    , m_tmpWwanEnabled(NetworkManager::isWwanEnabled())
{
    QDBusConnection::sessionBus().connect(QStringLiteral(AGENT_SERVICE),
                                          QStringLiteral(AGENT_PATH),
                                          QStringLiteral(AGENT_IFACE),
                                          QStringLiteral("secretsError"),
                                          this,
                                          SLOT(secretAgentError(QString, QString)));

    if (!Configuration::self().hotspotConnectionPath().isEmpty()) {
        NetworkManager::ActiveConnection::Ptr hotspot = NetworkManager::findActiveConnection(Configuration::self().hotspotConnectionPath());
        if (!hotspot) {
            Configuration::self().setHotspotConnectionPath(QString());
        }
    }

    m_hotspotSupported = checkHotspotSupported();

    if (NetworkManager::checkVersion(1, 16, 0)) {
        connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionTypeChanged, this, &Handler::primaryConnectionTypeChanged);
    }
}

Handler::~Handler() = default;

void Handler::activateConnection(const QString &connection, const QString &device, const QString &specificObject)
{
    activateConnectionInternal(connection, device, specificObject);
}

QCoro::Task<void> Handler::activateConnectionInternal(const QString &connection, const QString &device, const QString &specificObject)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con) {
        qCWarning(PLASMA_NM_LIBS_LOG) << "Not possible to activate this connection";
        co_return;
    }

    if (con->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
        NetworkManager::VpnSetting::Ptr vpnSetting = con->settings()->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
        if (vpnSetting) {
            qCDebug(PLASMA_NM_LIBS_LOG) << "Checking VPN" << con->name() << "type:" << vpnSetting->serviceType();

            // Check missing plasma-nm VPN plugin

            const auto filter = [vpnSetting](const KPluginMetaData &md) -> bool {
                return md.value(QStringLiteral("X-NetworkManager-Services")) == vpnSetting->serviceType();
            };

            const QList<KPluginMetaData> plasmaNmPlugins = KPluginMetaData::findPlugins(QStringLiteral("plasma/network/vpn"), filter);

            const QString pluginBaseName = vpnSetting->serviceType().remove(QLatin1String("org.freedesktop.NetworkManager."));

            if (plasmaNmPlugins.empty()) {
                qCWarning(PLASMA_NM_LIBS_LOG) << "VPN" << vpnSetting->serviceType() << "not found, skipping";
                auto notification = new KNotification(QStringLiteral("MissingVpnPlugin"), KNotification::Persistent, this);
                notification->setComponentName(QStringLiteral("networkmanagement"));
                notification->setTitle(con->name());
                notification->setText(i18n("Plasma is missing support for '%1' VPN connections.", pluginBaseName));
                notification->setIconName(QStringLiteral("dialog-error"));

                auto reportBugAction = notification->addAction(i18n("Report Bug"));
                connect(reportBugAction, &KNotificationAction::activated, this, [notification] {
                    auto *job = new KIO::OpenUrlJob(QUrl(KOSRelease().bugReportUrl()));
                    job->setStartupId(notification->xdgActivationToken().toUtf8());
                    job->start();
                });

                notification->sendEvent();

                co_return;
            }

            // Check missing NetworkManager VPN plugin
            GSList *networkManagerPlugins = nullptr;
            networkManagerPlugins = nm_vpn_plugin_info_list_load();

            NMVpnPluginInfo *plugin_info = nm_vpn_plugin_info_list_find_by_service(networkManagerPlugins, vpnSetting->serviceType().toStdString().c_str());

            if (!plugin_info) {
                qCWarning(PLASMA_NM_LIBS_LOG) << "VPN" << vpnSetting->serviceType() << "not found, skipping";
                auto notification = new KNotification(QStringLiteral("MissingVpnPlugin"), KNotification::Persistent, this);
                notification->setComponentName(QStringLiteral("networkmanagement"));
                notification->setTitle(con->name());
                notification->setText(i18n("NetworkManager is missing support for '%1' VPN connections.", pluginBaseName));
                notification->setIconName(QStringLiteral("dialog-error"));

                auto installAction = notification->addAction(i18n("Install"));
                connect(installAction, &KNotificationAction::activated, this, [notification, pluginBaseName] {
                    auto *job = new KIO::OpenUrlJob(QUrl(QStringLiteral("appstream:network-manager-") + pluginBaseName));
                    job->setStartupId(notification->xdgActivationToken().toUtf8());
                    job->start();
                });

                notification->sendEvent();
                co_return;
            }
        }
    }

    if (con->settings()->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
        NetworkManager::ModemDevice::Ptr nmModemDevice = NetworkManager::findNetworkInterface(device).objectCast<NetworkManager::ModemDevice>();
        if (nmModemDevice) {
            ModemManager::ModemDevice::Ptr mmModemDevice = ModemManager::findModemDevice(nmModemDevice->udi());
            if (mmModemDevice) {
                ModemManager::Modem::Ptr modem = mmModemDevice->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
                NetworkManager::GsmSetting::Ptr gsmSetting = con->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && gsmSetting->pinFlags() == NetworkManager::Setting::NotSaved && modem && modem->unlockRequired() > MM_MODEM_LOCK_NONE) {
                    QDBusInterface managerIface(QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                QStringLiteral("/org/kde/plasmanetworkmanagement"),
                                                QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                QDBusConnection::sessionBus(),
                                                this);
                    managerIface.call(QStringLiteral("unlockModem"), mmModemDevice->uni());
                    connect(modem.data(), &ModemManager::Modem::unlockRequiredChanged, this, &Handler::unlockRequiredChanged);
                    m_tmpConnectionPath = connection;
                    m_tmpDevicePath = device;
                    m_tmpSpecificPath = specificObject;
                    co_return;
                }
            }
        }
    }

    QDBusReply<QDBusObjectPath> reply = co_await NetworkManager::activateConnection(connection, device, specificObject);

    if (!reply.isValid()) {
        QString error = reply.error().message();
        KNotification *notification = new KNotification(QStringLiteral("FailedToActivateConnection"), KNotification::CloseOnTimeout, this);
        notification->setTitle(i18n("Failed to activate %1", con->name()));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(error);
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    }
}

void Handler::requestWifiCode(const QString &connectionPath, const QString &ssid, int _securityType)
{
    if (!m_requestWifiCodeWatcher.isNull()) {
        delete m_requestWifiCodeWatcher;
    }

    auto securityType = static_cast<NetworkManager::WirelessSecurityType>(_securityType);

    QString ret = QStringLiteral("WIFI:S:") + ssid + QLatin1Char(';');
    if (securityType != NetworkManager::NoneSecurity) {
        switch (securityType) {
        case NetworkManager::NoneSecurity:
        case NetworkManager::OWE:
            break;
        case NetworkManager::StaticWep:
            ret += QStringLiteral("T:WEP;");
            break;
        case NetworkManager::WpaPsk:
        case NetworkManager::Wpa2Psk:
            ret += QStringLiteral("T:WPA;");
            break;
        case NetworkManager::SAE:
            ret += QStringLiteral("T:SAE;");
            break;
        default:
        case NetworkManager::DynamicWep:
        case NetworkManager::WpaEap:
        case NetworkManager::Wpa2Eap:
        case NetworkManager::Wpa3SuiteB192:
        case NetworkManager::Leap:
            Q_EMIT wifiCodeReceived(QString(), ssid);
            return;
        }
    }

    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(connectionPath);
    if (!connection) {
        Q_EMIT wifiCodeReceived(QString(), ssid);
        return;
    }

    const auto key = QStringLiteral("802-11-wireless-security");
    auto reply = connection->secrets(key);
    m_requestWifiCodeWatcher = new QDBusPendingCallWatcher(reply, this);
    m_requestWifiCodeWatcher->setProperty("key", key);
    m_requestWifiCodeWatcher->setProperty("ret", ret);
    m_requestWifiCodeWatcher->setProperty("securityType", static_cast<int>(securityType));
    m_requestWifiCodeWatcher->setProperty("ssid", ssid);
    connect(m_requestWifiCodeWatcher, &QDBusPendingCallWatcher::finished, this, &Handler::slotRequestWifiCode);
}

void Handler::addAndActivateConnection(const QString &device, const QString &specificParameter, const QString &password)
{
    addAndActivateConnectionInternal(device, specificParameter, password);
}

QCoro::Task<void> Handler::addAndActivateConnectionInternal(const QString &device, const QString &specificObject, const QString &password)
{
    NetworkManager::AccessPoint::Ptr ap;
    NetworkManager::WirelessDevice::Ptr wifiDev;
    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (dev->type() == NetworkManager::Device::Wifi) {
            wifiDev = dev.objectCast<NetworkManager::WirelessDevice>();
            ap = wifiDev->findAccessPoint(specificObject);
            if (ap) {
                break;
            }
        }
    }

    if (!ap) {
        co_return;
    }

    NetworkManager::ConnectionSettings::Ptr settings =
        NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    settings->setId(ap->ssid());
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(true);

    UiUtils::setConnectionDefaultPermissions(settings);

    NetworkManager::WirelessSetting::Ptr wifiSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setInitialized(true);
    wifiSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setSsid(ap->ssid().toUtf8());
    if (ap->mode() == NetworkManager::AccessPoint::Adhoc) {
        wifiSetting->setMode(NetworkManager::WirelessSetting::Adhoc);
    }
    NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity =
        settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();

    NetworkManager::WirelessSecurityType securityType = NetworkManager::findBestWirelessSecurity(wifiDev->wirelessCapabilities(),
                                                                                                 true,
                                                                                                 (ap->mode() == NetworkManager::AccessPoint::Adhoc),
                                                                                                 ap->capabilities(),
                                                                                                 ap->wpaFlags(),
                                                                                                 ap->rsnFlags());

    if (securityType != NetworkManager::NoneSecurity) {
        wifiSecurity->setInitialized(true);
        wifiSetting->setSecurity(QStringLiteral("802-11-wireless-security"));
    }

    if (securityType == NetworkManager::Leap //
        || securityType == NetworkManager::DynamicWep //
        || securityType == NetworkManager::Wpa3SuiteB192 //
        || securityType == NetworkManager::Wpa2Eap //
        || securityType == NetworkManager::WpaEap) {
        if (securityType == NetworkManager::DynamicWep || securityType == NetworkManager::Leap) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
            if (securityType == NetworkManager::Leap) {
                wifiSecurity->setAuthAlg(NetworkManager::WirelessSecuritySetting::Leap);
            }
        } else if (securityType == NetworkManager::Wpa3SuiteB192) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192);
        } else {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
        }
        m_tmpConnectionUuid = settings->uuid();
        m_tmpDevicePath = device;
        m_tmpSpecificPath = specificObject;

        QPointer<ConnectionEditorDialog> editor = new ConnectionEditorDialog(settings);
        editor->setAttribute(Qt::WA_DeleteOnClose);
        editor->show();

        if (KWindowSystem::isPlatformX11()) {
            KX11Extras::setState(editor->winId(), NET::KeepAbove);
        }

        connect(editor.data(), &ConnectionEditorDialog::accepted, [editor, device, specificObject, this]() { //
            addAndActivateConnectionDBus(editor->setting(), device, specificObject);
        });
        editor->setModal(true);
        editor->show();
    } else {
        if (securityType == NetworkManager::StaticWep) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
            wifiSecurity->setWepKey0(password);
        } else if (securityType == NetworkManager::OWE) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::OWE);
        } else {
            if (ap->mode() == NetworkManager::AccessPoint::Adhoc) {
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
            } else if (securityType == NetworkManager::SAE) {
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::SAE);
            } else {
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
            }
            wifiSecurity->setPsk(password);
        }
        addAndActivateConnectionDBus(settings->toMap(), device, specificObject);
    }

    settings.clear();
}

QCoro::Task<void> Handler::addConnection(const NMVariantMapMap &map)
{
    const QString connectionId = map.value(QStringLiteral("connection")).value(QStringLiteral("id")).toString();

    QDBusReply<QDBusObjectPath> reply = co_await NetworkManager::addConnection(map);

    if (!reply.isValid()) {
        KNotification *notification = new KNotification(QStringLiteral("FailedToAddConnection"), KNotification::CloseOnTimeout, this);
        notification->setTitle(i18n("Failed to add connection %1", connectionId));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    } else {
        KNotification *notification = new KNotification(QStringLiteral("ConnectionAdded"), KNotification::CloseOnTimeout, this);
        notification->setText(i18n("Connection %1 has been added", connectionId));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setTitle(connectionId);
        notification->setIconName(QStringLiteral("dialog-information"));
        notification->sendEvent();
    }
}

struct AddConnectionData {
    QString id;
    Handler *handler;
};

void add_connection_cb(GObject *client, GAsyncResult *result, gpointer user_data)
{
    AddConnectionData *data = static_cast<AddConnectionData *>(user_data);

    GError *error = nullptr;
    NMRemoteConnection *connection = nm_client_add_connection2_finish(NM_CLIENT(client), result, NULL, &error);

    if (error) {
        KNotification *notification = new KNotification(QStringLiteral("FailedToAddConnection"), KNotification::CloseOnTimeout, data->handler);
        notification->setTitle(i18n("Failed to add connection %1", data->id));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(QString::fromUtf8(error->message));
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();

        g_error_free(error);
    } else {
        KNotification *notification = new KNotification(QStringLiteral("ConnectionAdded"), KNotification::CloseOnTimeout, data->handler);
        notification->setText(i18n("Connection %1 has been added", data->id));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setTitle(data->id);
        notification->setIconName(QStringLiteral("dialog-information"));
        notification->sendEvent();

        g_object_unref(connection);
    }

    delete data;
}

void Handler::addConnection(NMConnection *connection)
{
    NMClient *client = nm_client_new(nullptr, nullptr);

    AddConnectionData *userData = new AddConnectionData{QString::fromUtf8(nm_connection_get_id(connection)), this};

    nm_client_add_connection2(client,
                              nm_connection_to_dbus(connection, NM_CONNECTION_SERIALIZE_ALL),
                              NM_SETTINGS_ADD_CONNECTION2_FLAG_TO_DISK,
                              nullptr,
                              true,
                              nullptr,
                              add_connection_cb,
                              userData);
}

QCoro::Task<void> Handler::addAndActivateConnectionDBus(const NMVariantMapMap &map, const QString &device, const QString &specificObject)
{
    const QString name = map.value(QStringLiteral("connection")).value(QStringLiteral("id")).toString();
    QDBusReply<QDBusObjectPath> reply = co_await NetworkManager::addAndActivateConnection(map, device, specificObject);

    if (!reply.isValid()) {
        KNotification *notification = new KNotification(QStringLiteral("FailedToAddConnection"), KNotification::CloseOnTimeout, this);
        notification->setTitle(i18n("Failed to add %1", name));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    }
}

void Handler::deactivateConnection(const QString &connection, const QString &device)
{
    deactivateConnectionInternal(connection, device);
}

QCoro::Task<void> Handler::deactivateConnectionInternal(const QString &_connection, const QString &device)
{
    const QString connection = _connection;
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con) {
        qCWarning(PLASMA_NM_LIBS_LOG) << "Not possible to deactivate this connection";
        co_return;
    }

    QDBusReply<void> reply;
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        if (active->uuid() == con->uuid() && ((!active->devices().isEmpty() && active->devices().first() == device) || active->vpn())) {
            if (active->vpn()) {
                reply = co_await NetworkManager::deactivateConnection(active->path());
            } else {
                NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(active->devices().first());
                if (device) {
                    reply = co_await device->disconnectInterface();
                }
            }
        }
    }

    if (!reply.isValid()) {
        KNotification *notification = new KNotification(QStringLiteral("FailedToDeactivateConnection"), KNotification::CloseOnTimeout, this);
        notification->setTitle(i18n("Failed to deactivate %1", connection));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    }
}

void Handler::disconnectAll()
{
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        device->disconnectInterface();
    }
}

void Handler::enableAirplaneMode(bool enable)
{
    if (enable) {
        m_tmpWirelessEnabled = NetworkManager::isWirelessEnabled();
        m_tmpWwanEnabled = NetworkManager::isWwanEnabled();
        enableBluetooth(false);
        enableWireless(false);
        enableWwan(false);
    } else {
        enableBluetooth(true);
        if (m_tmpWirelessEnabled) {
            enableWireless(true);
        }
        if (m_tmpWwanEnabled) {
            enableWwan(true);
        }
    }
}

void setBluetoothEnabled(const QString &path, bool enabled)
{
    QDBusMessage message =
        QDBusMessage::createMethodCall(QStringLiteral("org.bluez"), path, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("Set"));
    QList<QVariant> arguments;
    arguments << QLatin1String("org.bluez.Adapter1");
    arguments << QLatin1String("Powered");
    arguments << QVariant::fromValue(QDBusVariant(QVariant(enabled)));
    message.setArguments(arguments);
    QDBusConnection::systemBus().asyncCall(message);
}

QCoro::Task<void> Handler::enableBluetooth(bool enable)
{
    qDBusRegisterMetaType<QMap<QDBusObjectPath, NMVariantMapMap>>();

    const QDBusMessage getObjects = QDBusMessage::createMethodCall("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");

    QDBusReply<QMap<QDBusObjectPath, NMVariantMapMap>> reply = co_await QDBusConnection::systemBus().asyncCall(getObjects);

    if (!reply.isValid()) {
        qCWarning(PLASMA_NM_LIBS_LOG) << reply.error().message();
        co_return;
    }

    for (const QDBusObjectPath &path : reply.value().keys()) {
        const QString objPath = path.path();
        qCDebug(PLASMA_NM_LIBS_LOG) << "inspecting path" << objPath;
        const QStringList interfaces = reply.value().value(path).keys();
        qCDebug(PLASMA_NM_LIBS_LOG) << "interfaces:" << interfaces;

        if (!interfaces.contains(QStringLiteral("org.bluez.Adapter1"))) {
            continue;
        }

        // We need to check previous state first
        if (!enable) {
            QDBusMessage getPowered = QDBusMessage::createMethodCall("org.bluez", objPath, "org.freedesktop.DBus.Properties", "Get");
            const QList<QVariant> arguments{QLatin1String("org.bluez.Adapter1"), QLatin1String("Powered")};
            getPowered.setArguments(arguments);

            QDBusReply<QVariant> reply = co_await QDBusConnection::systemBus().asyncCall(getPowered);

            if (!reply.isValid()) {
                qCWarning(PLASMA_NM_LIBS_LOG) << reply.error().message();
                co_return;
            }

            m_bluetoothAdapters.insert(objPath, reply.value().toBool());
            setBluetoothEnabled(objPath, false);
        } else if (m_bluetoothAdapters.value(objPath)) {
            setBluetoothEnabled(objPath, true);
        }
    }
}

void Handler::enableNetworking(bool enable)
{
    NetworkManager::setNetworkingEnabled(enable);
}

void Handler::enableWireless(bool enable)
{
    NetworkManager::setWirelessEnabled(enable);
}

void Handler::enableWwan(bool enable)
{
    NetworkManager::setWwanEnabled(enable);
}

void Handler::removeConnection(const QString &connection)
{
    removeConnectionInternal(connection);
}

QCoro::Task<void> Handler::removeConnectionInternal(const QString &connection)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con || con->uuid().isEmpty()) {
        qCWarning(PLASMA_NM_LIBS_LOG) << "Not possible to remove connection " << connection;
        co_return;
    }

    // Remove slave connections
    for (const NetworkManager::Connection::Ptr &connection : NetworkManager::listConnections()) {
        NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
        if (settings->master() == con->uuid()) {
            connection->remove();
        }
    }

    QDBusReply<void> reply = co_await con->remove();

    if (!reply.isValid()) {
        KNotification *notification = new KNotification(QStringLiteral("FailedToRemoveConnection"), KNotification::CloseOnTimeout, this);
        notification->setTitle(i18n("Failed to remove %1", con->name()));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    } else {
        KNotification *notification = new KNotification(QStringLiteral("ConnectionRemoved"), KNotification::CloseOnTimeout, this);
        notification->setText(i18n("Connection %1 has been removed", con->name()));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setTitle(con->name());
        notification->setIconName(QStringLiteral("dialog-information"));
        notification->sendEvent();
    }
}

QCoro::Task<void> Handler::updateConnection(NetworkManager::Connection::Ptr connection, const NMVariantMapMap &map)
{
    QDBusReply<void> reply = co_await connection->update(map);

    KNotification *notification = nullptr;
    if (!reply.isValid()) {
        notification = new KNotification(QStringLiteral("FailedToUpdateConnection"), KNotification::CloseOnTimeout);
        notification->setTitle(i18n("Failed to update connection %1", connection->name()));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    } else {
        notification = new KNotification(QStringLiteral("ConnectionUpdated"), KNotification::CloseOnTimeout);
        notification->setText(i18n("Connection %1 has been updated", connection->name()));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setTitle(connection->name());
        notification->setIconName(QStringLiteral("dialog-information"));
        notification->sendEvent();
    }
    connect(notification, &KNotification::closed, notification, &QObject::deleteLater);
}

void Handler::requestScan(const QString &interface)
{
    requestScanInternal(interface);
}

QCoro::Task<void> Handler::requestScanInternal(const QString &interface)
{
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();

            if (wifiDevice && wifiDevice->state() != NetworkManager::WirelessDevice::Unavailable) {
                if (!interface.isEmpty() && interface != wifiDevice->interfaceName()) {
                    continue;
                }

                if (!checkRequestScanRateLimit(wifiDevice)) {
                    QDateTime now = QDateTime::currentDateTime();
                    // for NM < 1.12, lastScan is not available
                    QDateTime lastScan = wifiDevice->lastScan();
                    QDateTime lastRequestScan = wifiDevice->lastRequestScan();
                    // Compute the next time we can run a scan
                    int timeout = NM_REQUESTSCAN_LIMIT_RATE;
                    if (lastScan.isValid() && lastScan.msecsTo(now) < NM_REQUESTSCAN_LIMIT_RATE) {
                        timeout = NM_REQUESTSCAN_LIMIT_RATE - lastScan.msecsTo(now);
                    } else if (lastRequestScan.isValid() && lastRequestScan.msecsTo(now) < NM_REQUESTSCAN_LIMIT_RATE) {
                        timeout = NM_REQUESTSCAN_LIMIT_RATE - lastRequestScan.msecsTo(now);
                    }
                    qCDebug(PLASMA_NM_LIBS_LOG) << "Rescheduling a request scan for" << wifiDevice->interfaceName() << "in" << timeout;
                    scheduleRequestScan(wifiDevice->interfaceName(), timeout);

                    if (!interface.isEmpty()) {
                        co_return;
                    }
                    continue;
                } else if (m_wirelessScanRetryTimer.contains(interface)) {
                    m_wirelessScanRetryTimer.value(interface)->stop();
                    delete m_wirelessScanRetryTimer.take(interface);
                }

                qCDebug(PLASMA_NM_LIBS_LOG) << "Requesting wifi scan on device" << wifiDevice->interfaceName();

                incrementScansCount();
                QDBusReply<void> reply = co_await wifiDevice->requestScan();

                if (!reply.isValid()) {
                    const QString interface = wifiDevice->interfaceName();
                    qCWarning(PLASMA_NM_LIBS_LOG) << "Wireless scan on" << interface << "failed:" << reply.error().message();
                    scanRequestFailed(interface);
                } else {
                    qCDebug(PLASMA_NM_LIBS_LOG) << "Wireless scan on" << wifiDevice->interfaceName() << "succeeded";
                }
                decrementScansCount();
            }
        }
    }
}

void Handler::incrementScansCount()
{
    m_ongoingScansCount += 1;
    if (m_ongoingScansCount == 1) {
        Q_EMIT scanningChanged();
    }
}

void Handler::decrementScansCount()
{
    if (m_ongoingScansCount == 0) {
        qCDebug(PLASMA_NM_LIBS_LOG) << "Extra decrementScansCount() called";
        return;
    }
    m_ongoingScansCount -= 1;
    if (m_ongoingScansCount == 0) {
        Q_EMIT scanningChanged();
    }
}

void Handler::createHotspot()
{
    createHotspotInternal();
}

QCoro::Task<void> Handler::createHotspotInternal()
{
    bool foundInactive = false;
    bool useApMode = false;
    NetworkManager::WirelessDevice::Ptr wifiDev;

    NetworkManager::ConnectionSettings::Ptr connectionSettings;
    connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));

    NetworkManager::WirelessSetting::Ptr wifiSetting =
        connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setMode(NetworkManager::WirelessSetting::Adhoc);
    wifiSetting->setSsid(Configuration::self().hotspotName().toUtf8());

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            wifiDev = device.objectCast<NetworkManager::WirelessDevice>();
            if (wifiDev) {
                if (!wifiDev->isActive()) {
                    foundInactive = true;
                } else {
                    // Prefer previous device if it was inactive
                    if (foundInactive) {
                        break;
                    }
                }

                if (wifiDev->wirelessCapabilities().testFlag(NetworkManager::WirelessDevice::ApCap)) {
                    useApMode = true;
                }

                // We prefer inactive wireless card with AP capabilities
                if (foundInactive && useApMode) {
                    break;
                }
            }
        }
    }

    if (!wifiDev) {
        qCWarning(PLASMA_NM_LIBS_LOG) << "Failed to create hotspot: missing wireless device";
        co_return;
    }

    wifiSetting->setInitialized(true);
    wifiSetting->setMode(useApMode ? NetworkManager::WirelessSetting::Ap : NetworkManager::WirelessSetting::Adhoc);

    if (!Configuration::self().hotspotPassword().isEmpty()) {
        NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity =
            connectionSettings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
        wifiSecurity->setInitialized(true);

        if (useApMode) {
            // Use WPA2
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
            wifiSecurity->setPsk(Configuration::self().hotspotPassword());
            wifiSecurity->setPskFlags(NetworkManager::Setting::AgentOwned);
        } else {
            // Use WEP
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
            wifiSecurity->setWepKeyType(NetworkManager::WirelessSecuritySetting::Passphrase);
            wifiSecurity->setWepTxKeyindex(0);
            wifiSecurity->setWepKey0(Configuration::self().hotspotPassword());
            wifiSecurity->setWepKeyFlags(NetworkManager::Setting::AgentOwned);
            wifiSecurity->setAuthAlg(NetworkManager::WirelessSecuritySetting::Open);
        }
    }

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Shared);
    ipv4Setting->setInitialized(true);

    connectionSettings->setId(Configuration::self().hotspotName());
    connectionSettings->setAutoconnect(false);
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());

    const QVariantMap options = {{QLatin1String("persist"), QLatin1String("volatile")}};

    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath, QVariantMap> reply =
        co_await NetworkManager::addAndActivateConnection2(connectionSettings->toMap(), wifiDev->uni(), QString(), options);

    if (!reply.isValid()) {
        KNotification *notification = new KNotification(QStringLiteral("FailedToCreateHotspot"), KNotification::CloseOnTimeout, this);
        notification->setTitle(i18n("Failed to create hotspot %1", Configuration::self().hotspotName()));
        notification->setComponentName(QStringLiteral("networkmanagement"));
        notification->setText(reply.error().message());
        notification->setIconName(QStringLiteral("dialog-warning"));
        notification->sendEvent();
    } else {
        const QString activeConnectionPath = reply.argumentAt(1).value<QDBusObjectPath>().path();

        if (activeConnectionPath.isEmpty()) {
            co_return;
        }

        Configuration::self().setHotspotConnectionPath(activeConnectionPath);

        NetworkManager::ActiveConnection::Ptr hotspot = NetworkManager::findActiveConnection(activeConnectionPath);

        if (!hotspot) {
            co_return;
        }

        connect(hotspot.data(), &NetworkManager::ActiveConnection::stateChanged, [this](NetworkManager::ActiveConnection::State state) {
            if (state > NetworkManager::ActiveConnection::Activated) {
                Configuration::self().setHotspotConnectionPath(QString());
                Q_EMIT hotspotDisabled();
            }
        });

        Q_EMIT hotspotCreated();
    }
}

void Handler::stopHotspot()
{
    const QString activeConnectionPath = Configuration::self().hotspotConnectionPath();

    if (activeConnectionPath.isEmpty()) {
        return;
    }

    NetworkManager::ActiveConnection::Ptr hotspot = NetworkManager::findActiveConnection(activeConnectionPath);

    if (!hotspot) {
        return;
    }

    NetworkManager::deactivateConnection(activeConnectionPath);
    Configuration::self().setHotspotConnectionPath(QString());

    Q_EMIT hotspotDisabled();
}

bool Handler::checkRequestScanRateLimit(const NetworkManager::WirelessDevice::Ptr &wifiDevice)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastScan = wifiDevice->lastScan();
    QDateTime lastRequestScan = wifiDevice->lastRequestScan();

    // if the last scan finished within the last 10 seconds
    bool ret = lastScan.isValid() && lastScan.msecsTo(now) < NM_REQUESTSCAN_LIMIT_RATE;
    // or if the last Request was sent within the last 10 seconds
    ret |= lastRequestScan.isValid() && lastRequestScan.msecsTo(now) < NM_REQUESTSCAN_LIMIT_RATE;
    // skip the request scan
    if (ret) {
        qCDebug(PLASMA_NM_LIBS_LOG) << "Last scan finished" << lastScan.msecsTo(now) << "ms ago and last request scan was sent" //
                                    << lastRequestScan.msecsTo(now) << "ms ago, Skipping scanning interface:" << wifiDevice->interfaceName();
        return false;
    }
    return true;
}

bool Handler::checkHotspotSupported()
{
    if (NetworkManager::checkVersion(1, 16, 0)) {
        bool unusedWifiFound = false;
        bool wifiFound = false;

        for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
            if (device->type() == NetworkManager::Device::Wifi) {
                wifiFound = true;

                NetworkManager::WirelessDevice::Ptr wifiDev = device.objectCast<NetworkManager::WirelessDevice>();
                if (wifiDev && !wifiDev->isActive()) {
                    unusedWifiFound = true;
                }
            }
        }

        if (!wifiFound) {
            return false;
        }

        if (unusedWifiFound) {
            return true;
        }

        // Check if the primary connection which is used for internet connectivity is not using WiFi
        if (NetworkManager::primaryConnectionType() != NetworkManager::ConnectionSettings::Wireless) {
            return true;
        }
    }

    return false;
}

void Handler::scheduleRequestScan(const QString &interface, int timeout)
{
    QTimer *timer;
    if (!m_wirelessScanRetryTimer.contains(interface)) {
        // create a timer for the interface
        timer = new QTimer();
        timer->setSingleShot(true);
        m_wirelessScanRetryTimer.insert(interface, timer);
        auto retryAction = [this, interface]() {
            requestScan(interface);
        };
        connect(timer, &QTimer::timeout, this, retryAction);
    } else {
        // set the new value for an existing timer
        timer = m_wirelessScanRetryTimer.value(interface);
        if (timer->isActive()) {
            timer->stop();
        }
    }

    // +1 ms is added to avoid having the scan being rejetted by nm
    // because it is run at the exact last millisecond of the requestScan threshold
    timer->setInterval(timeout + 1);
    timer->start();
}

void Handler::scanRequestFailed(const QString &interface)
{
    scheduleRequestScan(interface, 2000);
}

void Handler::secretAgentError(const QString &connectionPath, const QString &message)
{
    // If the password was wrong, forget it
    removeConnection(connectionPath);
    Q_EMIT connectionActivationFailed(connectionPath, message);
}

void Handler::primaryConnectionTypeChanged(NetworkManager::ConnectionSettings::ConnectionType type)
{
    Q_UNUSED(type)
    m_hotspotSupported = checkHotspotSupported();
    Q_EMIT hotspotSupportedChanged(m_hotspotSupported);
}

void Handler::unlockRequiredChanged(MMModemLock modemLock)
{
    if (modemLock == MM_MODEM_LOCK_NONE) {
        activateConnection(m_tmpConnectionPath, m_tmpDevicePath, m_tmpSpecificPath);
    }
}

void Handler::slotRequestWifiCode(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();

    QString ret = watcher->property("ret").toString();
    const QString ssid = watcher->property("ssid").toString();
    QDBusPendingReply<NMVariantMapMap> reply = *watcher;
    if (!reply.isValid() || reply.isError()) {
        Q_EMIT wifiCodeReceived(ret % QLatin1Char(';'), ssid);
        return;
    }

    const auto secret = reply.argumentAt<0>()[watcher->property("key").toString()];
    QString pass;
    switch (static_cast<NetworkManager::WirelessSecurityType>(watcher->property("securityType").toInt())) {
    case NetworkManager::NoneSecurity:
    case NetworkManager::OWE:
        break;
    case NetworkManager::WpaPsk:
    case NetworkManager::Wpa2Psk:
    case NetworkManager::SAE:
        pass = secret[QStringLiteral("psk")].toString();
        break;
    default:
        Q_EMIT wifiCodeReceived(QString(), ssid);
        return;
    }
    if (!pass.isEmpty()) {
        ret += QStringLiteral("P:") % pass % QLatin1Char(';');
    }

    Q_EMIT wifiCodeReceived(ret % QLatin1Char(';'), ssid);
}

#include "moc_handler.cpp"
