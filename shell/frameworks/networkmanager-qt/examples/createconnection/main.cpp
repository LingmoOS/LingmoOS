/*
    SPDX-FileCopyrightText: 2012 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <arpa/inet.h>

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include <QDBusMetaType>
#include <QTextStream>
#include <QUuid>

using namespace NetworkManager;

int main()
{
    qDBusRegisterMetaType<NMVariantMapMap>();
    QTextStream qout(stdout, QIODevice::WriteOnly);
    QTextStream qin(stdin, QIODevice::ReadOnly);

    NetworkManager::ConnectionSettings *settings = new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless);
    const Device::List deviceList = NetworkManager::networkInterfaces();

    WirelessDevice::Ptr wifiDevice;

    // We have to find some wireless device
    for (Device::Ptr dev : deviceList) {
        if (dev->type() == Device::Wifi) {
            wifiDevice = qobject_cast<WirelessDevice *>(dev);
            break;
        }
    }

    if (!wifiDevice) {
        return 1;
    }

    const QStringList accessPointList = wifiDevice->accessPoints();
    QString ssid;
    QString result;
    QString accessPointPath;

    // Check for available accesspoint
    for (const QString &ap : accessPointList) {
        AccessPoint accessPoint(ap);
        // For simplification we use APs only with Wep security or without any security
        /* clang-format off */
        if (accessPoint.wpaFlags().testFlag(AccessPoint::PairWep40)
            || accessPoint.wpaFlags().testFlag(AccessPoint::PairWep104)
            || accessPoint.wpaFlags().testFlag(AccessPoint::GroupWep40)
            || accessPoint.wpaFlags().testFlag(AccessPoint::GroupWep104)
            || !accessPoint.wpaFlags()) { /* clang-format on */
            qout << "Do you want to connect to " << accessPoint.ssid() << "?" << Qt::endl;
            qout << "Yes/No: " << Qt::flush;
            qin >> result;

            if (result.toLower() == "yes" || result == "y") {
                ssid = accessPoint.ssid();
                accessPointPath = ap;
                break;
            }
        }
    }

    // Now we will prepare our new connection, we have to specify ID and create new UUID
    settings->setId(ssid);
    settings->setUuid(QUuid::createUuid().toString().mid(1, QUuid::createUuid().toString().length() - 2));

    // For wireless setting we have to specify SSID
    NetworkManager::WirelessSetting::Ptr wirelessSetting = settings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();
    wirelessSetting->setSsid(ssid.toUtf8());

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = settings->setting(Setting::Ipv4).dynamicCast<Ipv4Setting>();
    ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Automatic);

    // Optional password setting. Can be skipped if you do not need encryption.
    NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity =
        settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    wifiSecurity->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
    wifiSecurity->setPsk("12345678");
    wifiSecurity->setInitialized(true);
    wirelessSetting->setSecurity("802-11-wireless-security");

    // We try to add and activate our new wireless connection
    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> reply = NetworkManager::addAndActivateConnection(settings->toMap(), wifiDevice->uni(), accessPointPath);

    reply.waitForFinished();

    // Check if this connection was added successfuly
    if (reply.isValid()) {
        // Now our connection should be added in NetworkManager and we can print all settings pre-filled from NetworkManager
        NetworkManager::Connection connection(reply.value().path());
        NetworkManager::ConnectionSettings::Ptr newSettings = connection.settings();
        // Print resulting settings
        qDebug() << (*newSettings.data());

        // Continue with adding secrets
        NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting =
            newSettings->setting(Setting::WirelessSecurity).dynamicCast<WirelessSecuritySetting>();
        if (!wirelessSecuritySetting->needSecrets().isEmpty()) {
            qDebug() << "Need secrets: " << wirelessSecuritySetting->needSecrets();
            // TODO: fill missing secrets
        }

    } else {
        qDebug() << "Connection failed: " << reply.error();
    }
}
