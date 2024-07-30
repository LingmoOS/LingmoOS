// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include "soliddevicenotifier.h"

#include <Solid/Block>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

#include "device.h"

#include "kded_debug.h"

void SolidDeviceNotifier::start()
{
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &SolidDeviceNotifier::checkUDI);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &SolidDeviceNotifier::removeUDI);
    loadData();
}

void SolidDeviceNotifier::loadData()
{
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    for (const auto &device : devices) {
        checkSolidDevice(device);
    }
}

void SolidDeviceNotifier::checkUDI(const QString &udi)
{
    checkSolidDevice(Solid::Device(udi));
}

void SolidDeviceNotifier::checkSolidDevice(const Solid::Device &device)
{
    qCDebug(KDED) << "!!!! " << device.udi();

    // This seems fairly awkward on a solid level. The actual device
    // isn't really trivial to identify. It certainly mustn't be a
    // filesystem but beyond that it's entirely unclear.
    // The trouble here is that we'll only want to run smartctl on
    // actual devices, not the partitions on the devices as otherwise
    // we'll have trouble validating the output as we'd not know
    // if it is incomplete because the device wasn't a device or
    // there's no data or smartctl is broken or the auth helper is broken...
    if (!device.is<Solid::StorageVolume>()) {
        qCDebug(KDED) << "   not a volume";
        return; // certainly not an interesting device
    }
    switch (device.as<Solid::StorageVolume>()->usage()) {
    case Solid::StorageVolume::Unused:
        Q_FALLTHROUGH();
    case Solid::StorageVolume::FileSystem:
        Q_FALLTHROUGH();
    case Solid::StorageVolume::Encrypted:
        Q_FALLTHROUGH();
    case Solid::StorageVolume::Other:
        Q_FALLTHROUGH();
    case Solid::StorageVolume::Raid:
        qCDebug(KDED) << "   bad type" << device.as<Solid::StorageVolume>()->usage();
        return;
    case Solid::StorageVolume::PartitionTable:
        break;
    }

    qCDebug(KDED) << "evaluating!";

    Q_EMIT addDevice(new Device(device));
}

#include "moc_soliddevicenotifier.cpp"
