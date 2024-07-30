/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notification.h"
#include "plasma_nm_kded.h"

#include <uiutils.h>

#include <NetworkManagerQt/Manager>

#include <KLocalizedString>
#include <KNotification>

#include <QDBusConnection>
#include <QIcon>
#include <QTimer>

#include <algorithm>
#include <chrono>

using namespace std::chrono_literals;

Notification::Notification(QObject *parent)
    : QObject(parent)
{
    // devices
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        addDevice(device);
    }

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &Notification::deviceAdded);

    // connections
    for (const NetworkManager::ActiveConnection::Ptr &ac : NetworkManager::activeConnections()) {
        addActiveConnection(ac);
    }

    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activeConnectionAdded,
            this,
            QOverload<const QString &>::of(&Notification::addActiveConnection));

    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(onPrepareForSleep(bool)));

    // After 10 seconds we can consider it to not have "just launched" anymore
    QTimer::singleShot(10s, this, [this]() {
        m_justLaunched = false;
    });
}

void Notification::deviceAdded(const QString &uni)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
    addDevice(device);
}

void Notification::addDevice(const NetworkManager::Device::Ptr &device)
{
    connect(device.data(), &NetworkManager::Device::stateChanged, this, &Notification::stateChanged);
}

void Notification::stateChanged(NetworkManager::Device::State newstate,
                                NetworkManager::Device::State oldstate,
                                NetworkManager::Device::StateChangeReason reason)
{
    Q_UNUSED(oldstate)

    auto device = qobject_cast<NetworkManager::Device *>(sender());
    if (newstate == NetworkManager::Device::Activated && m_notifications.contains(device->uni())) {
        KNotification *notify = m_notifications.value(device->uni());
        notify->close();
        return;
    } else if (newstate != NetworkManager::Device::Failed) {
        return;
    }

    const QString identifier = UiUtils::prettyInterfaceName(device->type(), device->interfaceName());
    QString text;
    switch (reason) {
    case NetworkManager::Device::NoReason:
    case NetworkManager::Device::UnknownReason:
    case NetworkManager::Device::NowManagedReason:
    case NetworkManager::Device::NowUnmanagedReason:
        return;
    case NetworkManager::Device::ConfigFailedReason:
        text = i18nc("@info:status Notification when the device failed due to ConfigFailedReason", "The device could not be configured");
        break;
    case NetworkManager::Device::ConfigUnavailableReason:
        text = i18nc("@info:status Notification when the device failed due to ConfigUnavailableReason", "IP configuration was unavailable");
        break;
    case NetworkManager::Device::ConfigExpiredReason:
        text = i18nc("@info:status Notification when the device failed due to ConfigExpiredReason", "IP configuration expired");
        break;
    case NetworkManager::Device::NoSecretsReason:
        text = i18nc("@info:status Notification when the device failed due to NoSecretsReason", "No secrets were provided");
        break;
    case NetworkManager::Device::AuthSupplicantDisconnectReason:
        text = i18nc("@info:status Notification when the device failed due to AuthSupplicantDisconnectReason", "Authorization supplicant disconnected");
        break;
    case NetworkManager::Device::AuthSupplicantConfigFailedReason:
        text = i18nc("@info:status Notification when the device failed due to AuthSupplicantConfigFailedReason",
                     "Authorization supplicant's configuration failed");
        break;
    case NetworkManager::Device::AuthSupplicantFailedReason:
        text = i18nc("@info:status Notification when the device failed due to AuthSupplicantFailedReason", "Authorization supplicant failed");
        break;
    case NetworkManager::Device::AuthSupplicantTimeoutReason:
        text = i18nc("@info:status Notification when the device failed due to AuthSupplicantTimeoutReason", "Authorization supplicant timed out");
        break;
    case NetworkManager::Device::PppStartFailedReason:
        text = i18nc("@info:status Notification when the device failed due to PppStartFailedReason", "PPP failed to start");
        break;
    case NetworkManager::Device::PppDisconnectReason:
        text = i18nc("@info:status Notification when the device failed due to PppDisconnectReason", "PPP disconnected");
        break;
    case NetworkManager::Device::PppFailedReason:
        text = i18nc("@info:status Notification when the device failed due to PppFailedReason", "PPP failed");
        break;
    case NetworkManager::Device::DhcpStartFailedReason:
        text = i18nc("@info:status Notification when the device failed due to DhcpStartFailedReason", "DHCP failed to start");
        break;
    case NetworkManager::Device::DhcpErrorReason:
        text = i18nc("@info:status Notification when the device failed due to DhcpErrorReason", "A DHCP error occurred");
        break;
    case NetworkManager::Device::DhcpFailedReason:
        text = i18nc("@info:status Notification when the device failed due to DhcpFailedReason", "DHCP failed ");
        break;
    case NetworkManager::Device::SharedStartFailedReason:
        text = i18nc("@info:status Notification when the device failed due to SharedStartFailedReason", "The shared service failed to start");
        break;
    case NetworkManager::Device::SharedFailedReason:
        text = i18nc("@info:status Notification when the device failed due to SharedFailedReason", "The shared service failed");
        break;
    case NetworkManager::Device::AutoIpStartFailedReason:
        text = i18nc("@info:status Notification when the device failed due to AutoIpStartFailedReason", "The auto IP service failed to start");
        break;
    case NetworkManager::Device::AutoIpErrorReason:
        text = i18nc("@info:status Notification when the device failed due to AutoIpErrorReason", "The auto IP service reported an error");
        break;
    case NetworkManager::Device::AutoIpFailedReason:
        text = i18nc("@info:status Notification when the device failed due to AutoIpFailedReason", "The auto IP service failed");
        break;
    case NetworkManager::Device::ModemBusyReason:
        text = i18nc("@info:status Notification when the device failed due to ModemBusyReason", "The modem is busy");
        break;
    case NetworkManager::Device::ModemNoDialToneReason:
        text = i18nc("@info:status Notification when the device failed due to ModemNoDialToneReason", "The modem has no dial tone");
        break;
    case NetworkManager::Device::ModemNoCarrierReason:
        text = i18nc("@info:status Notification when the device failed due to ModemNoCarrierReason", "The modem shows no carrier");
        break;
    case NetworkManager::Device::ModemDialTimeoutReason:
        text = i18nc("@info:status Notification when the device failed due to ModemDialTimeoutReason", "The modem dial timed out");
        break;
    case NetworkManager::Device::ModemDialFailedReason:
        text = i18nc("@info:status Notification when the device failed due to ModemDialFailedReason", "The modem dial failed");
        break;
    case NetworkManager::Device::ModemInitFailedReason:
        text = i18nc("@info:status Notification when the device failed due to ModemInitFailedReason", "The modem could not be initialized");
        break;
    case NetworkManager::Device::GsmApnSelectFailedReason:
        text = i18nc("@info:status Notification when the device failed due to GsmApnSelectFailedReason", "The GSM APN could not be selected");
        break;
    case NetworkManager::Device::GsmNotSearchingReason:
        text = i18nc("@info:status Notification when the device failed due to GsmNotSearchingReason", "The GSM modem is not searching");
        break;
    case NetworkManager::Device::GsmRegistrationDeniedReason:
        text = i18nc("@info:status Notification when the device failed due to GsmRegistrationDeniedReason", "GSM network registration was denied");
        break;
    case NetworkManager::Device::GsmRegistrationTimeoutReason:
        text = i18nc("@info:status Notification when the device failed due to GsmRegistrationTimeoutReason", "GSM network registration timed out");
        break;
    case NetworkManager::Device::GsmRegistrationFailedReason:
        text = i18nc("@info:status Notification when the device failed due to GsmRegistrationFailedReason", "GSM registration failed");
        break;
    case NetworkManager::Device::GsmPinCheckFailedReason:
        text = i18nc("@info:status Notification when the device failed due to GsmPinCheckFailedReason", "The GSM PIN check failed");
        break;
    case NetworkManager::Device::FirmwareMissingReason:
        text = i18nc("@info:status Notification when the device failed due to FirmwareMissingReason", "Device firmware is missing");
        break;
    case NetworkManager::Device::DeviceRemovedReason:
        text = i18nc("@info:status Notification when the device failed due to DeviceRemovedReason", "The device was removed");
        break;
    case NetworkManager::Device::SleepingReason:
        text = i18nc("@info:status Notification when the device failed due to SleepingReason", "The networking system is now sleeping");
        break;
    case NetworkManager::Device::ConnectionRemovedReason:
        text = i18nc("@info:status Notification when the device failed due to ConnectionRemovedReason", "The connection was removed");
        break;
    case NetworkManager::Device::UserRequestedReason:
        return;
    case NetworkManager::Device::CarrierReason:
        text = i18nc("@info:status Notification when the device failed due to CarrierReason", "The cable was disconnected");
        break;
    case NetworkManager::Device::ConnectionAssumedReason:
    case NetworkManager::Device::SupplicantAvailableReason:
        return;
    case NetworkManager::Device::ModemNotFoundReason:
        text = i18nc("@info:status Notification when the device failed due to ModemNotFoundReason", "The modem could not be found");
        break;
    case NetworkManager::Device::BluetoothFailedReason:
        text = i18nc("@info:status Notification when the device failed due to BluetoothFailedReason", "The bluetooth connection failed or timed out");
        break;
    case NetworkManager::Device::GsmSimNotInserted:
        text = i18nc("@info:status Notification when the device failed due to GsmSimNotInserted", "GSM Modem's SIM Card not inserted");
        break;
    case NetworkManager::Device::GsmSimPinRequired:
        text = i18nc("@info:status Notification when the device failed due to GsmSimPinRequired", "GSM Modem's SIM Pin required");
        break;
    case NetworkManager::Device::GsmSimPukRequired:
        text = i18nc("@info:status Notification when the device failed due to GsmSimPukRequired", "GSM Modem's SIM Puk required");
        break;
    case NetworkManager::Device::GsmSimWrong:
        text = i18nc("@info:status Notification when the device failed due to GsmSimWrong", "GSM Modem's SIM wrong");
        break;
    case NetworkManager::Device::InfiniBandMode:
        text = i18nc("@info:status Notification when the device failed due to InfiniBandMode", "InfiniBand device does not support connected mode");
        break;
    case NetworkManager::Device::DependencyFailed:
        text = i18nc("@info:status Notification when the device failed due to DependencyFailed", "A dependency of the connection failed");
        break;
    case NetworkManager::Device::Br2684Failed:
        text = i18nc("@info:status Notification when the device failed due to Br2684Failed", "Problem with the RFC 2684 Ethernet over ADSL bridge");
        break;
    case NetworkManager::Device::ModemManagerUnavailable:
        text = i18nc("@info:status Notification when the device failed due to ModemManagerUnavailable", "ModemManager not running");
        break;
    case NetworkManager::Device::SsidNotFound:
        return;
    case NetworkManager::Device::SecondaryConnectionFailed:
        text =
            i18nc("@info:status Notification when the device failed due to SecondaryConnectionFailed", "A secondary connection of the base connection failed");
        break;
    case NetworkManager::Device::DcbFcoeFailed:
        text = i18nc("@info:status Notification when the device failed due to DcbFcoeFailed", "DCB or FCoE setup failed");
        break;
    case NetworkManager::Device::TeamdControlFailed:
        text = i18nc("@info:status Notification when the device failed due to TeamdControlFailed", "teamd control failed");
        break;
    case NetworkManager::Device::ModemFailed:
        text = i18nc("@info:status Notification when the device failed due to ModemFailed", "Modem failed or no longer available");
        break;
    case NetworkManager::Device::ModemAvailable:
        text = i18nc("@info:status Notification when the device failed due to ModemAvailable", "Modem now ready and available");
        break;
    case NetworkManager::Device::SimPinIncorrect:
        text = i18nc("@info:status Notification when the device failed due to SimPinIncorrect", "The SIM PIN was incorrect");
        break;
    case NetworkManager::Device::NewActivation:
        text = i18nc("@info:status Notification when the device failed due to NewActivation", "A new connection activation was enqueued");
        break;
    case NetworkManager::Device::ParentChanged:
        text = i18nc("@info:status Notification when the device failed due to ParentChanged", "The device's parent changed");
        break;
    case NetworkManager::Device::ParentManagedChanged:
        text = i18nc("@info:status Notification when the device failed due to ParentManagedChanged", "The device parent's management changed");
        break;
    case NetworkManager::Device::Reserved:
        return;
    }

    if (m_notifications.contains(device->uni())) {
        KNotification *notify = m_notifications.value(device->uni());
        notify->setText(text.toHtmlEscaped());
        notify->sendEvent();
    } else {
        auto notify = new KNotification(QStringLiteral("DeviceFailed"), KNotification::CloseOnTimeout);
        connect(notify, &KNotification::closed, this, &Notification::notificationClosed);
        notify->setProperty("uni", device->uni());
        notify->setComponentName(QStringLiteral("networkmanagement"));
        notify->setIconName(QStringLiteral("dialog-warning"));
        notify->setTitle(identifier);
        notify->setText(text.toHtmlEscaped());
        m_notifications[device->uni()] = notify;
        notify->sendEvent();
    }
}

