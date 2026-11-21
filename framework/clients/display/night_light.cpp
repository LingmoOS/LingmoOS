// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/display/night_light.h"

#include <cstdint>
#include <string>

using namespace Lingmo::Base;

namespace Lingmo::Display {

static constexpr const char* kService = "org.lingmo.Services";
static constexpr const char* kPath = "/org/lingmo/Services/NightLight";
static constexpr const char* kInterface = "org.lingmo.Services.NightLight";

template<typename T>
void NightLight::update(T& member, T value)
{
    if (member == value)
        return;
    member = value;
    changed.emit();
}

NightLight::NightLight()
    : m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    dbus::Connection* bus = m_service.connection();
    if (!bus)
        return;
    const auto onBool = [this](const char* member, bool NightLight::*field) {
        m_matches.push_back(m_service.onSignal(member, [this, field](const dbus::Message& message) {
            if (const auto value = message.argument<bool>())
                update(this->*field, *value);
        }));
    };
    const auto onInt = [this](const char* member, int NightLight::*field) {
        m_matches.push_back(m_service.onSignal(member, [this, field](const dbus::Message& message) {
            if (const auto value = message.argument<int32_t>())
                update(this->*field, int(*value));
        }));
    };
    onBool("enabledChanged", &NightLight::m_enabled);
    onBool("availableChanged", &NightLight::m_available);
    onInt("scheduleChanged", &NightLight::m_schedule);
    onInt("customStartChanged", &NightLight::m_customStart);
    onInt("customEndChanged", &NightLight::m_customEnd);
    onInt("sunsetChanged", &NightLight::m_sunset);
    onInt("sunriseChanged", &NightLight::m_sunrise);
    m_matches.push_back(m_service.onSignal("temperatureChanged", [this](const dbus::Message& message) {
        if (const auto value = message.argument<double>())
            update(m_temperature, *value);
    }));
    // Reports the owner at once, which starts the first read.
    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        if (!owner.empty())
            load();
    });
}

NightLight::~NightLight() = default;

void NightLight::load()
{
    m_load = m_service.getAllAsync([this](const dbus::Reply& reply) {
        const auto values = reply.value<dbus::VariantMap>();
        if (!values)
            return;
        const auto value = [&values](const char* name) {
            const auto found = values->find(name);
            return found == values->end() ? dbus::Variant() : found->second;
        };
        m_enabled = value("enabled").value<bool>(false);
        m_temperature = value("temperature").value<double>(m_temperature);
        m_schedule = value("schedule").value<int32_t>(0);
        m_customStart = value("customStart").value<int32_t>(m_customStart);
        m_customEnd = value("customEnd").value<int32_t>(m_customEnd);
        m_available = value("available").value<bool>(false);
        m_sunset = value("sunset").value<int32_t>(m_sunset);
        m_sunrise = value("sunrise").value<int32_t>(m_sunrise);
        m_loaded = true;
        changed.emit();
    });
}

void NightLight::setEnabled(bool enabled)
{
    if (enabled == m_enabled)
        return;
    update(m_enabled, enabled);
    m_service.send("setEnabled", enabled);
}

void NightLight::setTemperature(double temperature)
{
    if (temperature == m_temperature)
        return;
    update(m_temperature, temperature);
    m_service.send("setTemperature", temperature);
}

void NightLight::setSchedule(int schedule)
{
    if (schedule == m_schedule)
        return;
    update(m_schedule, schedule);
    m_service.send("setSchedule", int32_t(schedule));
}

void NightLight::setCustomStart(int minutes)
{
    if (minutes == m_customStart)
        return;
    update(m_customStart, minutes);
    m_service.send("setCustomStart", int32_t(minutes));
}

void NightLight::setCustomEnd(int minutes)
{
    if (minutes == m_customEnd)
        return;
    update(m_customEnd, minutes);
    m_service.send("setCustomEnd", int32_t(minutes));
}

void NightLight::setPreview(bool preview)
{
    if (preview == m_preview)
        return;
    update(m_preview, preview);
    m_service.send("setPreview", preview);
}

} // namespace Lingmo::Display
