// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/bluez/devices_model.h"

#include <algorithm>

#include "base/locale.h"
#include "lingmo/bluez/manager.h"

namespace Lingmo::bluez {

DevicesModel::DevicesModel(std::shared_ptr<Manager> manager)
{
    setManager(std::move(manager));
}

void DevicesModel::setManager(std::shared_ptr<Manager> manager)
{
    if (m_manager == manager)
        return;
    m_subscriptions.clear();
    m_manager = std::move(manager);
    if (m_manager)
        m_subscriptions.push_back(m_manager->devicesChanged.connect([this] { rebuild(); }));
    rebuild();
}

void DevicesModel::setFilter(Filter filter)
{
    if (m_filter == filter)
        return;
    m_filter = filter;
    rebuild();
}

void DevicesModel::setNamedOnly(bool namedOnly)
{
    if (m_namedOnly == namedOnly)
        return;
    m_namedOnly = namedOnly;
    rebuild();
}

bool DevicesModel::accepts(const BluetoothObject& device) const
{
    if ((m_filter == PairedDevices && !device.paired()) || (m_filter == UnpairedDevices && device.paired()))
        return false;
    if (!m_namedOnly)
        return true;
    // BlueZ aliases an unnamed device to its address, dashes for colons.
    std::string address = device.address();
    std::replace(address.begin(), address.end(), ':', '-');
    return device.name() != address;
}

void DevicesModel::rebuild()
{
    std::vector<std::shared_ptr<BluetoothObject>> devices;
    if (m_manager) {
        for (const auto& device : m_manager->devices()) {
            if (accepts(*device))
                devices.push_back(device);
        }
    }
    const base::Locale& locale = base::Locale::system();
    std::stable_sort(devices.begin(), devices.end(), [&locale](const auto& left, const auto& right) {
        if (left->connected() != right->connected())
            return left->connected();
        if (left->paired() != right->paired())
            return left->paired();
        const int comparison = locale.compare(left->name(), right->name());
        return comparison ? comparison < 0 : left->path() < right->path();
    });

    // The manager reports every change of a device, so one that moves nothing still updates the rows.
    const bool rowsDiffer = devices != m_devices;
    m_devices = std::move(devices);
    if (rowsDiffer)
        rowsChanged.emit();
    else
        dataChanged.emit();
}

} // namespace Lingmo::bluez