void Notification::addActiveConnection(const QString &path)
{
    NetworkManager::ActiveConnection::Ptr ac = NetworkManager::findActiveConnection(path);
    if (ac && ac->isValid()) {
        addActiveConnection(ac);
    }
}

void Notification::addActiveConnection(const NetworkManager::ActiveConnection::Ptr &ac)
{
    if (ac->vpn()) {
        NetworkManager::VpnConnection::Ptr vpnConnection = ac.objectCast<NetworkManager::VpnConnection>();
        connect(vpnConnection.data(), &NetworkManager::VpnConnection::stateChanged, this, &Notification::onVpnConnectionStateChanged);
    } else if (ac->type() != NetworkManager::ConnectionSettings::Bond //
               && ac->type() != NetworkManager::ConnectionSettings::Bridge //
               && ac->type() != NetworkManager::ConnectionSettings::Generic //
               && ac->type() != NetworkManager::ConnectionSettings::Infiniband //
               && ac->type() != NetworkManager::ConnectionSettings::Team //
               && ac->type() != NetworkManager::ConnectionSettings::Vlan //
               && ac->type() != NetworkManager::ConnectionSettings::Tun) {
        connect(ac.data(), &NetworkManager::ActiveConnection::stateChanged, this, &Notification::onActiveConnectionStateChanged);
    }
}

