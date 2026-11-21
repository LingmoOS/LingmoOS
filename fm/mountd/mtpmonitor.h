// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "base/timer.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct udev;
struct udev_device;
struct udev_monitor;

// USB devices that speak MTP, as udev marks them.
class MtpMonitor
{
public:
    struct Device
    {
        // mtp://<udev ID_SERIAL>, which stays the same when the device is plugged in again.
        std::string id;
        std::string name;
        uint32_t bus = 0;
        uint8_t number = 0;
    };

    MtpMonitor();
    ~MtpMonitor();

    MtpMonitor(const MtpMonitor &) = delete;
    MtpMonitor &operator=(const MtpMonitor &) = delete;

    std::vector<Device> devices() const;
    Device device(const std::string &id) const;

    // udev's events for a USB device: one that leaves is removed only once it stayed away a while.
    void arrive(const std::string &sysPath, const Device &device);
    void leave(const std::string &sysPath);

    Lingmo::base::Signal<std::string> added;
    Lingmo::base::Signal<std::string> removed;

private:
    void handle(udev_device *device, bool present);
    void receive();

    struct udev *m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;
    int m_watch = 0;
    // By sysfs path, which a removal names.
    std::map<std::string, Device> m_devices;
    std::map<std::string, std::unique_ptr<Lingmo::base::Timer>> m_leaving;
};
