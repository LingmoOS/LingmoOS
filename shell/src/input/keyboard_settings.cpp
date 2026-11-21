// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/keyboard_settings.h"
#include "core/logging.h"

#include <algorithm>
#include <limits>

#include "core/preferences.h"
#include "input/dbus_export.h"
#include "input/types.h"

namespace Lingmo::Input {

static constexpr auto service = "org.lingmo.Keyboard1";
static constexpr auto path = "/org/lingmo/Keyboard1";
static constexpr auto group = "Keyboard/";

static constexpr auto pcConvention = "pc";
static constexpr auto commandConvention = "command";

// What xkbcommon compiles for empty rule names.
static std::string defaultLayout()
{
    return "us";
}

static int storedMilliseconds(const base::Settings &settings, const std::string &key,
                              int fallback, int min, int max)
{
    // A value out of int's range is not a number either.
    constexpr int64_t invalid = std::numeric_limits<int64_t>::min();
    const int64_t value = settings.intValue(key, invalid);
    if (!settings.contains(key))
        return std::clamp(fallback, min, max);
    if (value == invalid || value < std::numeric_limits<int>::min()
        || value > std::numeric_limits<int>::max())
        return fallback;
    return std::clamp(int(value), min, max);
}

KeyboardSettings::KeyboardSettings(const std::string &settingsFile)
    : m_settings(settingsFile.empty() ? preferencesFile("compositor") : settingsFile)
{
    m_repeatEnabled = m_settings.boolValue("Keyboard/repeatEnabled", true);
    m_repeatDelay = storedMilliseconds(m_settings, "Keyboard/repeatDelay", kDefaultKeyRepeatDelayMs,
                                       kMinRepeatDelayMs, kMaxRepeatDelayMs);
    m_repeatInterval = storedMilliseconds(m_settings, "Keyboard/repeatInterval",
                                          kDefaultKeyRepeatIntervalMs, kMinRepeatIntervalMs,
                                          kMaxRepeatIntervalMs);
    m_layout = m_settings.stringValue("Keyboard/layout");
    if (m_settings.stringValue("Keyboard/shortcutConvention") == commandConvention)
        m_shortcutConvention = ShortcutConvention::Command;
}

KeyboardSettings::~KeyboardSettings() = default;

bool KeyboardSettings::registerService()
{
    if (m_object)
        return true;
    auto exported = DBusExport::create(service, path, service);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export keyboard settings: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addProperty("repeatEnabled", [this] { return m_repeatEnabled; });
    object.addProperty("repeatDelay", [this] { return int32_t(m_repeatDelay); });
    object.addProperty("repeatInterval", [this] { return int32_t(m_repeatInterval); });
    object.addProperty("layout", [this] { return layout(); });
    object.addProperty("shortcutConvention", [this] { return shortcutConventionName(); });
    object.addMethod("setRepeatEnabled", [this](bool enabled) { setRepeatEnabled(enabled); },
                     base::dbus::ArgumentNames{{"enabled"}, {}});
    object.addMethod("setRepeatDelay",
                     [this](int32_t milliseconds) { setRepeatDelay(milliseconds); },
                     base::dbus::ArgumentNames{{"milliseconds"}, {}});
    object.addMethod("setRepeatInterval",
                     [this](int32_t milliseconds) { setRepeatInterval(milliseconds); },
                     base::dbus::ArgumentNames{{"milliseconds"}, {}});
    object.addMethod("setLayout", [this](const std::string &name) { return setLayout(name); },
                     base::dbus::ArgumentNames{{"name"}, {"result"}});
    object.addMethod("setShortcutConvention",
                     [this](const std::string &name) { return setShortcutConvention(name); },
                     base::dbus::ArgumentNames{{"name"}, {"result"}});
    for (const char *member : {"repeatEnabledChanged", "repeatDelayChanged",
                               "repeatIntervalChanged", "layoutChanged",
                               "shortcutConventionChanged"})
        object.addSignal(member, "");
    if (!exported->publish("keyboard settings"))
        return false;
    m_object = std::move(exported);
    return true;
}

void KeyboardSettings::notify(const base::Signal<> &signal, const char *member)
{
    signal();
    if (m_object)
        m_object->emitSignal(member);
}

void KeyboardSettings::setLayoutApplier(LayoutApplier applier)
{
    m_applier = std::move(applier);
    if (m_layout.empty() || !m_applier || m_applier(m_layout))
        return;
    LM_WARNING(lingmoInput).printf("Cannot use the saved keyboard layout \"%s\"", m_layout.c_str());
    m_layout.clear();
    notify(layoutChanged, "layoutChanged");
}

std::string KeyboardSettings::layout() const
{
    return m_layout.empty() ? defaultLayout() : m_layout;
}

std::string KeyboardSettings::shortcutConventionName() const
{
    return m_shortcutConvention == ShortcutConvention::Command ? commandConvention : pcConvention;
}

template<typename T>
void KeyboardSettings::save(const std::string &key, const std::optional<T> &value)
{
    if (value)
        m_settings.setValue(group + key, *value);
    else
        m_settings.remove(group + key);
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist keyboard settings to" << m_settings.path();
}

void KeyboardSettings::setRepeatEnabled(bool enabled)
{
    if (m_repeatEnabled == enabled)
        return;
    m_repeatEnabled = enabled;
    save("repeatEnabled", std::optional(enabled));
    notify(repeatEnabledChanged, "repeatEnabledChanged");
}

void KeyboardSettings::setRepeatDelay(int milliseconds)
{
    milliseconds = std::clamp(milliseconds, kMinRepeatDelayMs, kMaxRepeatDelayMs);
    if (m_repeatDelay == milliseconds)
        return;
    m_repeatDelay = milliseconds;
    save("repeatDelay", std::optional(milliseconds));
    notify(repeatDelayChanged, "repeatDelayChanged");
}

void KeyboardSettings::setRepeatInterval(int milliseconds)
{
    milliseconds = std::clamp(milliseconds, kMinRepeatIntervalMs, kMaxRepeatIntervalMs);
    if (m_repeatInterval == milliseconds)
        return;
    m_repeatInterval = milliseconds;
    save("repeatInterval", std::optional(milliseconds));
    notify(repeatIntervalChanged, "repeatIntervalChanged");
}

bool KeyboardSettings::setLayout(const std::string &name)
{
    if (name == m_layout)
        return true;
    if (!m_applier || !m_applier(name))
        return false;
    m_layout = name;
    save("layout", name.empty() ? std::nullopt : std::optional(name));
    notify(layoutChanged, "layoutChanged");
    return true;
}

bool KeyboardSettings::setShortcutConvention(const std::string &name)
{
    ShortcutConvention convention;
    if (name == pcConvention)
        convention = ShortcutConvention::Pc;
    else if (name == commandConvention)
        convention = ShortcutConvention::Command;
    else
        return false;
    if (convention == m_shortcutConvention)
        return true;
    m_shortcutConvention = convention;
    save("shortcutConvention",
         convention == ShortcutConvention::Pc ? std::nullopt : std::optional(name));
    notify(shortcutConventionChanged, "shortcutConventionChanged");
    return true;
}

} // namespace Lingmo::Input