void Notification::onActiveConnectionStateChanged(NetworkManager::ActiveConnection::State state)
{
    auto ac = qobject_cast<NetworkManager::ActiveConnection *>(sender());

    QString eventId, text, iconName;
    const QString acName = ac->id();
    const QString connectionId = ac->id();

    if (state == NetworkManager::ActiveConnection::Activated) {
        // Don't send notifications about activated connections just because the
        // daemon was launched as these were not explicitly user-initiated actions,
        // and notifications for automatic actions and background processes are
        // annoying and unnecessary
        if (m_justLaunched) {
            qCDebug(PLASMA_NM_KDED_LOG) << "Not emitting connection activated notification as the daemon was just launched";
            return;
        }

        // Also don't notify for re-made connections after waking up from sleep
        auto foundConnection = std::find_if(m_activeConnectionsBeforeSleep.constBegin(), m_activeConnectionsBeforeSleep.constEnd(), [ac](const QString &uuid) {
            return uuid == ac->uuid();
        });

        if (foundConnection != m_activeConnectionsBeforeSleep.constEnd()) {
            qCDebug(PLASMA_NM_KDED_LOG) << "Not emitting connection activated notification as the connection was active prior to suspend";
            return;
        }

        eventId = QStringLiteral("ConnectionActivated");
        text = i18n("Connection '%1' activated.", acName);

        switch (ac->type()) {
        case NetworkManager::ConnectionSettings::Wireless:
            iconName = QStringLiteral("network-wireless-on");
            break;
        case NetworkManager::ConnectionSettings::Wired:
            iconName = QStringLiteral("network-wired-activated");
            break;
        case NetworkManager::ConnectionSettings::WireGuard:
            iconName = QStringLiteral("network-vpn");
            break;
        default: // silence warning
            break;
        }
    } else if (state == NetworkManager::ActiveConnection::Deactivated) {
        if (m_preparingForSleep) {
            qCDebug(PLASMA_NM_KDED_LOG) << "Not emitting connection deactivated notification as we're about to suspend";
            return;
        }

        if (m_checkActiveConnectionOnResumeTimer && m_checkActiveConnectionOnResumeTimer->isActive()) {
            qCDebug(PLASMA_NM_KDED_LOG) << "Not emitting connection deactivated notification as we've just woken up from suspend";
            return;
        }

        eventId = QStringLiteral("ConnectionDeactivated");
        text = i18n("Connection '%1' deactivated.", acName);

        switch (ac->type()) {
        case NetworkManager::ConnectionSettings::Wireless:
            iconName = QStringLiteral("network-wireless-disconnected");
            break;
        case NetworkManager::ConnectionSettings::Wired:
            iconName = QStringLiteral("network-unavailable");
            break;
        default: // silence warning
            break;
        }
    } else {
        qCWarning(PLASMA_NM_KDED_LOG) << "Unhandled active connection state change: " << state;
        return;
    }

    KNotification *notify = m_notifications.value(connectionId);

    if (!notify) {
        notify = new KNotification(eventId, KNotification::CloseOnTimeout);
        connect(notify, &KNotification::closed, this, &Notification::notificationClosed);
        notify->setProperty("uni", connectionId);
        notify->setComponentName(QStringLiteral("networkmanagement"));
        m_notifications[connectionId] = notify;
    }

    if (!iconName.isEmpty()) {
        notify->setIconName(iconName);
    } else {
        if (state == NetworkManager::ActiveConnection::Activated) {
            notify->setIconName(QStringLiteral("dialog-information"));
        } else {
            notify->setIconName(QStringLiteral("dialog-warning"));
        }
    }
    notify->setTitle(acName);
    notify->setText(text.toHtmlEscaped());
    notify->sendEvent();
}

