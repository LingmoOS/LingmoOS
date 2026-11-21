// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <vector>

#include "base/signal.h"
#include "lingmo/bluez/bluetooth_object.h"

namespace Lingmo::bluez {

class Manager;

// The manager's devices that pass a filter, connected ones first, then
// paired ones, then by name in the user's language.
class DevicesModel
{
public:
    enum Filter {
        AllDevices,
        PairedDevices,
        UnpairedDevices,
    };

    explicit DevicesModel(std::shared_ptr<Manager> manager = {});

    const std::shared_ptr<Manager>& manager() const { return m_manager; }
    void setManager(std::shared_ptr<Manager> manager);

    Filter filter() const { return m_filter; }
    void setFilter(Filter filter);

    // Leaves out devices whose alias is only their address: mostly anonymous
    // BLE advertisers nobody can pick out.
    bool namedOnly() const { return m_namedOnly; }
    void setNamedOnly(bool namedOnly);

    int count() const { return int(m_devices.size()); }
    const std::vector<std::shared_ptr<BluetoothObject>>& devices() const { return m_devices; }

    // Which devices are listed, or their order, changed.
    base::Signal<> rowsChanged;
    // The same devices in the same order, whose values may have changed.
    base::Signal<> dataChanged;

private:
    bool accepts(const BluetoothObject& device) const;
    void rebuild();

    std::shared_ptr<Manager> m_manager;
    std::vector<std::shared_ptr<BluetoothObject>> m_devices;
    std::vector<base::Subscription> m_subscriptions;
    Filter m_filter = AllDevices;
    bool m_namedOnly = false;
};

} // namespace Lingmo::bluez
