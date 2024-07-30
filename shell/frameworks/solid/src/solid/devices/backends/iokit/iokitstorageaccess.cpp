/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitstorageaccess.h"

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

using namespace Solid::Backends::IOKit;

IOKitStorageAccess::IOKitStorageAccess(IOKitDevice *device)
    : DeviceInterface(device)
    , daDict(new DADictionary(device))
{
    connect(device, SIGNAL(propertyChanged(QMap<QString, int>)), this, SLOT(onPropertyChanged(QMap<QString, int>)));
}

IOKitStorageAccess::IOKitStorageAccess(const IOKitDevice *device)
    : DeviceInterface(device)
    , daDict(new DADictionary(device))
{
    connect(device, SIGNAL(propertyChanged(QMap<QString, int>)), this, SLOT(onPropertyChanged(QMap<QString, int>)));
}

IOKitStorageAccess::~IOKitStorageAccess()
{
    delete daDict;
}

bool IOKitStorageAccess::isAccessible() const
{
    filePath();
    const QVariant isMounted = m_device->property(QStringLiteral("isMounted"));
    return isMounted.isValid() && isMounted.toBool();
}

QString IOKitStorageAccess::filePath() const
{
    // mount points can change (but can they between invocations of filePath()?)
    QString mountPoint;
    if (const CFURLRef urlRef = daDict->cfUrLRefForKey(kDADiskDescriptionVolumePathKey)) {
        const CFStringRef mpRef = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
        mountPoint = QString::fromCFString(mpRef);
        CFRelease(mpRef);
        m_device->setProperty("mountPoint", QVariant(mountPoint));
        bool isMounted = !mountPoint.isEmpty();
        const QString isMountedKey = QStringLiteral("isMounted");
        const QVariant wasMounted = m_device->property(isMountedKey);
        if (wasMounted.isValid() && wasMounted.toBool() != isMounted) {
            IOKitStorageAccess(m_device).onPropertyChanged(QMap<QString, int>{{isMountedKey, isMounted}});
        }
        m_device->setProperty("isMounted", QVariant(isMounted));
    }
    return mountPoint;
}

bool IOKitStorageAccess::isIgnored() const
{
    if (m_device->iOKitPropertyExists(QStringLiteral("Open"))) {
        // ignore storage volumes that aren't mounted
        bool isIgnored = m_device->property(QStringLiteral("Open")).toBool() == false;
        m_device->setProperty("isIgnored", QVariant(isIgnored));
        return isIgnored;
    }
    const QVariant isIgnored = m_device->property(QStringLiteral("isIgnored"));
    return isIgnored.isValid() && isIgnored.toBool();
}

bool IOKitStorageAccess::isEncrypted() const
{
    // TODO: Implementation left for IOKit developer
    return false;
}

bool IOKitStorageAccess::setup()
{
    // TODO?
    return false;
}

bool IOKitStorageAccess::teardown()
{
    // TODO?
    return false;
}

void IOKitStorageAccess::onPropertyChanged(const QMap<QString, int> &changes)
{
    for (auto it = changes.cbegin(); it != changes.cend(); ++it) {
        if (it.key() == QLatin1String("isMounted")) {
            Q_EMIT accessibilityChanged(m_device->property(QStringLiteral("isMounted")).toBool(), m_device->udi());
        }
    }
}

#include "moc_iokitstorageaccess.cpp"
