// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mtpmonitor.h"

#include "base/event_loop.h"
#include "base/strings.h"
#include "base/url.h"

#include <libudev.h>

#include <cstring>
#include <sys/epoll.h>

using namespace Lingmo::base;

// Allowing access on a phone's screen drops it off the bus and brings it back.
static constexpr int kLeaveDelayMs = 5000;

static std::string udevProperty(udev_device *device, const char *name)
{
    const char *value = udev_device_get_property_value(device, name);
    return value ? std::string(value) : std::string();
}

// udev's *_ENC values escape bytes as \xNN.
static std::string decoded(const std::string &bytes)
{
    std::string result;
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (bytes[i] == '\\' && i + 3 < bytes.size() && bytes[i + 1] == 'x') {
            result.push_back(char(strings::toNumber<int>(bytes.substr(i + 2, 2), 16).value_or(0)));
            i += 3;
        } else {
            result.push_back(bytes[i]);
        }
    }
    return std::string(strings::trimmed(result));
}

static std::string nameOf(udev_device *device)
{
    const std::string player = udevProperty(device, "ID_MEDIA_PLAYER_MODEL");
    if (!player.empty())
        return player;

    if (const char *product = udev_device_get_sysattr_value(device, "product"))
        return std::string(strings::trimmed(product));

    const std::string name = decoded(udevProperty(device, "ID_VENDOR_ENC")) + ' '
            + decoded(udevProperty(device, "ID_MODEL_ENC"));
    return std::string(strings::trimmed(name));
}

MtpMonitor::MtpMonitor()
    : m_udev(udev_new())
{
    if (!m_udev)
        return;

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", "usb_device");
    udev_monitor_enable_receiving(m_monitor);

    m_watch = EventLoop::current()->watch(udev_monitor_get_fd(m_monitor), EPOLLIN, [this](uint32_t) {
        receive();
    });

    udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_add_match_property(enumerate, "ID_MTP_DEVICE", "1");
    udev_enumerate_scan_devices(enumerate);

    udev_list_entry *entry = nullptr;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enumerate))
    {
        if (udev_device *device = udev_device_new_from_syspath(m_udev,
                                                               udev_list_entry_get_name(entry))) {
            handle(device, true);
            udev_device_unref(device);
        }
    }
    udev_enumerate_unref(enumerate);
}

MtpMonitor::~MtpMonitor()
{
    if (m_watch)
        EventLoop::current()->unwatch(m_watch);
    if (m_monitor)
        udev_monitor_unref(m_monitor);
    if (m_udev)
        udev_unref(m_udev);
}

void MtpMonitor::receive()
{
    while (udev_device *device = udev_monitor_receive_device(m_monitor)) {
        const char *action = udev_device_get_action(device);
        if (action && std::strcmp(action, "remove") == 0)
            handle(device, false);
        else if (action && (std::strcmp(action, "add") == 0 || std::strcmp(action, "change") == 0))
            handle(device, true);
        udev_device_unref(device);
    }
}

std::vector<MtpMonitor::Device> MtpMonitor::devices() const
{
    std::vector<Device> result;
    for (const auto &[path, device] : m_devices)
        result.push_back(device);
    return result;
}

MtpMonitor::Device MtpMonitor::device(const std::string &id) const
{
    for (const auto &[path, device] : m_devices) {
        if (device.id == id)
            return device;
    }
    return {};
}

void MtpMonitor::handle(udev_device *device, bool present)
{
    const std::string sysPath = udev_device_get_syspath(device);

    if (!present) {
        leave(sysPath);
        return;
    }

    // Its interfaces carry the same properties but no bus and device numbers.
    const char *type = udev_device_get_devtype(device);
    if (!type || std::strcmp(type, "usb_device") != 0
        || udevProperty(device, "ID_MTP_DEVICE") != "1")
        return;

    // The short serial is empty when udev made one up; the full one is always set.
    const std::string serial = udevProperty(device, "ID_SERIAL");
    if (serial.empty())
        return;

    Device entry;
    entry.id = "mtp://" + url::percentEncoded(serial);
    entry.name = nameOf(device);
    entry.bus = strings::toNumber<uint32_t>(udevProperty(device, "BUSNUM")).value_or(0);
    entry.number = uint8_t(strings::toNumber<uint32_t>(udevProperty(device, "DEVNUM")).value_or(0));
    arrive(sysPath, entry);
}

void MtpMonitor::leave(const std::string &sysPath)
{
    const auto found = m_devices.find(sysPath);
    if (found == m_devices.end())
        return;

    const std::string id = found->second.id;
    m_devices.erase(found);

    auto timer = std::make_unique<Timer>();
    timer->setSingleShot(true);
    timer->onTimeout = [this, id] {
        // Erasing destroys this callback, captures included.
        const std::string gone = id;
        m_leaving.erase(gone);
        removed.emit(gone);
    };
    timer->start(std::chrono::milliseconds(kLeaveDelayMs));
    m_leaving[id] = std::move(timer);
}

void MtpMonitor::arrive(const std::string &sysPath, const Device &entry)
{
    // Two devices with one serial cannot be told apart; the first one is kept.
    for (const auto &[path, known] : m_devices) {
        if (known.id == entry.id && path != sysPath)
            return;
    }

    const bool returning = m_leaving.erase(entry.id) > 0;
    const bool known = m_devices.contains(sysPath);
    m_devices[sysPath] = entry;
    if (known)
        return;

    // A device back on the bus starts a new session: one opened before it left answers
    // nothing any more, so it goes, and the entry stays listed.
    if (returning)
        removed.emit(entry.id);
    added.emit(entry.id);
}
