// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/mouse_settings.h"
#include "core/logging.h"

#include <algorithm>
#include <cmath>

#include "core/preferences.h"
#include "input/dbus_export.h"

namespace Lingmo::Input {

static constexpr auto service = "org.lingmo.Mouse1";
static constexpr auto path = "/org/lingmo/Mouse1";
static constexpr auto group = "Mouse/";

MouseSettings::MouseSettings(const std::string &settingsFile)
    : m_settings(settingsFile.empty() ? preferencesFile("compositor") : settingsFile)
{
    m_shakeToLocate = m_settings.boolValue("Mouse/shakeToLocate", true);
    m_leftHanded = m_settings.boolValue("Mouse/leftHanded", false);
    m_naturalScroll = m_settings.boolValue("Mouse/naturalScroll", m_naturalScroll);
    // The legacy accelerationProfile preference means flat (acceleration disabled).
    m_acceleration = !m_settings.boolValue("Mouse/accelerationProfile", false);
    const double speed = m_settings.doubleValue("Mouse/pointerAcceleration", NAN);
    if (std::isfinite(speed))
        m_pointerAcceleration = std::clamp(speed, -1.0, 1.0);
}

MouseSettings::~MouseSettings() = default;

bool MouseSettings::registerService()
{
    if (m_object)
        return true;
    auto exported = DBusExport::create(service, path, service);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export mouse settings: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addProperty("shakeToLocate", [this] { return m_shakeToLocate; });
    object.addMethod("setShakeToLocate", [this](bool enabled) { setShakeToLocate(enabled); });
    object.addSignal("shakeToLocateChanged", "");
    object.addProperty("available", [this] { return m_available; });
    object.addProperty("leftHanded", [this] { return m_leftHanded; });
    object.addProperty("naturalScroll", [this] { return m_naturalScroll; });
    object.addProperty("acceleration", [this] { return m_acceleration; });
    object.addProperty("pointerAcceleration", [this] { return m_pointerAcceleration; });
    object.addMethod("setLeftHanded", [this](bool enabled) { setLeftHanded(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setNaturalScroll", [this](bool enabled) { setNaturalScroll(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setAcceleration", [this](bool enabled) { setAcceleration(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setPointerAcceleration",
                     [this](double value) { setPointerAcceleration(value); },
                     base::dbus::ArgumentNames{{"value"}, {}});
    for (const char *member : {"availableChanged", "leftHandedChanged", "naturalScrollChanged",
                               "accelerationChanged", "pointerAccelerationChanged",
                               "settingsChanged"})
        object.addSignal(member, "");
    if (!exported->publish("mouse settings"))
        return false;
    m_object = std::move(exported);
    return true;
}

template<typename T>
void MouseSettings::save(const std::string &key, T value)
{
    m_settings.setValue(group + key, value);
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist mouse settings to" << m_settings.path();
    notify(settingsChanged, "settingsChanged");
}

void MouseSettings::notify(const base::Signal<> &signal, const char *member)
{
    signal();
    if (m_object)
        m_object->emitSignal(member);
}

void MouseSettings::setShakeToLocate(bool enabled)
{
    if (m_shakeToLocate == enabled) return;
    m_shakeToLocate = enabled;
    save("shakeToLocate", enabled);
    notify(shakeToLocateChanged, "shakeToLocateChanged");
}

void MouseSettings::setAvailable(bool available)
{
    if (m_available == available)
        return;
    m_available = available;
    notify(availableChanged, "availableChanged");
}

void MouseSettings::setLeftHanded(bool enabled)
{
    if (m_leftHanded == enabled)
        return;
    m_leftHanded = enabled;
    save("leftHanded", enabled);
    notify(leftHandedChanged, "leftHandedChanged");
}

void MouseSettings::setNaturalScroll(bool enabled)
{
    if (m_naturalScroll == enabled)
        return;
    m_naturalScroll = enabled;
    save("naturalScroll", enabled);
    notify(naturalScrollChanged, "naturalScrollChanged");
}

void MouseSettings::setAcceleration(bool enabled)
{
    if (m_acceleration == enabled)
        return;
    m_acceleration = enabled;
    save("accelerationProfile", !enabled);
    notify(accelerationChanged, "accelerationChanged");
}

void MouseSettings::setPointerAcceleration(double value)
{
    if (!std::isfinite(value))
        return;
    value = std::clamp(value, -1.0, 1.0);
    if (m_pointerAcceleration == value)
        return;
    m_pointerAcceleration = value;
    save("pointerAcceleration", value);
    notify(pointerAccelerationChanged, "pointerAccelerationChanged");
}
} // namespace Lingmo::Input
