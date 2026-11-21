// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/touchpad_settings.h"
#include "core/logging.h"

#include <algorithm>
#include <cmath>

#include "core/preferences.h"
#include "input/dbus_export.h"

namespace Lingmo::Input {

static constexpr auto service = "org.lingmo.Touchpad1";
static constexpr auto path = "/org/lingmo/Touchpad1";
static constexpr auto group = "Touchpad/";

TouchpadSettings::TouchpadSettings(const std::string &settingsFile)
    : m_settings(settingsFile.empty() ? preferencesFile("compositor") : settingsFile)
{
    const auto flag = [this](const char *key, bool fallback) {
        return m_settings.boolValue(group + std::string(key), fallback);
    };
    m_enabled = flag("enabled", m_enabled);
    m_disableWithMouse = flag("disableWithMouse", m_disableWithMouse);
    m_disableWhileTyping = flag("disableWhileTyping", m_disableWhileTyping);
    m_tapToClick = flag("tapToClick", m_tapToClick);
    m_tapDragging = flag("tapDragging", m_tapDragging);
    m_naturalScroll = flag("naturalScroll", m_naturalScroll);
    m_twoFingerSecondaryClick = flag("twoFingerSecondaryClick", m_twoFingerSecondaryClick);
    const double speed = m_settings.doubleValue("Touchpad/pointerAcceleration", NAN);
    if (std::isfinite(speed))
        m_pointerAcceleration = std::clamp(speed, -1.0, 1.0);
    const double scroll = m_settings.doubleValue("Touchpad/scrollSpeed", kDefaultScrollSpeed);
    if (std::isfinite(scroll))
        m_scrollSpeed = std::clamp(scroll, kMinimumScrollSpeed, kMaximumScrollSpeed);
}

TouchpadSettings::~TouchpadSettings() = default;

bool TouchpadSettings::registerService()
{
    if (m_object)
        return true;
    auto exported = DBusExport::create(service, path, service);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export touchpad settings: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addProperty("available", [this] { return m_available; });
    object.addProperty("enabled", [this] { return m_enabled; });
    object.addProperty("disableWithMouse", [this] { return m_disableWithMouse; });
    object.addProperty("disableWhileTyping", [this] { return m_disableWhileTyping; });
    object.addProperty("tapToClick", [this] { return m_tapToClick; });
    object.addProperty("tapDragging", [this] { return m_tapDragging; });
    object.addProperty("naturalScroll", [this] { return m_naturalScroll; });
    object.addProperty("pointerAcceleration", [this] { return m_pointerAcceleration; });
    object.addProperty("scrollSpeed", [this] { return m_scrollSpeed; });
    object.addProperty("secondaryClickConfigurable",
                       [this] { return m_secondaryClickConfigurable; });
    object.addProperty("twoFingerSecondaryClick", [this] { return m_twoFingerSecondaryClick; });
    object.addMethod("setEnabled", [this](bool enabled) { setEnabled(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setDisableWithMouse", [this](bool enabled) { setDisableWithMouse(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setDisableWhileTyping",
                     [this](bool enabled) { setDisableWhileTyping(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setTapToClick", [this](bool enabled) { setTapToClick(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setTapDragging", [this](bool enabled) { setTapDragging(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setNaturalScroll", [this](bool enabled) { setNaturalScroll(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setPointerAcceleration",
                     [this](double value) { setPointerAcceleration(value); },
                     base::dbus::ArgumentNames{{"value"}, {}});
    object.addMethod("setScrollSpeed", [this](double value) { setScrollSpeed(value); },
                     base::dbus::ArgumentNames{{"value"}, {}});
    object.addMethod("setTwoFingerSecondaryClick",
                     [this](bool enabled) { setTwoFingerSecondaryClick(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    for (const char *member :
         {"availableChanged", "enabledChanged", "disableWithMouseChanged",
          "disableWhileTypingChanged", "tapToClickChanged", "tapDraggingChanged",
          "naturalScrollChanged", "pointerAccelerationChanged", "scrollSpeedChanged",
          "secondaryClickConfigurableChanged", "twoFingerSecondaryClickChanged",
          "settingsChanged"})
        object.addSignal(member, "");
    if (!exported->publish("touchpad settings"))
        return false;
    m_object = std::move(exported);
    return true;
}

template<typename T>
void TouchpadSettings::save(const std::string &key, T value)
{
    m_settings.setValue(group + key, value);
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist touchpad settings to" << m_settings.path();
    notify(settingsChanged, "settingsChanged");
}

void TouchpadSettings::notify(const base::Signal<> &signal, const char *member)
{
    signal();
    if (m_object)
        m_object->emitSignal(member);
}

void TouchpadSettings::setAvailable(bool available)
{
    if (m_available == available)
        return;
    m_available = available;
    notify(availableChanged, "availableChanged");
}

void TouchpadSettings::setSecondaryClickConfigurable(bool configurable)
{
    if (m_secondaryClickConfigurable == configurable)
        return;
    m_secondaryClickConfigurable = configurable;
    notify(secondaryClickConfigurableChanged, "secondaryClickConfigurableChanged");
}

void TouchpadSettings::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    save("enabled", enabled);
    notify(enabledChanged, "enabledChanged");
}

void TouchpadSettings::setDisableWithMouse(bool enabled)
{
    if (m_disableWithMouse == enabled)
        return;
    m_disableWithMouse = enabled;
    save("disableWithMouse", enabled);
    notify(disableWithMouseChanged, "disableWithMouseChanged");
}

void TouchpadSettings::setDisableWhileTyping(bool enabled)
{
    if (m_disableWhileTyping == enabled)
        return;
    m_disableWhileTyping = enabled;
    save("disableWhileTyping", enabled);
    notify(disableWhileTypingChanged, "disableWhileTypingChanged");
}

void TouchpadSettings::setTapToClick(bool enabled)
{
    if (m_tapToClick == enabled)
        return;
    m_tapToClick = enabled;
    save("tapToClick", enabled);
    notify(tapToClickChanged, "tapToClickChanged");
}

void TouchpadSettings::setTapDragging(bool enabled)
{
    if (m_tapDragging == enabled)
        return;
    m_tapDragging = enabled;
    save("tapDragging", enabled);
    notify(tapDraggingChanged, "tapDraggingChanged");
}

void TouchpadSettings::setNaturalScroll(bool enabled)
{
    if (m_naturalScroll == enabled)
        return;
    m_naturalScroll = enabled;
    save("naturalScroll", enabled);
    notify(naturalScrollChanged, "naturalScrollChanged");
}

void TouchpadSettings::setPointerAcceleration(double value)
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

void TouchpadSettings::setScrollSpeed(double value)
{
    if (!std::isfinite(value))
        return;
    value = std::clamp(value, kMinimumScrollSpeed, kMaximumScrollSpeed);
    if (m_scrollSpeed == value)
        return;
    m_scrollSpeed = value;
    save("scrollSpeed", value);
    notify(scrollSpeedChanged, "scrollSpeedChanged");
}

void TouchpadSettings::setTwoFingerSecondaryClick(bool enabled)
{
    if (m_twoFingerSecondaryClick == enabled)
        return;
    m_twoFingerSecondaryClick = enabled;
    save("twoFingerSecondaryClick", enabled);
    notify(twoFingerSecondaryClickChanged, "twoFingerSecondaryClickChanged");
}
} // namespace Lingmo::Input
