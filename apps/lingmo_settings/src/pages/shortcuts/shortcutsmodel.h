// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/signal.h"
#include "pages/input/compositorservice.h"

namespace Lingmo::base::json {
class Value;
}

namespace settings {

// The keyboard shortcuts page's view of the compositor's
// org.lingmo.Shortcuts1 service. The compositor owns the bindings and the
// defaults; this side owns the order the actions are listed in, their
// categories and their descriptions, the way
// ash/webui/shortcut_customization_ui's layout table does for the ChromeOS
// Shortcuts app.
class ShortcutsModel
{
public:
    // The compositor's modifier bits.
    enum Modifier : uint32_t {
        ShiftDown = 1 << 0,
        ControlDown = 1 << 1,
        AltDown = 1 << 2,
        MetaDown = 1 << 3,
    };

    struct Accelerator
    {
        uint32_t keysym = 0;
        uint32_t modifiers = 0;
        std::string keyName;
        // "default" or "user".
        std::string type;
        // As the user reads them: {"Ctrl", "Shift", "Q"}.
        std::vector<std::string> parts;
        // The parts joined: "Ctrl + Shift + Q".
        std::string text;

        bool operator==(const Accelerator&) const = default;
    };

    struct Action
    {
        std::string name;
        std::string category;
        std::string description;
        std::vector<Accelerator> accelerators;
        std::vector<Accelerator> defaults;

        bool isDefault() const { return accelerators == defaults; }
    };

    // "success" and "restore-success-with-conflicts" mean the change was made.
    struct Result
    {
        std::string result;
        // For the user; empty on success.
        std::string message;
        std::string conflictAction;
        std::string conflictDescription;
    };

    ShortcutsModel();
    ~ShortcutsModel();

    ShortcutsModel(const ShortcutsModel&) = delete;
    ShortcutsModel& operator=(const ShortcutsModel&) = delete;

    bool isAvailable() const { return m_service.isAvailable(); }
    bool hasCustomAccelerators() const { return m_hasCustomAccelerators; }
    // The categories the page shows, in order.
    static std::vector<std::string> categories();
    // Command is the shortcut key: the modifiers go by their Command names.
    bool commandKeys() const { return m_commandKeys; }

    // The actions the compositor can perform, in the page's order.
    const std::vector<Action>& actions() const { return m_actions; }
    const Action* action(const std::string& name) const;
    std::string descriptionFor(const std::string& action) const;

    Result addAccelerator(const std::string& action, uint32_t keycode, uint32_t keysym, uint32_t modifiers);
    Result replaceAccelerator(const std::string& action, uint32_t oldKeysym, uint32_t oldModifiers, uint32_t keycode,
                              uint32_t keysym, uint32_t modifiers);
    Result removeAccelerator(const std::string& action, uint32_t keysym, uint32_t modifiers);
    Result restoreDefault(const std::string& action);
    Result restoreAllDefaults();

    // While recording, the compositor performs no shortcut, so pressing Meta
    // does not open the launcher under the dialog.
    void setRecording(bool recording);

    std::vector<std::string> modifierParts(uint32_t modifiers) const;
    // The parts of a key pressed with the modifiers held; the compositor
    // names the key from its evdev code.
    std::vector<std::string> partsForKey(uint32_t keycode, uint32_t keysym, uint32_t modifiers);
    static bool isModifierKey(uint32_t keysym);
    // The modifier bit a modifier key itself stands for, or 0.
    static uint32_t modifierForKey(uint32_t keysym);
    // The keysym the service is sent for a key, as the Qt version derived it
    // from Qt's key code: letters upper case, keys Qt has no code for 0. The
    // service goes by the evdev code when there is one.
    static uint32_t serviceKeysym(uint32_t keysym);

    // After the actions were read again, or the service came or went.
    Lingmo::base::Signal<> changed;

private:
    void reload();
    Accelerator acceleratorFromJson(const Lingmo::base::json::Value& object) const;
    std::string keyDisplay(const std::string& keyName, const std::string& keyText) const;
    template<typename... Args>
    Result call(const std::string& method, const Args&... args);
    Result resultFromJson(const std::string& text) const;
    std::string messageFor(const std::string& result, const std::string& conflictDescription) const;

    CompositorService m_service;
    Lingmo::base::Subscription m_serviceChanged;
    std::vector<Action> m_actions;
    bool m_hasCustomAccelerators = false;
    bool m_commandKeys = false;
};

} // namespace settings
