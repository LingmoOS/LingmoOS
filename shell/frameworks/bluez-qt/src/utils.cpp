/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "utils.h"
#include "bluezqt_export.h"
#include "manager.h"
#include "obexmanager.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QPointer>

namespace BluezQt
{
class GlobalData
{
public:
    explicit GlobalData();

    bool testRun;
    QString orgFreedesktopDBus;
    QString orgFreedesktopDBusProperties;
    QString orgBluez;
    QString orgBluezAdapter1;
    QString orgBluezBattery1;
    QString orgBluezDevice1;
    QString orgBluezGattService1;
    QString orgBluezGattCharateristic1;
    QString orgBluezGattDescriptor1;
    QString orgBluezInput1;
    QString orgBluezGattManager1;
    QString orgBluezLEAdvertisingManager1;
    QString orgBluezMedia1;
    QString orgBluezMediaPlayer1;
    QString orgBluezMediaTransport1;
    QString orgBluezAgentManager1;
    QString orgBluezProfileManager1;
    QString orgBluezObex;
    QString orgBluezObexClient1;
    QString orgBluezObexAgentManager1;
    QString orgBluezObexSession1;
    QString orgBluezObexTransfer1;
    QPointer<Manager> manager;
    QPointer<ObexManager> obexManager;
};

GlobalData::GlobalData()
{
    testRun = false;
    orgFreedesktopDBus = QStringLiteral("org.freedesktop.DBus");
    orgFreedesktopDBusProperties = QStringLiteral("org.freedesktop.DBus.Properties");
    orgBluez = QStringLiteral("org.bluez");
    orgBluezAdapter1 = QStringLiteral("org.bluez.Adapter1");
    orgBluezBattery1 = QStringLiteral("org.bluez.Battery1");
    orgBluezDevice1 = QStringLiteral("org.bluez.Device1");
    orgBluezGattService1 = QStringLiteral("org.bluez.GattService1");
    orgBluezGattCharateristic1 = QStringLiteral("org.bluez.GattCharacteristic1");
    orgBluezGattDescriptor1 = QStringLiteral("org.bluez.GattDescriptor1");
    orgBluezInput1 = QStringLiteral("org.bluez.Input1");
    orgBluezGattManager1 = QStringLiteral("org.bluez.GattManager1");
    orgBluezLEAdvertisingManager1 = QStringLiteral("org.bluez.LEAdvertisingManager1");
    orgBluezMedia1 = QStringLiteral("org.bluez.Media1");
    orgBluezMediaPlayer1 = QStringLiteral("org.bluez.MediaPlayer1");
    orgBluezMediaTransport1 = QStringLiteral("org.bluez.MediaTransport1");
    orgBluezAgentManager1 = QStringLiteral("org.bluez.AgentManager1");
    orgBluezProfileManager1 = QStringLiteral("org.bluez.ProfileManager1");
    orgBluezObex = QStringLiteral("org.bluez.obex");
    orgBluezObexClient1 = QStringLiteral("org.bluez.obex.Client1");
    orgBluezObexAgentManager1 = QStringLiteral("org.bluez.obex.AgentManager1");
    orgBluezObexSession1 = QStringLiteral("org.bluez.obex.Session1");
    orgBluezObexTransfer1 = QStringLiteral("org.bluez.obex.Transfer1");
}

Q_GLOBAL_STATIC(GlobalData, globalData)

// For fakebluez tests
BLUEZQT_EXPORT void bluezqt_initFakeBluezTestRun()
{
    globalData->testRun = true;
    globalData->orgBluez = QStringLiteral("org.kde.bluezqt.fakebluez");
    globalData->orgBluezObex = QStringLiteral("org.kde.bluezqt.fakebluez");
}

QString Strings::orgFreedesktopDBus()
{
    return globalData->orgFreedesktopDBus;
}

QString Strings::orgFreedesktopDBusProperties()
{
    return globalData->orgFreedesktopDBusProperties;
}

QString Strings::orgBluez()
{
    return globalData->orgBluez;
}

QString Strings::orgBluezAdapter1()
{
    return globalData->orgBluezAdapter1;
}

QString Strings::orgBluezBattery1()
{
    return globalData->orgBluezBattery1;
}

QString Strings::orgBluezDevice1()
{
    return globalData->orgBluezDevice1;
}

QString Strings::orgBluezGattService1()
{
    return globalData->orgBluezGattService1;
}

QString Strings::orgBluezGattCharacteristic1()
{
    return globalData->orgBluezGattCharateristic1;
}

QString Strings::orgBluezGattDescriptor1()
{
    return globalData->orgBluezGattDescriptor1;
}

QString Strings::orgBluezInput1()
{
    return globalData->orgBluezInput1;
}

QString Strings::orgBluezGattManager1()
{
    return globalData->orgBluezGattManager1;
}

QString Strings::orgBluezLEAdvertisingManager1()
{
    return globalData->orgBluezLEAdvertisingManager1;
}

QString Strings::orgBluezMedia1()
{
    return globalData->orgBluezMedia1;
}

QString Strings::orgBluezMediaPlayer1()
{
    return globalData->orgBluezMediaPlayer1;
}

QString Strings::orgBluezMediaTransport1()
{
    return globalData->orgBluezMediaTransport1;
}

QString Strings::orgBluezAgentManager1()
{
    return globalData->orgBluezAgentManager1;
}

QString Strings::orgBluezProfileManager1()
{
    return globalData->orgBluezProfileManager1;
}

QString Strings::orgBluezObex()
{
    return globalData->orgBluezObex;
}

QString Strings::orgBluezObexClient1()
{
    return globalData->orgBluezObexClient1;
}

QString Strings::orgBluezObexAgentManager1()
{
    return globalData->orgBluezObexAgentManager1;
}

QString Strings::orgBluezObexSession1()
{
    return globalData->orgBluezObexSession1;
}

QString Strings::orgBluezObexTransfer1()
{
    return globalData->orgBluezObexTransfer1;
}

QDBusConnection DBusConnection::orgBluez()
{
    if (globalData->testRun) {
        return QDBusConnection::sessionBus();
    }
    return QDBusConnection::systemBus();
}

QDBusConnection DBusConnection::orgBluezObex()
{
    return QDBusConnection::sessionBus();
}

Manager *Instance::manager()
{
    return globalData->manager;
}

void Instance::setManager(Manager *manager)
{
    globalData->manager = manager;
}

ObexManager *Instance::obexManager()
{
    return globalData->obexManager;
}

void Instance::setObexManager(ObexManager *obexManager)
{
    globalData->obexManager = obexManager;
}

QStringList stringListToUpper(const QStringList &list)
{
    QStringList converted;
    converted.reserve(list.size());
    for (const QString &str : list) {
        converted.append(str.toUpper());
    }
    return converted;
}

ManData variantToManData(const QVariant &v) {
    // Map to return
    ManData manData;

    if (!v.isValid()) {
        return manData;
    }

    // Map with QVariant values
    QMap<uint16_t,QVariant> vMap;
    const QDBusArgument &dbusArgs = v.value<QDBusArgument>();
    dbusArgs >> vMap;
    // Iterate & convert values to byte array
    QMap<uint16_t,QVariant>::const_iterator i;
    for (i = vMap.constBegin(); i != vMap.constEnd(); ++i) {
        const QVariant &value = i.value();
        const uint16_t &key = i.key();
        manData.insert(key,value.toByteArray());
    }
    return manData;
}

Device::Type classToType(quint32 classNum)
{
    switch ((classNum & 0x1f00) >> 8) {
    case 0x01:
        return Device::Computer;
    case 0x02:
        switch ((classNum & 0xfc) >> 2) {
        case 0x04:
            return Device::Modem;
        default:
            return Device::Phone;
        }
    case 0x03:
        return Device::Network;
    case 0x04:
        switch ((classNum & 0xfc) >> 2) {
        case 0x01:
        case 0x02:
            return Device::Headset;
        case 0x06:
            return Device::Headphones;
        default:
            return Device::AudioVideo;
        }
    case 0x05:
        switch ((classNum & 0xc0) >> 6) {
        case 0x00:
            switch ((classNum & 0x1e) >> 2) {
            case 0x01:
            case 0x02:
                return Device::Joypad;
            }
            break;
        case 0x01:
            return Device::Keyboard;
        case 0x02:
            switch ((classNum & 0x1e) >> 2) {
            case 0x05:
                return Device::Tablet;
            default:
                return Device::Mouse;
            }
        }
        return Device::Peripheral;
    case 0x06:
        if (classNum & 0x80) {
            return Device::Printer;
        } else if (classNum & 0x20) {
            return Device::Camera;
        }
        return Device::Imaging;
    case 0x07:
        return Device::Wearable;
    case 0x08:
        return Device::Toy;
    case 0x09:
        return Device::Health;
    default:
        return Device::Uncategorized;
    }
}

Device::Type appearanceToType(quint16 appearance)
{
    switch ((appearance & 0xffc0) >> 6) {
    case 0x00:
        return Device::Uncategorized;
    case 0x01: // Generic Phone
        return Device::Phone;
    case 0x02: // Generic Computer
        return Device::Computer;
    case 0x05: // Generic Display
        return Device::AudioVideo;
    case 0x0a: // Generic Media Player
        return Device::AudioVideo;
    case 0x0b: // Generic Barcode Scanner
        return Device::Peripheral;
    case 0x0f: // Generic HID
        switch (appearance & 0x3f) {
        case 0x01: // Keyboard
            return Device::Keyboard;
        case 0x02: // Mouse
            return Device::Mouse;
        case 0x03: // Joystick
        case 0x04: // Gamepad
            return Device::Joypad;
        case 0x05: // Digitizer Tablet
            return Device::Tablet;
        case 0x08: // Barcode Scanner
            return Device::Peripheral;
        }
        // fall-through
    default:
        return Device::Uncategorized;
    }
}

} // namespace BluezQt
