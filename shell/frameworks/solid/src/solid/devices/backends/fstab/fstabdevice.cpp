/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fstabdevice.h"
#include "fstab_debug.h"
#include "fstabhandling.h"
#include "fstabnetworkshare.h"
#include "fstabservice.h"
#include <QCoreApplication>
#include <QDir>
#include <QUrl>

using namespace Solid::Backends::Fstab;

FstabDevice::FstabDevice(QString uid)
    : Solid::Ifaces::Device()
    , m_uid(uid)
{
    m_device = m_uid;
    m_device.remove(parentUdi() + "/");

    const QString &fstype = FstabHandling::fstype(m_device);
    qCDebug(FSTAB_LOG) << "Adding " << m_device << "type:" << fstype;

    if (m_device.startsWith("//")) {
        m_vendor = m_device.mid(2, m_device.indexOf("/", 2) - 2);
        m_product = m_device.mid(m_device.indexOf("/", 2) + 1);
        m_storageType = StorageType::NetworkShare;
    } else if (fstype.startsWith("nfs")) {
        m_vendor = m_device.left(m_device.indexOf(":/"));
        m_product = m_device.mid(m_device.indexOf(":/") + 1);
        m_storageType = StorageType::NetworkShare;
    } else if (fstype.startsWith("fuse.") || fstype == QLatin1String("overlay")) {
        m_vendor = fstype;
        m_product = m_device.mid(m_device.indexOf(fstype) + fstype.length());
        QString home = QDir::homePath();
        if (m_product.startsWith(home)) {
            m_product = "~" + m_product.mid(home.length());
        }
        if ((fstype == QLatin1String("fuse.encfs")) || (fstype == QLatin1String("fuse.cryfs")) || (fstype == QLatin1String("fuse.gocryptfs"))) {
            m_storageType = StorageType::Encrypted;
        }
    }

    const QStringList &gvfsOptions = FstabHandling::options(m_device);
    for (const QString &option : gvfsOptions) {
        if (const auto tag = QLatin1String("x-gvfs-name="); option.startsWith(tag)) {
            const QStringView encoded = QStringView(option).mid(tag.size());
            m_displayName = QUrl::fromPercentEncoding(encoded.toLatin1());
        } else if (const auto tag = QLatin1String("x-gvfs-icon="); option.startsWith(tag)) {
            const QStringView encoded = QStringView(option).mid(tag.size());
            m_iconName = QUrl::fromPercentEncoding(encoded.toLatin1());
        }
    }

    if (m_storageType == StorageType::NetworkShare) {
        m_description = QCoreApplication::translate("", "%1 on %2", "%1 is sharename, %2 is servername").arg(m_product, m_vendor);
    } else {
        m_description = QCoreApplication::translate("", "%1 (%2)", "%1 is mountpoint, %2 is fs type").arg(m_product, m_vendor);
    }

    if (m_displayName.isEmpty()) {
        const QStringList currentMountPoints = FstabHandling::currentMountPoints(m_device);
        if (currentMountPoints.isEmpty()) {
            const QStringList mountPoints = FstabHandling::mountPoints(m_device);
            m_displayName = mountPoints.isEmpty() ? m_description : mountPoints.first();
        } else {
            m_displayName = currentMountPoints.first();
        }
    }

    if (m_iconName.isEmpty()) {
        if (m_storageType == StorageType::NetworkShare) {
            m_iconName = QLatin1String("network-server");
        } else if (m_storageType == StorageType::Encrypted) {
            m_iconName = QLatin1String("folder-decrypted");
        } else {
            const QStringList &mountPoints = FstabHandling::mountPoints(m_device);
            const QString home = QDir::homePath();
            if (mountPoints.contains("/")) {
                m_iconName = QStringLiteral("drive-harddisk-root");
            } else if (mountPoints.contains(home)) {
                m_iconName = QStringLiteral("user-home");
            } else {
                m_iconName = QStringLiteral("folder");
            }
        }
    }
}

FstabDevice::~FstabDevice()
{
}

QString FstabDevice::udi() const
{
    return m_uid;
}

QString FstabDevice::parentUdi() const
{
    return QString::fromLatin1(FSTAB_UDI_PREFIX);
}

QString FstabDevice::vendor() const
{
    return m_vendor;
}

QString FstabDevice::product() const
{
    return m_product;
}

QString FstabDevice::icon() const
{
    return m_iconName;
}

QStringList FstabDevice::emblems() const
{
    QStringList res;
    if (!m_storageAccess) {
        FstabDevice *d = const_cast<FstabDevice *>(this);
        d->m_storageAccess = new FstabStorageAccess(d);
    }
    if (m_storageAccess->isAccessible()) {
        res << "emblem-mounted";
    } else {
        res << "emblem-unmounted";
    }

    return res;
}

QString FstabDevice::displayName() const
{
    return m_displayName;
}

QString FstabDevice::description() const
{
    return m_description;
}

bool FstabDevice::isEncrypted() const
{
    return m_storageType == FstabDevice::StorageType::Encrypted;
}

bool FstabDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &interfaceType) const
{
    if (interfaceType == Solid::DeviceInterface::StorageAccess) {
        return true;
    }
    if ((m_storageType == StorageType::NetworkShare) && (interfaceType == Solid::DeviceInterface::NetworkShare)) {
        return true;
    }
    return false;
}

QObject *FstabDevice::createDeviceInterface(const Solid::DeviceInterface::Type &interfaceType)
{
    if (interfaceType == Solid::DeviceInterface::StorageAccess) {
        if (!m_storageAccess) {
            m_storageAccess = new FstabStorageAccess(this);
        }
        return m_storageAccess;
    } else if ((m_storageType == StorageType::NetworkShare) && (interfaceType == Solid::DeviceInterface::NetworkShare)) {
        return new FstabNetworkShare(this);
    }
    return nullptr;
}

QString FstabDevice::device() const
{
    return m_device;
}

void FstabDevice::onMtabChanged(const QString &device)
{
    if (m_device == device) {
        Q_EMIT mtabChanged(device);
    }
}

#include "moc_fstabdevice.cpp"