void Notification::onVpnConnectionStateChanged(NetworkManager::VpnConnection::State state, NetworkManager::VpnConnection::StateChangeReason reason)
{
    auto vpn = qobject_cast<NetworkManager::VpnConnection *>(sender());

    QString eventId, text;
    const QString vpnName = vpn->connection()->name();
    const QString connectionId = vpn->path();

    if (state == NetworkManager::VpnConnection::Activated) {
        eventId = QStringLiteral("ConnectionActivated");
        text = i18n("VPN connection '%1' activated.", vpnName);
    } else if (state == NetworkManager::VpnConnection::Failed) {
        eventId = QStringLiteral("FailedToActivateConnection");
        text = i18n("VPN connection '%1' failed to activate.", vpnName);
    } else if (state == NetworkManager::VpnConnection::Disconnected) {
        eventId = QStringLiteral("ConnectionDeactivated");
        text = i18n("VPN connection '%1' deactivated.", vpnName);
    } else {
        qCWarning(PLASMA_NM_KDED_LOG) << "Unhandled VPN connection state change: " << state;
        return;
    }

    switch (reason) {
    case NetworkManager::VpnConnection::UserDisconnectedReason:
        text = i18n("VPN connection '%1' deactivated.", vpnName);
        break;
    case NetworkManager::VpnConnection::DeviceDisconnectedReason:
        text = i18n("VPN connection '%1' was deactivated because the device it was using was disconnected.", vpnName);
        break;
    case NetworkManager::VpnConnection::ServiceStoppedReason:
        text = i18n("The service providing the VPN connection '%1' was stopped.", vpnName);
        break;
    case NetworkManager::VpnConnection::IpConfigInvalidReason:
        text = i18n("The IP config of the VPN connection '%1', was invalid.", vpnName);
        break;
    case NetworkManager::VpnConnection::ConnectTimeoutReason:
        text = i18n("The connection attempt to the VPN service timed out.");
        break;
    case NetworkManager::VpnConnection::ServiceStartTimeoutReason:
        text = i18n("A timeout occurred while starting the service providing the VPN connection '%1'.", vpnName);
        break;
    case NetworkManager::VpnConnection::ServiceStartFailedReason:
        text = i18n("Starting the service providing the VPN connection '%1' failed.", vpnName);
        break;
    case NetworkManager::VpnConnection::NoSecretsReason:
        text = i18n("Necessary secrets for the VPN connection '%1' were not provided.", vpnName);
        break;
    case NetworkManager::VpnConnection::LoginFailedReason:
        text = i18n("Authentication to the VPN server failed.");
        break;
    case NetworkManager::VpnConnection::ConnectionRemovedReason:
        text = i18n("The connection was deleted.");
        break;
    default:
    case NetworkManager::VpnConnection::UnknownReason:
    case NetworkManager::VpnConnection::NoneReason:
        break;
    }

    auto notify = new KNotification(eventId, KNotification::CloseOnTimeout);
    connect(notify, &KNotification::closed, this, &Notification::notificationClosed);
    notify->setProperty("uni", connectionId);
    notify->setComponentName(QStringLiteral("networkmanagement"));
    if (state == NetworkManager::VpnConnection::Activated) {
        notify->setIconName(QStringLiteral("dialog-information"));
    } else if (state == NetworkManager::VpnConnection::Disconnected && reason == NetworkManager::VpnConnection::UserDisconnectedReason) {
        // Don't show warning icon if the user explicitly disconnected
        notify->setIconName(QStringLiteral("dialog-information"));
    } else {
        notify->setIconName(QStringLiteral("dialog-warning"));
    }
    notify->setTitle(vpnName);
    notify->setText(text.toHtmlEscaped());
    m_notifications[connectionId] = notify;
    notify->sendEvent();
}

