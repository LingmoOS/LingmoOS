// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/bluez/bluetooth_object.h"

#include "lingmo/bluez/manager.h"

using namespace Lingmo::Base;

namespace Lingmo::bluez {

BluetoothObject::BluetoothObject(std::string path, Manager* manager)
    : m_path(std::move(path))
    , m_manager(manager)
{
}

dbus::Variant BluetoothObject::value(const std::string& property) const
{
    return value(isAdapter() ? kAdapterInterface : kDeviceInterface, property);
}

dbus::Variant BluetoothObject::value(const std::string& interface, const std::string& property) const
{
    const auto found = m_interfaces.find(interface);
    if (found == m_interfaces.end())
        return {};
    const auto entry = found->second.find(property);
    return entry == found->second.end() ? dbus::Variant() : entry->second;
}

std::string BluetoothObject::address() const
{
    return value("Address").value<std::string>();
}

std::string BluetoothObject::name() const
{
    std::string name = value("Alias").value<std::string>();
    if (name.empty())
        name = value("Name").value<std::string>();
    return name.empty() ? address() : name;
}

std::string BluetoothObject::icon() const
{
    return value("Icon").value<std::string>();
}

bool BluetoothObject::powered() const
{
    return value("Powered").value<bool>();
}

bool BluetoothObject::discovering() const
{
    return value("Discovering").value<bool>();
}

bool BluetoothObject::discoverable() const
{
    return value("Discoverable").value<bool>();
}

bool BluetoothObject::pairable() const
{
    return value("Pairable").value<bool>();
}

bool BluetoothObject::paired() const
{
    return value("Paired").value<bool>();
}

bool BluetoothObject::connected() const
{
    return value("Connected").value<bool>();
}

bool BluetoothObject::trusted() const
{
    return value("Trusted").value<bool>();
}

bool BluetoothObject::blocked() const
{
    return value("Blocked").value<bool>();
}

bool BluetoothObject::servicesResolved() const
{
    return value("ServicesResolved").value<bool>();
}

int BluetoothObject::rssi() const
{
    const auto signal = value("RSSI").get<int16_t>();
    return signal ? *signal : -32768;
}

int BluetoothObject::batteryPercentage() const
{
    const auto percentage = value("org.bluez.Battery1", "Percentage").get<uint8_t>();
    return percentage ? *percentage : -1;
}

std::vector<std::string> BluetoothObject::uuids() const
{
    return value("UUIDs").value<std::vector<std::string>>();
}

std::string BluetoothObject::adapterPath() const
{
    return value("Adapter").value<dbus::ObjectPath>().path;
}

bool BluetoothObject::isAdapter() const
{
    return m_interfaces.contains(kAdapterInterface);
}

bool BluetoothObject::isDevice() const
{
    return m_interfaces.contains(kDeviceInterface);
}

bool BluetoothObject::hasBluezInterface() const
{
    for (const auto& [interface, properties] : m_interfaces) {
        if (interface.starts_with("org.bluez."))
            return true;
    }
    return false;
}

void BluetoothObject::setValue(const std::string& property, const dbus::Variant& value)
{
    if (m_manager)
        m_manager->setObjectProperty(this, property, value);
}

void BluetoothObject::setName(const std::string& name)
{
    setValue("Alias", name);
}

void BluetoothObject::setPowered(bool enabled)
{
    setValue("Powered", enabled);
}

void BluetoothObject::setDiscoverable(bool enabled)
{
    setValue("Discoverable", enabled);
}

void BluetoothObject::setPairable(bool enabled)
{
    setValue("Pairable", enabled);
}

void BluetoothObject::setTrusted(bool enabled)
{
    setValue("Trusted", enabled);
}

void BluetoothObject::setBlocked(bool enabled)
{
    setValue("Blocked", enabled);
}

void BluetoothObject::connectToDevice()
{
    if (m_manager)
        m_manager->operate(this, "Connect");
}

void BluetoothObject::disconnectFromDevice()
{
    if (m_manager)
        m_manager->operate(this, "Disconnect");
}

void BluetoothObject::pair()
{
    if (m_manager)
        m_manager->operate(this, "Pair");
}

void BluetoothObject::cancelPairing()
{
    if (m_manager)
        m_manager->operate(this, "CancelPairing");
}

void BluetoothObject::forget()
{
    if (m_manager)
        m_manager->operate(this, "RemoveDevice");
}

void BluetoothObject::connectProfile(const std::string& uuid)
{
    if (m_manager)
        m_manager->operate(this, "ConnectProfile", uuid);
}

void BluetoothObject::disconnectProfile(const std::string& uuid)
{
    if (m_manager)
        m_manager->operate(this, "DisconnectProfile", uuid);
}

void BluetoothObject::startDiscovery()
{
    if (m_manager)
        m_manager->discovery(this, true);
}

void BluetoothObject::stopDiscovery()
{
    if (m_manager)
        m_manager->discovery(this, false);
}

void BluetoothObject::mediaCommand(const std::string& command)
{
    if (m_manager)
        m_manager->mediaCommand(this, command);
}

void BluetoothObject::readDeviceInformation()
{
    if (m_manager)
        m_manager->readDeviceInformation(this);
}

void BluetoothObject::update(const std::string& interface, const dbus::VariantMap& properties,
                             const std::vector<std::string>& invalidated)
{
    dbus::VariantMap& values = m_interfaces[interface];
    for (const std::string& key : invalidated) {
        values.erase(key);
        m_revisions[interface][key] = ++m_revision;
    }
    for (const auto& [key, value] : properties) {
        values.insert_or_assign(key, value);
        m_revisions[interface][key] = ++m_revision;
    }
    changed.emit();
}

uint64_t BluetoothObject::revision(const std::string& interface, const std::string& property) const
{
    const auto found = m_revisions.find(interface);
    if (found == m_revisions.end())
        return 0;
    const auto entry = found->second.find(property);
    return entry == found->second.end() ? 0 : entry->second;
}

void BluetoothObject::removeInterface(const std::string& interface)
{
    m_interfaces.erase(interface);
    m_revisions.erase(interface);
    changed.emit();
}

void BluetoothObject::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    changed.emit();
}

void BluetoothObject::setModelNumber(const std::string& modelNumber)
{
    if (m_modelNumber == modelNumber)
        return;
    m_modelNumber = modelNumber;
    changed.emit();
}

void BluetoothObject::setFirmwareRevision(const std::string& firmwareRevision)
{
    if (m_firmwareRevision == firmwareRevision)
        return;
    m_firmwareRevision = firmwareRevision;
    changed.emit();
}

} // namespace Lingmo::bluez
