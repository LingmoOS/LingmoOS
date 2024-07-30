/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "imobiledevice.h"

#include <QCoreApplication>
#include <QScopeGuard>

#include "imobile_debug.h"

#include "imobile.h"
#include "imobileportablemediaplayer.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

using namespace Solid::Backends::IMobile;

IMobileDevice::IMobileDevice(const QString &udi)
    : Solid::Ifaces::Device()
    , m_udi(udi)
{
    const QString deviceId = udi.mid(udiPrefix().length() + 1);

    idevice_t device;
    auto ret = idevice_new(&device, deviceId.toUtf8().constData());
    if (ret != IDEVICE_E_SUCCESS) {
        qCWarning(IMOBILE) << "Failed to create device instance for" << deviceId << ret;
        return;
    }

    auto deviceCleanup = qScopeGuard([device] {
        idevice_free(device);
    });

    lockdownd_client_t lockdowndClient = nullptr;
    auto lockdownRet = lockdownd_client_new(device, &lockdowndClient, "kde_solid_imobile");
    if (lockdownRet != LOCKDOWN_E_SUCCESS || !lockdowndClient) {
        qCWarning(IMOBILE) << "Failed to create lockdownd client for" << deviceId;
        return;
    }

    auto lockdowndClientCleanup = qScopeGuard([lockdowndClient] {
        lockdownd_client_free(lockdowndClient);
    });

    char *name = nullptr;
    lockdownRet = lockdownd_get_device_name(lockdowndClient, &name);
    if (lockdownRet != LOCKDOWN_E_SUCCESS) {
        qCWarning(IMOBILE) << "Failed to get device name for" << deviceId << lockdownRet;
    } else if (name) {
        m_name = QString::fromUtf8(name);
        free(name);
    }

    plist_t deviceClassEntry = nullptr;
    lockdownRet = lockdownd_get_value(lockdowndClient, nullptr /*global domain*/, "DeviceClass", &deviceClassEntry);
    if (lockdownRet != LOCKDOWN_E_SUCCESS) {
        qCWarning(IMOBILE) << "Failed to get device class for" << deviceId << lockdownRet;
    } else {
        char *deviceClass = nullptr;
        plist_get_string_val(deviceClassEntry, &deviceClass);
        if (deviceClass) {
            m_deviceClass = QString::fromUtf8(deviceClass);
            free(deviceClass);
        }
    }
}

IMobileDevice::~IMobileDevice()
{
}

QString IMobileDevice::udi() const
{
    return m_udi;
}

QString IMobileDevice::parentUdi() const
{
    return udiPrefix();
}

QString IMobileDevice::vendor() const
{
    return QCoreApplication::translate("imobiledevice", "Apple", "Company name");
}

QString IMobileDevice::product() const
{
    // TODO would be nice to use actual product names, e.g. "iPhone 5S"
    // but accessing device type requires doing a handshake with the device,
    // which will fail if locked or not paired, and also would require us
    // to maintain a giant mapping table
    return m_deviceClass;
}

QString IMobileDevice::icon() const
{
    if (m_deviceClass.contains(QLatin1String("iPod"))) {
        return QStringLiteral("multimedia-player-apple-ipod-touch");
    } else if (m_deviceClass.contains(QLatin1String("iPad"))) {
        return QStringLiteral("computer-apple-ipad");
    } else {
        return QStringLiteral("phone-apple-iphone");
    }
}

QStringList IMobileDevice::emblems() const
{
    return {};
}

QString IMobileDevice::description() const
{
    return m_name;
}

bool IMobileDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
        // TODO would be cool to support GenericInterface for reading
        // arbitrary plist configuration, cf. what ideviceinfo tool does

    case Solid::DeviceInterface::PortableMediaPlayer:
        return true;

    default:
        return false;
    }
}

QObject *IMobileDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }

    switch (type) {
    case Solid::DeviceInterface::PortableMediaPlayer:
        return new PortableMediaPlayer(this);

    default:
        Q_UNREACHABLE();
        return nullptr;
    }
}

#include "moc_imobiledevice.cpp"