void Notification::notificationClosed()
{
    auto notify = qobject_cast<KNotification *>(sender());
    m_notifications.remove(notify->property("uni").toString());
}

void Notification::onPrepareForSleep(bool sleep)
{
    m_preparingForSleep = sleep;

    if (m_checkActiveConnectionOnResumeTimer) {
        m_checkActiveConnectionOnResumeTimer->stop();
    }

    if (sleep) {
        // store all active notifications so we don't show a "is connected" notification
        // on resume if we were connected previously
        m_activeConnectionsBeforeSleep.clear();
        const auto &connections = NetworkManager::activeConnections();
        for (const auto &connection : connections) {
            if (!connection->vpn() && connection->state() == NetworkManager::ActiveConnection::State::Activated) {
                m_activeConnectionsBeforeSleep << connection->uuid();
            }
        }
    } else {
        if (!m_checkActiveConnectionOnResumeTimer) {
            m_checkActiveConnectionOnResumeTimer = new QTimer(this);
            m_checkActiveConnectionOnResumeTimer->setInterval(10s);
            m_checkActiveConnectionOnResumeTimer->setSingleShot(true);
            connect(m_checkActiveConnectionOnResumeTimer, &QTimer::timeout, this, &Notification::onCheckActiveConnectionOnResume);
        }

        m_checkActiveConnectionOnResumeTimer->start();
    }
}

