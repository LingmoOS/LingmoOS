// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/upower/device.h"

#include <chrono>
#include <tuple>

using namespace Lingmo::Base::dbus;

namespace Lingmo::upower {

static constexpr const char* kService = "org.freedesktop.UPower";
static constexpr const char* kInterface = "org.freedesktop.UPower.Device";

int64_t integerValue(const Variant& value, int64_t fallback)
{
    if (const auto v = value.get<int32_t>())
        return *v;
    if (const auto v = value.get<uint32_t>())
        return *v;
    if (const auto v = value.get<int64_t>())
        return *v;
    if (const auto v = value.get<uint64_t>())
        return int64_t(*v);
    if (const auto v = value.get<int16_t>())
        return *v;
    if (const auto v = value.get<uint16_t>())
        return *v;
    if (const auto v = value.get<uint8_t>())
        return *v;
    if (const auto v = value.get<double>())
        return int64_t(*v);
    return fallback;
}

Device::Device(std::string path, Connection* bus)
    : m_device(bus, kService, std::move(path), kInterface)
{
    m_propertiesMatch = m_device.onPropertiesChanged(
            [this](const VariantMap& changed, const std::vector<std::string>&) {
                merge(changed);
                propertiesChanged.emit(changed);
            });
    load();
}

Device::~Device() = default;

std::string Device::nativePath() const
{
    const auto it = m_properties.find("NativePath");
    return it == m_properties.end() ? std::string() : it->second.value<std::string>();
}

bool Device::chargeThresholdSupported() const
{
    const auto it = m_properties.find("ChargeThresholdSupported");
    return it != m_properties.end() && it->second.value<bool>();
}

bool Device::chargeThresholdEnabled() const
{
    const auto it = m_properties.find("ChargeThresholdEnabled");
    return it != m_properties.end() && it->second.value<bool>();
}

int Device::chargeStartThreshold() const
{
    const auto it = m_properties.find("ChargeStartThreshold");
    return it == m_properties.end() ? 0 : int(integerValue(it->second));
}

int Device::chargeEndThreshold() const
{
    const auto it = m_properties.find("ChargeEndThreshold");
    return it == m_properties.end() ? 0 : int(integerValue(it->second));
}

void Device::enableChargeThreshold(bool enabled, std::function<void(const Error& error)> done)
{
    Proxy device(m_device.connection(), kService, m_device.path(), kInterface);
    device.setInteractiveAuthorizationAllowed(true);
    // Long enough for an authentication dialog.
    device.setTimeout(std::chrono::seconds(120));
    m_enableCall = device.callAsync(
            "EnableChargeThreshold",
            [done = std::move(done)](const Reply& reply) {
                if (done)
                    done(reply.error());
            },
            enabled);
}

PendingCall Device::history(HistoryType type, uint32_t timespan, uint32_t resolution, HistoryCallback done) const
{
    using Samples = std::vector<std::tuple<uint32_t, double, uint32_t>>;
    return m_device.callAsync(
            "GetHistory",
            [done = std::move(done)](const Reply& reply) {
                const std::optional<Samples> samples = reply.value<Samples>();
                if (!samples) {
                    done(std::nullopt);
                    return;
                }
                std::vector<HistorySample> history;
                history.reserve(samples->size());
                for (const auto& [time, value, state] : *samples)
                    history.push_back({time, value, state});
                done(history);
            },
            std::string(type == HistoryType::Rate ? "rate" : "charge"), timespan, resolution);
}

void Device::load()
{
    m_loadCall = m_device.getAllAsync([this](const Reply& reply) {
        if (const auto properties = reply.value<VariantMap>()) {
            merge(*properties);
            propertiesChanged.emit(*properties);
        }
        if (!m_ready) {
            m_ready = true;
            readyChanged.emit();
        }
    });
}

void Device::merge(const VariantMap& properties)
{
    for (const auto& [name, value] : properties)
        m_properties[name] = value;
}

} // namespace Lingmo::upower
