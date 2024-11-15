/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "declarativedevice.h"
#include "declarativeadapter.h"
// #include "declarativebattery.h"
#include "declarativeinput.h"
#include "declarativemediaplayer.h"

#include <QStringList>

DeclarativeDevice::DeclarativeDevice(BluezQt::DevicePtr device, DeclarativeAdapter *adapter)
    : QObject(adapter)
    , m_device(device)
    , m_adapter(adapter)
    // , m_battery(nullptr)
    , m_input(nullptr)
    , m_mediaPlayer(nullptr)
{
    connect(m_device.data(), &BluezQt::Device::nameChanged, this, &DeclarativeDevice::nameChanged);
    connect(m_device.data(), &BluezQt::Device::friendlyNameChanged, this, &DeclarativeDevice::friendlyNameChanged);
    connect(m_device.data(), &BluezQt::Device::remoteNameChanged, this, &DeclarativeDevice::remoteNameChanged);
    connect(m_device.data(), &BluezQt::Device::deviceClassChanged, this, &DeclarativeDevice::deviceClassChanged);
    connect(m_device.data(), &BluezQt::Device::typeChanged, this, &DeclarativeDevice::typeChanged);
    connect(m_device.data(), &BluezQt::Device::appearanceChanged, this, &DeclarativeDevice::appearanceChanged);
    connect(m_device.data(), &BluezQt::Device::iconChanged, this, &DeclarativeDevice::iconChanged);
    connect(m_device.data(), &BluezQt::Device::pairedChanged, this, &DeclarativeDevice::pairedChanged);
    connect(m_device.data(), &BluezQt::Device::trustedChanged, this, &DeclarativeDevice::trustedChanged);
    connect(m_device.data(), &BluezQt::Device::blockedChanged, this, &DeclarativeDevice::blockedChanged);
    connect(m_device.data(), &BluezQt::Device::legacyPairingChanged, this, &DeclarativeDevice::legacyPairingChanged);
    connect(m_device.data(), &BluezQt::Device::rssiChanged, this, &DeclarativeDevice::rssiChanged);
    connect(m_device.data(), &BluezQt::Device::connectedChanged, this, &DeclarativeDevice::connectedChanged);
    connect(m_device.data(), &BluezQt::Device::uuidsChanged, this, &DeclarativeDevice::uuidsChanged);
    connect(m_device.data(), &BluezQt::Device::modaliasChanged, this, &DeclarativeDevice::modaliasChanged);
    connect(m_device.data(), &BluezQt::Device::mediaPlayerChanged, this, &DeclarativeDevice::updateMediaPlayer);
    connect(m_device.data(), &BluezQt::Device::inputChanged, this, &DeclarativeDevice::updateInput);
    // connect(m_device.data(), &BluezQt::Device::batteryChanged, this, &DeclarativeDevice::updateBattery);

    connect(m_device.data(), &BluezQt::Device::deviceRemoved, this, [this]() {
        Q_EMIT deviceRemoved(this);
    });

    connect(m_device.data(), &BluezQt::Device::deviceChanged, this, [this]() {
        Q_EMIT deviceChanged(this);
    });

    updateInput();
    updateMediaPlayer();
}

QString DeclarativeDevice::ubi() const
{
    return m_device->ubi();
}

QString DeclarativeDevice::address() const
{
    return m_device->address();
}

QString DeclarativeDevice::name() const
{
    return m_device->name();
}

void DeclarativeDevice::setName(const QString &name)
{
    m_device->setName(name);
}

QString DeclarativeDevice::friendlyName() const
{
    return m_device->friendlyName();
}

QString DeclarativeDevice::remoteName() const
{
    return m_device->remoteName();
}

quint32 DeclarativeDevice::deviceClass() const
{
    return m_device->deviceClass();
}

BluezQt::Device::Type DeclarativeDevice::type() const
{
    return m_device->type();
}

quint16 DeclarativeDevice::appearance() const
{
    return m_device->appearance();
}

QString DeclarativeDevice::icon() const
{
    return m_device->icon();
}

bool DeclarativeDevice::isPaired() const
{
    return m_device->isPaired();
}

bool DeclarativeDevice::isTrusted() const
{
    return m_device->isTrusted();
}

void DeclarativeDevice::setTrusted(bool trusted)
{
    m_device->setTrusted(trusted);
}

bool DeclarativeDevice::isBlocked() const
{
    return m_device->isBlocked();
}

void DeclarativeDevice::setBlocked(bool blocked)
{
    m_device->setBlocked(blocked);
}

bool DeclarativeDevice::hasLegacyPairing() const
{
    return m_device->hasLegacyPairing();
}

qint16 DeclarativeDevice::rssi() const
{
    return m_device->rssi();
}

bool DeclarativeDevice::isConnected() const
{
    return m_device->isConnected();
}

QStringList DeclarativeDevice::uuids() const
{
    return m_device->uuids();
}

QString DeclarativeDevice::modalias() const
{
    return m_device->modalias();
}

//DeclarativeBattery *DeclarativeDevice::battery() const
//{
//    return m_battery;
//}

DeclarativeInput *DeclarativeDevice::input() const
{
    return m_input;
}

DeclarativeMediaPlayer *DeclarativeDevice::mediaPlayer() const
{
    return m_mediaPlayer;
}

DeclarativeAdapter *DeclarativeDevice::adapter() const
{
    return m_adapter;
}

BluezQt::PendingCall *DeclarativeDevice::connectToDevice()
{
    return m_device->connectToDevice();
}

BluezQt::PendingCall *DeclarativeDevice::disconnectFromDevice()
{
    return m_device->disconnectFromDevice();
}

BluezQt::PendingCall *DeclarativeDevice::connectProfile(const QString &uuid)
{
    return m_device->connectProfile(uuid);
}

BluezQt::PendingCall *DeclarativeDevice::disconnectProfile(const QString &uuid)
{
    return m_device->disconnectProfile(uuid);
}

BluezQt::PendingCall *DeclarativeDevice::pair()
{
    return m_device->pair();
}

BluezQt::PendingCall *DeclarativeDevice::cancelPairing()
{
    return m_device->cancelPairing();
}

void DeclarativeDevice::updateBattery()
{
//    if (m_battery) {
//        m_battery->deleteLater();
//        m_battery = nullptr;
//    }

//    if (m_device->battery()) {
//        m_battery = new DeclarativeBattery(m_device->battery(), this);
//    }

//    Q_EMIT batteryChanged(m_battery);
}

void DeclarativeDevice::updateInput()
{
    if (m_input) {
        m_input->deleteLater();
        m_input = nullptr;
    }

    if (m_device->input()) {
        m_input = new DeclarativeInput(m_device->input(), this);
    }

    Q_EMIT inputChanged(m_input);
}

void DeclarativeDevice::updateMediaPlayer()
{
    if (m_mediaPlayer) {
        m_mediaPlayer->deleteLater();
        m_mediaPlayer = nullptr;
    }

    if (m_device->mediaPlayer()) {
        m_mediaPlayer = new DeclarativeMediaPlayer(m_device->mediaPlayer(), this);
    }

    Q_EMIT mediaPlayerChanged(m_mediaPlayer);
}
