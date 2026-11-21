// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nightlight.h"

#include <algorithm>
#include <cmath>

using namespace std::chrono_literals;
using namespace Lingmo::Base;

// kManualAnimationDuration.
static constexpr std::chrono::milliseconds kManualDuration = 1s;
// Settings sends a held slider's value every 100 ms; a second would trail it.
static constexpr std::chrono::milliseconds kPreviewDuration = 100ms;

static constexpr double kDefaultTemperature = 0.5;

NightLight::NightLight(Settings &settings, Clock clock, const std::string &displayService)
    : ScheduledFeature({"NightLight", "Enabled", "Schedule", "HoldUntil", "CustomStart", "CustomEnd"},
                       settings, std::move(clock))
    , m_display(displayService)
{
    const double temperature = m_settings.doubleValue("NightLight/Temperature", kDefaultTemperature);
    m_temperature = std::isfinite(temperature) ? std::clamp(temperature, 0.0, 1.0)
                                               : kDefaultTemperature;

    m_subscriptions.push_back(m_display.stateChanged.connect([this] {
        updateAvailable();
    }));
    // A compositor that starts, or starts again, shows no tint until told.
    m_subscriptions.push_back(m_display.availabilityChanged.connect([this] {
        sendToCompositor(kManualDuration);
    }));

    start();
}

NightLight::~NightLight() = default;

bool NightLight::publish(dbus::Connection *bus)
{
    if (!bus)
        return false;
    m_bus = bus;
    m_object = std::make_unique<dbus::Object>(bus, "/org/lingmo/Services/NightLight",
                                              "org.lingmo.Services.NightLight");
    dbus::Object &object = *m_object;
    object.addProperty("enabled", [this] { return enabled(); });
    object.addProperty("temperature", [this] { return m_temperature; });
    object.addProperty("schedule", [this] { return int32_t(schedule()); });
    object.addProperty("customStart", [this] { return int32_t(customStart()); });
    object.addProperty("customEnd", [this] { return int32_t(customEnd()); });
    object.addProperty("available", [this] { return m_available; });
    object.addProperty("sunset", [this] { return int32_t(sunset()); });
    object.addProperty("sunrise", [this] { return int32_t(sunrise()); });
    object.addMethod("setEnabled", [this](bool value) { setEnabled(value); }, {{"enabled"}, {}});
    object.addMethod("setTemperature", [this](double value) { setTemperature(value); },
                     {{"temperature"}, {}});
    object.addMethod("setSchedule", [this](int32_t value) { setSchedule(value); }, {{"schedule"}, {}});
    object.addMethod("setCustomStart", [this](int32_t value) { setCustomStart(value); }, {{"minutes"}, {}});
    object.addMethod("setCustomEnd", [this](int32_t value) { setCustomEnd(value); }, {{"minutes"}, {}});
    object.addMethod("setPreview", "b", "", [this](dbus::MethodCall &call) {
        bool preview = false;
        if (!call.read(preview)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
            return;
        }
        setPreview(preview, call.message().sender());
        call.reply();
    }, {{"preview"}, {}});
    object.addSignal("enabledChanged", "b", {"enabled"});
    object.addSignal("temperatureChanged", "d", {"temperature"});
    object.addSignal("scheduleChanged", "i", {"schedule"});
    object.addSignal("customStartChanged", "i", {"minutes"});
    object.addSignal("customEndChanged", "i", {"minutes"});
    object.addSignal("availableChanged", "b", {"available"});
    object.addSignal("sunsetChanged", "i", {"minutes"});
    object.addSignal("sunriseChanged", "i", {"minutes"});

    m_subscriptions.push_back(enabledChanged.connect([this](bool value) {
        m_object->emitSignal("enabledChanged", value);
    }));
    m_subscriptions.push_back(temperatureChanged.connect([this](double value) {
        m_object->emitSignal("temperatureChanged", value);
    }));
    m_subscriptions.push_back(scheduleChanged.connect([this](int value) {
        m_object->emitSignal("scheduleChanged", int32_t(value));
    }));
    m_subscriptions.push_back(customStartChanged.connect([this](int value) {
        m_object->emitSignal("customStartChanged", int32_t(value));
    }));
    m_subscriptions.push_back(customEndChanged.connect([this](int value) {
        m_object->emitSignal("customEndChanged", int32_t(value));
    }));
    m_subscriptions.push_back(availableChanged.connect([this](bool value) {
        m_object->emitSignal("availableChanged", value);
    }));
    m_subscriptions.push_back(sunsetChanged.connect([this](int value) {
        m_object->emitSignal("sunsetChanged", int32_t(value));
    }));
    m_subscriptions.push_back(sunriseChanged.connect([this](int value) {
        m_object->emitSignal("sunriseChanged", int32_t(value));
    }));
    return object.publish();
}

void NightLight::setTemperature(double temperature)
{
    if (!std::isfinite(temperature))
        return;
    temperature = std::clamp(temperature, 0.0, 1.0);
    if (temperature == m_temperature)
        return;
    m_temperature = temperature;
    write("Temperature", temperature);
    if (m_previewing)
        sendToCompositor(kPreviewDuration);
    else if (enabled())
        sendToCompositor(kManualDuration);
    temperatureChanged.emit(temperature);
}

void NightLight::setPreview(bool preview, const std::string &caller)
{
    if (!preview) {
        endPreview();
        return;
    }
    // A caller that crashes mid-drag must not leave the screen tinted.
    if (!caller.empty() && m_bus) {
        m_previewWatch = m_bus->watchName(caller, [this](const std::string &owner) {
            if (owner.empty())
                endPreview();
        });
    }
    if (m_previewing)
        return;
    m_previewing = true;
    if (!enabled())
        sendToCompositor(kManualDuration);
}

void NightLight::showEnabled(std::chrono::milliseconds duration)
{
    sendToCompositor(duration);
}

void NightLight::endPreview()
{
    m_previewWatch = {};
    if (!m_previewing)
        return;
    m_previewing = false;
    if (!enabled())
        sendToCompositor(kManualDuration);
}

void NightLight::sendToCompositor(std::chrono::milliseconds duration)
{
    // Until its state has loaded, availabilityChanged sends it all.
    if (m_display.available())
        m_display.setNightLight(enabled() || m_previewing ? m_temperature : 0.0,
                                uint32_t(duration.count()));
}

void NightLight::updateAvailable()
{
    const bool available = m_display.nightLightAvailable();
    if (available == m_available)
        return;
    m_available = available;
    availableChanged.emit(available);
}
