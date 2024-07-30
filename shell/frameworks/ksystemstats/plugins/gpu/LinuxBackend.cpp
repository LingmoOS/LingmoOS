/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "LinuxBackend.h"

#include <KLocalizedString>
#include <QDebug>

#include <libudev.h>

#include "LinuxAmdGpu.h"
#include "LinuxNvidiaGpu.h"

// Vendor ID strings, as used in sysfs
static const char *amdVendor = "0x1002";
static const char *intelVendor = "0x8086";
static const char *nvidiaVendor = "0x10de";

LinuxBackend::LinuxBackend(QObject *parent)
    : GpuBackend(parent)
{
}

void LinuxBackend::start()
{
    if (!m_udev) {
        m_udev = udev_new();
    }

    auto enumerate = udev_enumerate_new(m_udev);

    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "drm_minor");
    udev_enumerate_add_match_subsystem(enumerate, "drm");
    udev_enumerate_scan_devices(enumerate);

    auto devices = udev_enumerate_get_list_entry(enumerate);
    for (auto entry = devices; entry; entry = udev_list_entry_get_next(entry)) {
        auto path = udev_list_entry_get_name(entry);
        auto drmDevice = udev_device_new_from_syspath(m_udev, path);
        auto pciDevice = udev_device_get_parent(drmDevice);

        if (strstr(udev_device_get_sysname(drmDevice), "render") != NULL) {
            udev_device_unref(drmDevice);
            continue;
        }

        auto vendor = QByteArray(udev_device_get_sysattr_value(pciDevice, "vendor"));
        auto drmNumber = std::atoi(udev_device_get_sysnum(drmDevice));

        auto gpuId = QStringLiteral("gpu%1").arg(drmNumber);
        auto gpuName = i18nc("@title %1 is GPU number", "GPU %1", drmNumber + 1);

        GpuDevice *gpu = nullptr;
        if (vendor == amdVendor) {
            gpu = new LinuxAmdGpu{gpuId, gpuName, pciDevice};
        } else if (vendor == nvidiaVendor) {
            gpu = new LinuxNvidiaGpu{gpuId, gpuName, pciDevice};
        } else {
            qDebug() << "Found unsupported GPU:" << path;
            udev_device_unref(drmDevice);
            continue;
        }

        gpu->initialize();
        m_devices.append(gpu);
        Q_EMIT deviceAdded(gpu);

        udev_device_unref(drmDevice);
    }

    udev_enumerate_unref(enumerate);
}

void LinuxBackend::stop()
{
    qDeleteAll(m_devices);
    udev_unref(m_udev);
}

void LinuxBackend::update()
{
    for (auto device : std::as_const(m_devices)) {
        device->update();
    }
}

int LinuxBackend::deviceCount()
{
    return m_devices.count();
}

#include "moc_LinuxBackend.cpp"
