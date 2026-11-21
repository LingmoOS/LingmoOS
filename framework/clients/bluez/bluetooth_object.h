// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::bluez {

class Manager;

// An object BlueZ exports: an adapter, a device, or one of their parts, with
// the properties of each of its interfaces.
class BluetoothObject
{
public:
    using Interfaces = std::map<std::string, base::dbus::VariantMap>;

    BluetoothObject(std::string path, Manager* manager);

    const std::string& path() const { return m_path; }

    std::string address() const;
    // The alias, else the name, else the address.
    std::string name() const;
    std::string icon() const;
    bool powered() const;
    bool discovering() const;
    bool discoverable() const;
    bool pairable() const;
    bool paired() const;
    bool connected() const;
    bool trusted() const;
    bool blocked() const;
    bool servicesResolved() const;
    // -32768 when not known.
    int rssi() const;
    // -1 when not known.
    int batteryPercentage() const;
    std::vector<std::string> uuids() const;
    std::string adapterPath() const;

    // Waiting for a pairing, connection or other operation to finish.
    bool busy() const { return m_busy; }

    // From the device's Device Information service; empty until
    // readDeviceInformation() finds them on a connected device.
    const std::string& modelNumber() const { return m_modelNumber; }
    const std::string& firmwareRevision() const { return m_firmwareRevision; }

    bool isAdapter() const;
    bool isDevice() const;

    bool hasInterface(const std::string& interface) const { return m_interfaces.contains(interface); }
    bool hasBluezInterface() const;

    uint64_t revision(const std::string& interface, const std::string& property) const;
    // A property of the adapter or device interface.
    base::dbus::Variant value(const std::string& property) const;
    base::dbus::Variant value(const std::string& interface, const std::string& property) const;

    void setName(const std::string& name);
    void setPowered(bool enabled);
    void setDiscoverable(bool enabled);
    void setPairable(bool enabled);
    void setTrusted(bool enabled);
    void setBlocked(bool enabled);

    void connectToDevice();
    void disconnectFromDevice();
    void pair();
    void cancelPairing();
    void forget();
    void connectProfile(const std::string& uuid);
    void disconnectProfile(const std::string& uuid);
    void startDiscovery();
    void stopDiscovery();
    void mediaCommand(const std::string& command);
    void readDeviceInformation();

    // For the manager.
    void update(const std::string& interface, const base::dbus::VariantMap& properties,
                const std::vector<std::string>& invalidated = {});
    void removeInterface(const std::string& interface);
    void setBusy(bool busy);
    void setModelNumber(const std::string& modelNumber);
    void setFirmwareRevision(const std::string& firmwareRevision);
    // Gone from the manager: the object no longer reaches BlueZ.
    void detach() { m_manager = nullptr; }

    base::Signal<> changed;

private:
    void setValue(const std::string& property, const base::dbus::Variant& value);

    std::string m_path;
    Manager* m_manager;
    Interfaces m_interfaces;
    std::map<std::string, std::map<std::string, uint64_t>> m_revisions;
    uint64_t m_revision = 0;
    bool m_busy = false;
    std::string m_modelNumber;
    std::string m_firmwareRevision;
};

} // namespace Lingmo::bluez
