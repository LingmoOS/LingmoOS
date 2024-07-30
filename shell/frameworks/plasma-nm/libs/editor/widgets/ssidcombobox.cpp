/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ssidcombobox.h"
#include "uiutils.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>

#include <KLocalizedString>

bool signalCompare(const NetworkManager::WirelessNetwork::Ptr &one, const NetworkManager::WirelessNetwork::Ptr &two)
{
    return one->signalStrength() > two->signalStrength();
}

SsidComboBox::SsidComboBox(QWidget *parent)
    : KComboBox(parent)
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);

    connect(this, &SsidComboBox::editTextChanged, this, &SsidComboBox::slotEditTextChanged);
    connect(this, QOverload<int>::of(&SsidComboBox::activated), this, &SsidComboBox::slotCurrentIndexChanged);
}

QString SsidComboBox::ssid() const
{
    if (currentIndex() == 0 || currentText() != itemData(currentIndex()).toString()) {
        return currentText();
    } else {
        return itemData(currentIndex()).toString();
    }
}

void SsidComboBox::slotEditTextChanged(const QString &text)
{
    if (!text.contains(QLatin1String("Security:")) && !text.contains(QLatin1String("Frequency:"))) {
        Q_EMIT ssidChanged();
    }
}

void SsidComboBox::slotCurrentIndexChanged(int)
{
    setEditText(itemData(currentIndex()).toString());
}

void SsidComboBox::init(const QString &ssid)
{
    m_initialSsid = ssid;

    // qCDebug(PLASMA_NM_EDITOR_LOG) << "Initial ssid:" << m_initialSsid;

    QList<NetworkManager::WirelessNetwork::Ptr> networks;

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();

            for (const NetworkManager::WirelessNetwork::Ptr &newNetwork : wifiDevice->networks()) {
                bool found = false;
                for (const NetworkManager::WirelessNetwork::Ptr &existingNetwork : networks) {
                    if (newNetwork->ssid() == existingNetwork->ssid()) {
                        if (newNetwork->signalStrength() > existingNetwork->signalStrength()) {
                            networks.removeOne(existingNetwork);
                            break;
                        } else {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    networks << newNetwork;
                }
            }
        }
    }

    std::sort(networks.begin(), networks.end(), signalCompare);
    addSsidsToCombo(networks);

    int index = findData(m_initialSsid);
    if (index == -1) {
        insertItem(0, m_initialSsid, m_initialSsid);
        setCurrentIndex(0);
    } else {
        setCurrentIndex(index);
    }
    setEditText(m_initialSsid);
}

void SsidComboBox::addSsidsToCombo(const QList<NetworkManager::WirelessNetwork::Ptr> &networks)
{
    QList<NetworkManager::WirelessDevice::Ptr> wifiDevices;

    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (dev->type() == NetworkManager::Device::Wifi) {
            wifiDevices << dev.objectCast<NetworkManager::WirelessDevice>();
        }
    }

    bool empty = true;

    for (const NetworkManager::WirelessNetwork::Ptr &network : networks) {
        NetworkManager::AccessPoint::Ptr accessPoint = network->referenceAccessPoint();

        if (!accessPoint) {
            continue;
        }

        for (const NetworkManager::WirelessDevice::Ptr &wifiDev : std::as_const(wifiDevices)) {
            if (wifiDev->findNetwork(network->ssid()) == network) {
                if (!empty) {
                    insertSeparator(count());
                }
                empty = false;

                NetworkManager::WirelessSecurityType security =
                    NetworkManager::findBestWirelessSecurity(wifiDev->wirelessCapabilities(),
                                                             true,
                                                             (wifiDev->mode() == NetworkManager::WirelessDevice::Adhoc),
                                                             accessPoint->capabilities(),
                                                             accessPoint->wpaFlags(),
                                                             accessPoint->rsnFlags());
                if (security != NetworkManager::UnknownSecurity && security != NetworkManager::NoneSecurity) {
                    const QString text = i18n("%1 (%2%)\nSecurity: %3\nFrequency: %4 Mhz",
                                              accessPoint->ssid(),
                                              network->signalStrength(),
                                              UiUtils::labelFromWirelessSecurity(security),
                                              accessPoint->frequency());
                    addItem(QIcon::fromTheme(QStringLiteral("object-locked")), text, accessPoint->ssid());
                } else {
                    const QString text =
                        i18n("%1 (%2%)\nSecurity: Insecure\nFrequency: %3 Mhz", accessPoint->ssid(), network->signalStrength(), accessPoint->frequency());
                    addItem(QIcon::fromTheme(QStringLiteral("object-unlocked")), text, accessPoint->ssid());
                }
            }
        }
    }
}

#include "moc_ssidcombobox.cpp"
