// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "accelerators/hot_corner_settings.h"
#include "core/logging.h"

#include <algorithm>

#include <limits>

#include "accelerators/accelerator.h"
#include "core/preferences.h"
#include "input/dbus_export.h"

namespace Lingmo::Accelerators {
namespace {

constexpr auto service = "org.lingmo.HotCorners1";
constexpr auto path = "/org/lingmo/HotCorners1";

// The D-Bus names, also the stem of the preference keys.
constexpr std::array<std::pair<HotCorner, const char *>, 4> kCorners = {{
        {HotCorner::TopLeft, "top-left"},
        {HotCorner::TopRight, "top-right"},
        {HotCorner::BottomLeft, "bottom-left"},
        {HotCorner::BottomRight, "bottom-right"},
}};

// What a corner can require: AltGr is a level, not a modifier one holds.
constexpr uint32_t kCornerModifiers = ShiftDown | ControlDown | AltDown | MetaDown;

constexpr std::array<const char *, 4> kKeys = {"TopLeft", "TopRight", "BottomLeft", "BottomRight"};

size_t indexOf(HotCorner corner)
{
    return static_cast<size_t>(corner);
}

} // namespace

HotCornerSettings::HotCornerSettings(const std::string &settingsFile)
    : m_settings(settingsFile.empty() ? preferencesFile("compositor") : settingsFile)
{
    for (size_t i = 0; i < kKeys.size(); ++i) {
        const std::string key = std::string("HotCorners/") + kKeys[i];
        HotCornerBinding &binding = m_bindings[i];
        binding.action = acceleratorActionFromName(m_settings.stringValue(key));
        if (binding.action) {
            // A value that is not an unsigned 32-bit number holds none.
            const int64_t modifiers = m_settings.intValue(key + "Modifiers", 0);
            if (modifiers >= 0 && modifiers <= std::numeric_limits<uint32_t>::max())
                binding.modifiers = uint32_t(modifiers) & kCornerModifiers;
        }
    }
}

HotCornerSettings::~HotCornerSettings() = default;

bool HotCornerSettings::registerService()
{
    if (m_object)
        return true;
    auto exported = input::DBusExport::create(service, path, service);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export hot corner settings: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addProperty("topLeft", [this] { return topLeft(); });
    object.addProperty("topLeftModifiers", [this] { return int32_t(topLeftModifiers()); });
    object.addProperty("topRight", [this] { return topRight(); });
    object.addProperty("topRightModifiers", [this] { return int32_t(topRightModifiers()); });
    object.addProperty("bottomLeft", [this] { return bottomLeft(); });
    object.addProperty("bottomLeftModifiers", [this] { return int32_t(bottomLeftModifiers()); });
    object.addProperty("bottomRight", [this] { return bottomRight(); });
    object.addProperty("bottomRightModifiers",
                       [this] { return int32_t(bottomRightModifiers()); });
    object.addMethod("setCorner",
                     [this](const std::string &corner, const std::string &action,
                            int32_t modifiers) { setCorner(corner, action, modifiers); },
                     base::dbus::ArgumentNames{{"corner", "action", "modifiers"}, {}});
    object.addMethod("heldModifiers", [this] { return int32_t(heldModifiers()); });
    object.addSignal("changed", "");
    if (!exported->publish("hot corner settings"))
        return false;
    m_object = std::move(exported);
    return true;
}

const HotCornerBinding &HotCornerSettings::binding(HotCorner corner) const
{
    return m_bindings[indexOf(corner)];
}

void HotCornerSettings::setCorner(const std::string &corner, const std::string &action,
                                  int modifiers)
{
    const auto it = std::find_if(kCorners.begin(), kCorners.end(), [&corner](const auto &entry) {
        return corner == entry.second;
    });
    if (it == kCorners.end())
        return;
    HotCornerBinding binding;
    binding.action = acceleratorActionFromName(action);
    if (binding.action)
        binding.modifiers = uint32_t(modifiers) & kCornerModifiers;
    const size_t index = indexOf(it->first);
    if (m_bindings[index] == binding)
        return;
    m_bindings[index] = binding;

    const std::string key = std::string("HotCorners/") + kKeys[index];
    if (binding.action) {
        m_settings.setValue(key, acceleratorActionName(*binding.action));
        m_settings.setValue(key + "Modifiers", binding.modifiers);
    } else {
        m_settings.remove(key);
        m_settings.remove(key + "Modifiers");
    }
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist hot corner settings to" << m_settings.path();
    changed();
    if (m_object)
        m_object->emitSignal("changed");
}

int HotCornerSettings::heldModifiers() const
{
    return m_modifierSource ? int(m_modifierSource() & kCornerModifiers) : 0;
}

std::string HotCornerSettings::actionName(HotCorner corner) const
{
    const HotCornerBinding &binding = m_bindings[indexOf(corner)];
    return binding.action ? acceleratorActionName(*binding.action) : std::string();
}

int HotCornerSettings::modifiers(HotCorner corner) const
{
    return int(m_bindings[indexOf(corner)].modifiers);
}

} // namespace Lingmo::Accelerators
