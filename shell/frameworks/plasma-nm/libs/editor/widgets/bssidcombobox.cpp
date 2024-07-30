/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bssidcombobox.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessDevice>

#include <KLocalizedString>

bool signalCompare(const NetworkManager::AccessPoint::Ptr &one, const NetworkManager::AccessPoint::Ptr &two)
{
    return one->signalStrength() > two->signalStrength();
}

BssidComboBox::BssidComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);

    connect(this, &BssidComboBox::editTextChanged, this, &BssidComboBox::slotEditTextChanged);
    connect(this, QOverload<int>::of(&BssidComboBox::activated), this, &BssidComboBox::slotCurrentIndexChanged);
}

QString BssidComboBox::bssid() const
{
    QString result;
    if (!m_dirty)
        result = itemData(currentIndex()).toString();
    else
        result = currentText();

    // qCDebug(PLASMA_NM_EDITOR_LOG) << "Result:" << currentIndex() << result;

    return result;
}

bool BssidComboBox::isValid() const
{
    if (bssid().isEmpty()) {
        return true;
    }

    return NetworkManager::macAddressIsValid(bssid());
}

void BssidComboBox::slotEditTextChanged(const QString &)
{
    m_dirty = true;
    Q_EMIT bssidChanged();
}

void BssidComboBox::slotCurrentIndexChanged(int)
{
    m_dirty = false;
    setEditText(bssid());
    Q_EMIT bssidChanged();
}

void BssidComboBox::init(const QString &bssid, const QString &ssid)
{
    m_initialBssid = bssid;

    // qCDebug(PLASMA_NM_EDITOR_LOG) << "Initial ssid:" << m_initialBssid;

    QList<NetworkManager::AccessPoint::Ptr> aps;

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
            NetworkManager::WirelessNetwork::Ptr wifiNetwork = wifiDevice->findNetwork(ssid);

            if (!wifiNetwork) {
                continue;
            }

            for (const NetworkManager::AccessPoint::Ptr &newAp : wifiNetwork->accessPoints()) {
                bool found = false;
                for (const NetworkManager::AccessPoint::Ptr &existingAp : aps) {
                    if (newAp->hardwareAddress() == existingAp->hardwareAddress()) {
                        if (newAp->signalStrength() > existingAp->signalStrength()) {
                            aps.removeOne(existingAp);
                            break;
                        } else {
                            found = true;
                            break;
                        }
                    }
                }

                if (!found) {
                    aps << newAp;
                }
            }
        }
    }

    std::sort(aps.begin(), aps.end(), signalCompare);
    addBssidsToCombo(aps);

    const int index = findData(m_initialBssid);
    if (index == -1) {
        insertItem(0, m_initialBssid, m_initialBssid);
        setCurrentIndex(0);
    } else {
        setCurrentIndex(index);
    }
    setEditText(m_initialBssid);
}

void BssidComboBox::addBssidsToCombo(const QList<NetworkManager::AccessPoint::Ptr> &aps)
{
    clear();

    if (aps.isEmpty()) {
        addItem(i18n("First select the SSID"));
        return;
    }

    for (const NetworkManager::AccessPoint::Ptr &ap : aps) {
        if (!ap) {
            continue;
        }

        const QString text = i18n("%1 (%2%)\nFrequency: %3 Mhz\nChannel: %4",
                                  ap->hardwareAddress(),
                                  ap->signalStrength(),
                                  ap->frequency(),
                                  QString::number(NetworkManager::findChannel(ap->frequency())));
        addItem(text, QVariant::fromValue(ap->hardwareAddress()));
    }
}

#include "moc_bssidcombobox.cpp"