void Notification::onCheckActiveConnectionOnResume()
{
    if (m_activeConnectionsBeforeSleep.isEmpty()) {
        // if we weren't connected before, don't bother telling us now :)
        return;
    }

    m_activeConnectionsBeforeSleep.clear();

    const auto ac = NetworkManager::activeConnections();
    if (std::any_of(ac.constBegin(), ac.constEnd(), [](const auto &connection) {
            return connection->state() == NetworkManager::ActiveConnection::State::Activated
                || connection->state() == NetworkManager::ActiveConnection::State::Activating;
        })) {
        // we have an active or activating connection, don't tell the user we're no longer connected
        return;
    }

    auto notify = new KNotification(QStringLiteral("NoLongerConnected"), KNotification::CloseOnTimeout);
    connect(notify, &KNotification::closed, this, &Notification::notificationClosed);
    const QString uni = QStringLiteral("offlineNotification");
    notify->setProperty("uni", uni);
    notify->setComponentName(QStringLiteral("networkmanagement"));
    notify->setIconName(QStringLiteral("dialog-warning"));
    notify->setTitle(i18n("No Network Connection"));
    notify->setText(i18n("You are no longer connected to a network."));
    m_notifications[uni] = notify;
    notify->sendEvent();
}

#include "moc_notification.cpp"
