/*
    Copyright 2013-2014 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "handler.h"
#include "configuration.h"
#include "uiutils.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Ipv4Setting>

#include <libnm/nm-vpn-plugin-info.h>

#if WITH_MODEMMANAGER_SUPPORT
#include <ModemManagerQt/Manager>
#include <ModemManagerQt/ModemDevice>
#endif

#include <QDBusError>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include <QIcon>

#include <sys/types.h>
#include <pwd.h>

#define AGENT_SERVICE "org.kde.kded5"
#define AGENT_PATH "/modules/networkmanagement"
#define AGENT_IFACE "org.kde.plasmanetworkmanagement"

// 10 seconds
#define NM_REQUESTSCAN_LIMIT_RATE 10000

Handler::Handler(QObject *parent)
    : QObject(parent)
    , m_tmpWirelessEnabled(NetworkManager::isWirelessEnabled())
    , m_tmpWwanEnabled(NetworkManager::isWwanEnabled())
{
    ::passwd *pw = ::getpwuid(::getuid());
    m_userName = QString::fromLocal8Bit(pw->pw_name);

    QDBusConnection::sessionBus().connect(QStringLiteral(AGENT_SERVICE),
                                            QStringLiteral(AGENT_PATH),
                                            QStringLiteral(AGENT_IFACE),
                                            QStringLiteral("secretsError"),
                                            this, SLOT(secretAgentError(QString, QString)));


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

Handler::~Handler()
{
}

void Handler::activateConnection(const QString& connection, const QString& device, const QString& specificObject)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con) {
        qWarning() << "Not possible to activate this connection";
        return;
    }

    if (con->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
        NetworkManager::VpnSetting::Ptr vpnSetting = con->settings()->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
        if (vpnSetting) {
            qDebug() << "Checking VPN" << con->name() << "type:" << vpnSetting->serviceType();

            // bool pluginMissing = false;

            // // Check missing plasma-nm VPN plugin
            // const KService::List services = KServiceTypeTrader::self()->query("PlasmaNetworkManagement/VpnUiPlugin",
            //                                                                   QString::fromLatin1("[X-NetworkManager-Services]=='%1'").arg(vpnSetting->serviceType()));
            // pluginMissing = services.isEmpty();

            // // Check missing NetworkManager VPN plugin
            // if (!pluginMissing) {
            //     GSList *plugins = nullptr;
            //     plugins = nm_vpn_plugin_info_list_load();

            //     NMVpnPluginInfo *plugin_info = nm_vpn_plugin_info_list_find_by_service(plugins, vpnSetting->serviceType().toStdString().c_str());
            //     pluginMissing = !plugin_info;
            // }

            // if (pluginMissing) {
            //     qWarning() << "VPN" << vpnSetting->serviceType() << "not found, skipping";
            //     KNotification *notification = new KNotification("MissingVpnPlugin", KNotification::CloseOnTimeout, this);
            //     notification->setComponentName("networkmanagement");
            //     notification->setTitle(con->name());
            //     notification->setText(i18n("Missing VPN plugin"));
            //     notification->setIconName(QStringLiteral("dialog-warning"));
            //     notification->sendEvent();
            //     return;
            // }

        }
    }

#if WITH_MODEMMANAGER_SUPPORT
    if (con->settings()->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
        NetworkManager::ModemDevice::Ptr nmModemDevice = NetworkManager::findNetworkInterface(device).objectCast<NetworkManager::ModemDevice>();
        if (nmModemDevice) {
            ModemManager::ModemDevice::Ptr mmModemDevice = ModemManager::findModemDevice(nmModemDevice->udi());
            if (mmModemDevice) {
                ModemManager::Modem::Ptr modem = mmModemDevice->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
                NetworkManager::GsmSetting::Ptr gsmSetting = con->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && gsmSetting->pinFlags() == NetworkManager::Setting::NotSaved &&
                    modem && modem->unlockRequired() > MM_MODEM_LOCK_NONE) {
                    QDBusInterface managerIface("org.kde.plasmanetworkmanagement", "/org/kde/plasmanetworkmanagement", "org.kde.plasmanetworkmanagement", QDBusConnection::sessionBus(), this);
                    managerIface.call("unlockModem", mmModemDevice->uni());
                    connect(modem.data(), &ModemManager::Modem::unlockRequiredChanged, this, &Handler::unlockRequiredChanged);
                    m_tmpConnectionPath = connection;
                    m_tmpDevicePath = device;
                    m_tmpSpecificPath = specificObject;
                    return;
                }
            }
        }
    }
#endif

    QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::activateConnection(connection, device, specificObject);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("action", Handler::ActivateConnection);
    watcher->setProperty("connection", con->name());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
}

QString Handler::wifiCode(const QString& connectionPath, const QString& ssid, int _securityType) const
{
    NetworkManager::WirelessSecurityType securityType = static_cast<NetworkManager::WirelessSecurityType>(_securityType);

    QString ret = QStringLiteral("WIFI:S:") + ssid + QLatin1Char(';');
    if (securityType != NetworkManager::NoneSecurity) {
        switch (securityType) {
            case NetworkManager::NoneSecurity:
                break;
            case NetworkManager::StaticWep:
                ret += "T:WEP;";
                break;
            case NetworkManager::WpaPsk:
            case NetworkManager::Wpa2Psk:
                ret += "T:WPA;";
                break;
            case NetworkManager::SAE:
                ret += "T:SAE;";
                break;
            default:
            case NetworkManager::DynamicWep:
            case NetworkManager::WpaEap:
            case NetworkManager::Wpa2Eap:
            case NetworkManager::Leap:
                return {};
        }
    }

    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(connectionPath);
    if(!connection)
        return {};

    const auto key = QStringLiteral("802-11-wireless-security");
    auto reply = connection->secrets(key);

    const auto secret = reply.argumentAt<0>()[key];
    QString pass;
    switch (securityType) {
        case NetworkManager::NoneSecurity:
            break;
        case NetworkManager::WpaPsk:
        case NetworkManager::Wpa2Psk:
        case NetworkManager::SAE:
            pass = secret["psk"].toString();
            break;
        default:
            return {};
    }
    if (!pass.isEmpty())
        ret += QStringLiteral("P:") + pass + QLatin1Char(';');

    return ret + QLatin1Char(';');
}

void Handler::addAndActivateConnection(const QString& device, const QString& specificObject, const QString& password)
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
        return;
    }

    NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    settings->setId(ap->ssid());
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(true);
    settings->addToPermissions(m_userName, QString());

    NetworkManager::WirelessSetting::Ptr wifiSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setInitialized(true);
    wifiSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifiSetting->setSsid(ap->ssid().toUtf8());
    if (ap->mode() == NetworkManager::AccessPoint::Adhoc) {
        wifiSetting->setMode(NetworkManager::WirelessSetting::Adhoc);
    }
    NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();

    NetworkManager::WirelessSecurityType securityType = NetworkManager::findBestWirelessSecurity(wifiDev->wirelessCapabilities(), true, (ap->mode() == NetworkManager::AccessPoint::Adhoc), ap->capabilities(), ap->wpaFlags(), ap->rsnFlags());

    if (securityType != NetworkManager::NoneSecurity) {
        wifiSecurity->setInitialized(true);
        wifiSetting->setSecurity("802-11-wireless-security");
    }

    if (securityType == NetworkManager::Leap ||
        securityType == NetworkManager::DynamicWep ||
        securityType == NetworkManager::Wpa2Eap ||
        securityType == NetworkManager::WpaEap) {
        if (securityType == NetworkManager::DynamicWep || securityType == NetworkManager::Leap) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
            if (securityType == NetworkManager::Leap) {
                wifiSecurity->setAuthAlg(NetworkManager::WirelessSecuritySetting::Leap);
            }
        } else {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
        }
        m_tmpConnectionUuid = settings->uuid();
        m_tmpDevicePath = device;
        m_tmpSpecificPath = specificObject;

        // QPointer<ConnectionEditorDialog> editor = new ConnectionEditorDialog(settings);
        // editor->show();
        // KWindowSystem::setState(editor->winId(), NET::KeepAbove);
        // KWindowSystem::forceActiveWindow(editor->winId());
        // connect(editor.data(), &ConnectionEditorDialog::accepted,
        //         [editor, this] () {
        //             addConnection(editor->setting());
        //         });
        // connect(editor.data(), &ConnectionEditorDialog::finished,
        //         [editor] () {
        //             if (editor) {
        //                 editor->deleteLater();
        //             }
        //         });
        // editor->setModal(true);
        // editor->show();
    } else {
        if (securityType == NetworkManager::StaticWep) {
            wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
            wifiSecurity->setWepKey0(password);
            // if (KWallet::Wallet::isEnabled()) {
            //     wifiSecurity->setWepKeyFlags(NetworkManager::Setting::AgentOwned);
            // }
        } else {
            if (ap->mode() == NetworkManager::AccessPoint::Adhoc) {
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
            } else {
                wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
            }
            wifiSecurity->setPsk(password);
            // if (KWallet::Wallet::isEnabled()) {
            //     wifiSecurity->setPskFlags(NetworkManager::Setting::AgentOwned);
            // }
        }
        QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addAndActivateConnection(settings->toMap(), device, specificObject);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        watcher->setProperty("action", Handler::AddAndActivateConnection);
        watcher->setProperty("connection", settings->name());
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
    }

    settings.clear();
}

void Handler::addConnection(const NMVariantMapMap& map)
{
    QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addConnection(map);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("action", AddConnection);
    watcher->setProperty("connection", map.value("connection").value("id"));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
}

void Handler::deactivateConnection(const QString& connection, const QString& device)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con) {
        qWarning() << "Not possible to deactivate this connection";
        return;
    }

    QDBusPendingReply<> reply;
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        if (active->uuid() == con->uuid() && ((!active->devices().isEmpty() && active->devices().first() == device) ||
                                               active->vpn())) {
            if (active->vpn()) {
                reply = NetworkManager::deactivateConnection(active->path());
            } else {
                NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(active->devices().first());
                if (device) {
                    reply = device->disconnectInterface();
                }
            }
        }
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("action", Handler::DeactivateConnection);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
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

template<typename T>
void makeDBusCall(const QDBusMessage &message, QObject *context, std::function<void(QDBusPendingReply<T>)> func)
{
    QDBusPendingReply<T> reply = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, context);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, context, [func] (QDBusPendingCallWatcher *watcher) {
        const QDBusPendingReply<T> reply = *watcher;
        if (!reply.isValid()) {
            qWarning() << reply.error().message();
            return;
        }
        func(reply);
        watcher->deleteLater();
    });
}

void setBluetoothEnabled(QString path, bool enabled)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.bluez", path, "org.freedesktop.DBus.Properties", "Set");
    QList<QVariant> arguments;
    arguments << QLatin1String("org.bluez.Adapter1");
    arguments << QLatin1String("Powered");
    arguments << QVariant::fromValue(QDBusVariant(QVariant(enabled)));
    message.setArguments(arguments);
    QDBusConnection::systemBus().asyncCall(message);
}

void Handler::enableBluetooth(bool enable)
{
    qDBusRegisterMetaType< QMap<QDBusObjectPath, NMVariantMapMap > >();

    const QDBusMessage getObjects = QDBusMessage::createMethodCall("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");

    makeDBusCall<QMap<QDBusObjectPath, NMVariantMapMap>>(getObjects, this, [enable, this](const auto reply) {
        for (const QDBusObjectPath &path : reply.value().keys()) {
            const QString objPath = path.path();
            qDebug() << "inspecting path" << objPath;
            const QStringList interfaces = reply.value().value(path).keys();
            qDebug() << "interfaces:" << interfaces;

            if (!interfaces.contains("org.bluez.Adapter1")) {
                continue;
            }

            // We need to check previous state first
            if (!enable) {
                QDBusMessage getPowered = QDBusMessage::createMethodCall("org.bluez", objPath, "org.freedesktop.DBus.Properties", "Get");
                const QList<QVariant> arguments { QLatin1String("org.bluez.Adapter1"), QLatin1String("Powered") };
                getPowered.setArguments(arguments);

                makeDBusCall<QVariant>(getPowered, this, [objPath, this](const auto reply){
                    m_bluetoothAdapters.insert(objPath, reply.value().toBool());
                    setBluetoothEnabled(objPath, false);
                });
            } else if (m_bluetoothAdapters.value(objPath)) {
                setBluetoothEnabled(objPath, true);
            }
        }
    });
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

void Handler::removeConnection(const QString& connection)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con || con->uuid().isEmpty()) {
        qWarning() << "Not possible to remove connection " << connection;
        return;
    }

    // Remove slave connections
    for (const NetworkManager::Connection::Ptr &connection : NetworkManager::listConnections()) {
        NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
        if (settings->master() == con->uuid()) {
            connection->remove();
        }
    }

    QDBusPendingReply<> reply = con->remove();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("action", Handler::RemoveConnection);
    watcher->setProperty("connection", con->name());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
}

void Handler::updateConnection(const NetworkManager::Connection::Ptr& connection, const NMVariantMapMap& map)
{
    QDBusPendingReply<> reply = connection->update(map);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("action", UpdateConnection);
    watcher->setProperty("connection", connection->name());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
}

void Handler::requestScan(const QString &interface)
{
    for (NetworkManager::Device::Ptr device : NetworkManager::networkInterfaces()) {
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
                    qDebug() << "Rescheduling a request scan for" << wifiDevice->interfaceName() << "in" << timeout;
                    scheduleRequestScan(wifiDevice->interfaceName(), timeout);

                    if (!interface.isEmpty()) {
                        return;
                    }
                    continue;
                } else if (m_wirelessScanRetryTimer.contains(interface)){
                    m_wirelessScanRetryTimer.value(interface)->stop();
                    delete m_wirelessScanRetryTimer.take(interface);
                }

                qDebug() << "Requesting wifi scan on device" << wifiDevice->interfaceName();
                QDBusPendingReply<> reply = wifiDevice->requestScan();
                QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
                watcher->setProperty("action", Handler::RequestScan);
                watcher->setProperty("interface", wifiDevice->interfaceName());
                connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
            }
        }
    }
}

void Handler::createHotspot()
{
    bool foundInactive = false;
    bool useApMode = false;
    NetworkManager::WirelessDevice::Ptr wifiDev;

    NetworkManager::ConnectionSettings::Ptr connectionSettings;
    connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));

    NetworkManager::WirelessSetting::Ptr wifiSetting = connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
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
        qWarning() << "Failed to create hotspot: missing wireless device";
        return;
    }

    wifiSetting->setInitialized(true);
    wifiSetting->setMode(useApMode ? NetworkManager::WirelessSetting::Ap :NetworkManager::WirelessSetting::Adhoc);

    if (!Configuration::self().hotspotPassword().isEmpty()) {
        NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity = connectionSettings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
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

    const QVariantMap options = { {QLatin1String("persist"), QLatin1String("volatile")} };

    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath, QVariantMap> reply = NetworkManager::addAndActivateConnection2(connectionSettings->toMap(), wifiDev->uni(), QString(), options);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    watcher->setProperty("action", Handler::CreateHotspot);
    watcher->setProperty("connection", Configuration::self().hotspotName());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Handler::replyFinished);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, QOverload<QDBusPendingCallWatcher *>::of(&Handler::hotspotCreated));
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
        qDebug() << "Last scan finished " << lastScan.msecsTo(now) << "ms ago and last request scan was sent "
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
    emit connectionActivationFailed(connectionPath, message);
}

void Handler::replyFinished(QDBusPendingCallWatcher * watcher)
{
    // QDBusPendingReply<> reply = *watcher;
    // if (reply.isError() || !reply.isValid()) {
    //     KNotification *notification = nullptr;
    //     QString error = reply.error().message();
    //     Handler::HandlerAction action = (Handler::HandlerAction)watcher->property("action").toUInt();
    //     switch (action) {
    //         case Handler::ActivateConnection:
    //             notification = new KNotification("FailedToActivateConnection", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to activate %1", watcher->property("connection").toString()));
    //             break;
    //         case Handler::AddAndActivateConnection:
    //             notification = new KNotification("FailedToAddConnection", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to add %1", watcher->property("connection").toString()));
    //             break;
    //         case Handler::AddConnection:
    //             notification = new KNotification("FailedToAddConnection", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to add connection %1", watcher->property("connection").toString()));
    //             break;
    //         case Handler::DeactivateConnection:
    //             notification = new KNotification("FailedToDeactivateConnection", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to deactivate %1", watcher->property("connection").toString()));
    //             break;
    //         case Handler::RemoveConnection:
    //             notification = new KNotification("FailedToRemoveConnection", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to remove %1", watcher->property("connection").toString()));
    //             break;
    //         case Handler::UpdateConnection:
    //             notification = new KNotification("FailedToUpdateConnection", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to update connection %1", watcher->property("connection").toString()));
    //             break;
    //         case Handler::RequestScan:
    //         {
    //             const QString interface = watcher->property("interface").toString();
    //             qWarning() << "Wireless scan on" << interface << "failed:" << error;
    //             scanRequestFailed(interface);
    //             break;
    //         }
    //         case Handler::CreateHotspot:
    //             notification = new KNotification("FailedToCreateHotspot", KNotification::CloseOnTimeout, this);
    //             notification->setTitle(i18n("Failed to create hotspot %1", watcher->property("connection").toString()));
    //             break;
    //         default:
    //             break;
    //     }

    //     if (notification) {
    //         notification->setComponentName("networkmanagement");
    //         notification->setText(error);
    //         notification->setIconName(QStringLiteral("dialog-warning"));
    //         notification->sendEvent();
    //     }
    // } else {
    //     KNotification *notification = nullptr;
    //     Handler::HandlerAction action = (Handler::HandlerAction)watcher->property("action").toUInt();

    //     switch (action) {
    //         case Handler::AddConnection:
    //             notification = new KNotification("ConnectionAdded", KNotification::CloseOnTimeout, this);
    //             notification->setText(i18n("Connection %1 has been added", watcher->property("connection").toString()));
    //             break;
    //         case Handler::RemoveConnection:
    //             notification = new KNotification("ConnectionRemoved", KNotification::CloseOnTimeout, this);
    //             notification->setText(i18n("Connection %1 has been removed", watcher->property("connection").toString()));
    //             break;
    //         case Handler::UpdateConnection:
    //             notification = new KNotification("ConnectionUpdated", KNotification::CloseOnTimeout, this);
    //             notification->setText(i18n("Connection %1 has been updated", watcher->property("connection").toString()));
    //             break;
    //         case Handler::RequestScan:
    //             qDebug() << "Wireless scan on" << watcher->property("interface").toString() << "succeeded";
    //             break;
    //         default:
    //             break;
    //     }

    //     if (notification) {
    //         notification->setComponentName("networkmanagement");
    //         notification->setTitle(watcher->property("connection").toString());
    //         notification->setIconName(QStringLiteral("dialog-information"));
    //         notification->sendEvent();
    //     }
    // }

    watcher->deleteLater();
}

void Handler::hotspotCreated(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath, QVariantMap> reply = *watcher;

    if (!reply.isError() && reply.isValid()) {
        const QString activeConnectionPath = reply.argumentAt(1).value<QDBusObjectPath>().path();

        if (activeConnectionPath.isEmpty()) {
            return;
        }

        Configuration::self().setHotspotConnectionPath(activeConnectionPath);

        NetworkManager::ActiveConnection::Ptr hotspot = NetworkManager::findActiveConnection(activeConnectionPath);

        if (!hotspot) {
            return;
        }

        connect(hotspot.data(), &NetworkManager::ActiveConnection::stateChanged, [=] (NetworkManager::ActiveConnection::State state) {
            if (state > NetworkManager::ActiveConnection::Activated) {
                Configuration::self().setHotspotConnectionPath(QString());
                Q_EMIT hotspotDisabled();
            }
        });

        Q_EMIT hotspotCreated();
    }
}

void Handler::primaryConnectionTypeChanged(NetworkManager::ConnectionSettings::ConnectionType type)
{
    Q_UNUSED(type)
    m_hotspotSupported = checkHotspotSupported();
    Q_EMIT hotspotSupportedChanged(m_hotspotSupported);
}

#if WITH_MODEMMANAGER_SUPPORT
void Handler::unlockRequiredChanged(MMModemLock modemLock)
{
    if (modemLock == MM_MODEM_LOCK_NONE) {
        activateConnection(m_tmpConnectionPath, m_tmpDevicePath, m_tmpSpecificPath);
    }
}
#endif

