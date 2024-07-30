/*
    SPDX-FileCopyrightText: 2011 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bluetoothmonitor.h"
#include "plasma_nm_kded.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <NetworkManagerQt/BluetoothSetting>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>

BluetoothMonitor::BluetoothMonitor(QObject *parent)
    : QObject(parent)
{
}

BluetoothMonitor::~BluetoothMonitor() = default;

bool BluetoothMonitor::bluetoothConnectionExists(const QString &bdAddr, const QString &service)
{
    if (bdAddr.isEmpty() || service.isEmpty()) {
        return false;
    }

    NetworkManager::BluetoothSetting::ProfileType profile;

    if (service == QLatin1String("dun")) {
        profile = NetworkManager::BluetoothSetting::Dun;
    } else if (service == QLatin1String("nap")) {
        profile = NetworkManager::BluetoothSetting::Panu;
    } else {
        return false;
    }

    for (const NetworkManager::Connection::Ptr &con : NetworkManager::listConnections()) {
        if (con && con->settings() && con->settings()->connectionType() == NetworkManager::ConnectionSettings::Bluetooth) {
            NetworkManager::BluetoothSetting::Ptr btSetting =
                con->settings()->setting(NetworkManager::Setting::Bluetooth).staticCast<NetworkManager::BluetoothSetting>();
            if (btSetting->profileType() == profile && btSetting->bluetoothAddress() == NetworkManager::macAddressFromString(bdAddr)) {
                return true;
            }
        }
    }

    return false;
}

void BluetoothMonitor::addBluetoothConnection(const QString &bdAddr, const QString &service, const QString &connectionName)
{
    qCDebug(PLASMA_NM_KDED_LOG) << "Adding BT connection:" << bdAddr << service;

    if (bdAddr.isEmpty() || service.isEmpty() || connectionName.isEmpty()) {
        return;
    }

    if (service != QLatin1String("dun") && service != QLatin1String("nap")) {
        KMessageBox::error(nullptr, i18n("Only 'dun' and 'nap' services are supported."));
        return;
    }

    qCDebug(PLASMA_NM_KDED_LOG) << "Bdaddr == " << bdAddr;

    if (bluetoothConnectionExists(bdAddr, service)) {
        return;
    }

    if (service == QLatin1String("nap")) {
        NetworkManager::ConnectionSettings connectionSettings(NetworkManager::ConnectionSettings::Bluetooth, NM_BT_CAPABILITY_NAP);
        connectionSettings.setUuid(NetworkManager::ConnectionSettings::createNewUuid());
        connectionSettings.setId(connectionName);
        NetworkManager::BluetoothSetting::Ptr btSetting =
            connectionSettings.setting(NetworkManager::Setting::Bluetooth).staticCast<NetworkManager::BluetoothSetting>();
        btSetting->setBluetoothAddress(NetworkManager::macAddressFromString(bdAddr));
        btSetting->setProfileType(NetworkManager::BluetoothSetting::Panu);
        btSetting->setInitialized(true);

        NetworkManager::addConnection(connectionSettings.toMap());
    } else if (service == QLatin1String("dun")) {
        QPointer<MobileConnectionWizard> mobileConnectionWizard = new MobileConnectionWizard(NetworkManager::ConnectionSettings::Bluetooth);
        mobileConnectionWizard->setAttribute(Qt::WA_DeleteOnClose);
        connect(mobileConnectionWizard.data(), &MobileConnectionWizard::accepted, [bdAddr, connectionName, mobileConnectionWizard]() {
            if (mobileConnectionWizard->getError() == MobileProviders::Success) {
                qCDebug(PLASMA_NM_KDED_LOG) << "Mobile broadband wizard finished:" << mobileConnectionWizard->type() << mobileConnectionWizard->args();
                if (mobileConnectionWizard->args().count() == 2) { // GSM or CDMA
                    qCDebug(PLASMA_NM_KDED_LOG) << "Creating new DUN connection for BT device:" << bdAddr;

                    auto tmp = qdbus_cast<QVariantMap>(mobileConnectionWizard->args().value(1));
                    NetworkManager::ConnectionSettings connectionSettings(NetworkManager::ConnectionSettings::Bluetooth, NM_BT_CAPABILITY_DUN);
                    connectionSettings.setUuid(NetworkManager::ConnectionSettings::createNewUuid());
                    connectionSettings.setId(connectionName);
                    NetworkManager::BluetoothSetting::Ptr btSetting =
                        connectionSettings.setting(NetworkManager::Setting::Bluetooth).staticCast<NetworkManager::BluetoothSetting>();
                    btSetting->setBluetoothAddress(NetworkManager::macAddressFromString(bdAddr));
                    btSetting->setProfileType(NetworkManager::BluetoothSetting::Dun);
                    btSetting->setInitialized(true);

                    if (mobileConnectionWizard->type() == NetworkManager::ConnectionSettings::Gsm) {
                        connectionSettings.setting(NetworkManager::Setting::Gsm)->fromMap(tmp);
                        connectionSettings.setting(NetworkManager::Setting::Gsm)->setInitialized(true);
                    } else if (mobileConnectionWizard->type() == NetworkManager::ConnectionSettings::Cdma) {
                        connectionSettings.setting(NetworkManager::Setting::Cdma)->fromMap(tmp);
                        connectionSettings.setting(NetworkManager::Setting::Cdma)->setInitialized(true);
                    }

                    qCDebug(PLASMA_NM_KDED_LOG) << "Adding DUN connection" << connectionSettings;

                    NetworkManager::addConnection(connectionSettings.toMap());
                }
            }
        });
        mobileConnectionWizard->setModal(true);
        mobileConnectionWizard->show();
    }
}

#include "moc_bluetoothmonitor.cpp"
