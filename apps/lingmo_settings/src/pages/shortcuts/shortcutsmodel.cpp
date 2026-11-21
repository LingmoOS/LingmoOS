// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/shortcuts/shortcutsmodel.h"

#include <cstdio>
#include <map>

#include <xkbcommon/xkbcommon.h>

#include "base/i18n.h"
#include "base/json.h"


using namespace Lingmo::base;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trc;
using Lingmo::i18n::trNoop;

namespace settings {

namespace {

struct Layout
{
    const char* name;
    const char* category;
    const char* description;
};

} // namespace

// The order here is the order on the page, like kAcceleratorLayouts.
static constexpr const char* kGeneral = trNoop("General");
// TRANSLATORS: the group of keyboard shortcuts that act on windows.
static constexpr const char* kWindows = trNoop("Windows");
static constexpr const char* kDesks = trNoop("Desks");
static constexpr const char* kScreenshots = trNoop("Screenshots and Recording");
static const Layout kLayouts[] = {
        {"toggle-app-list", kGeneral, trNoop("Open the launcher")},
        {"toggle-search", kGeneral, trNoop("Open global search")},
        {"toggle-screen-keyboard", kGeneral, trNoop("Toggle screen keyboard")},
        {"lock-screen", kGeneral, trNoop("Lock the screen")},
        {"toggle-clipboard-history", kGeneral, trNoop("Show the clipboard history")},

        {"cycle-forward-mru", kWindows, trNoop("Switch to the next window")},
        {"cycle-backward-mru", kWindows, trNoop("Switch to the previous window")},
        {"toggle-overview", kWindows, trNoop("Show all windows")},
        {"toggle-show-desktop", kWindows, trNoop("Show the desktop")},
        {"toggle-maximized", kWindows, trNoop("Maximize or restore the window")},
        {"window-minimize", kWindows, trNoop("Minimize the window")},
        {"quit-application", kWindows, trNoop("Quit the application")},
        {"hide-application", kWindows, trNoop("Hide the application")},
        {"hide-other-applications", kWindows, trNoop("Hide other applications")},
        {"toggle-fullscreen", kWindows, trNoop("Toggle full screen")},
        {"window-cycle-snap-left", kWindows, trNoop("Snap the window to the left")},
        {"window-cycle-snap-right", kWindows, trNoop("Snap the window to the right")},
        {"close-window", kWindows, trNoop("Close the window")},
        {"move-active-window-between-displays", kWindows, trNoop("Move the window to the next display")},

        {"desks-activate-desk-left", kDesks, trNoop("Switch to the desk on the left")},
        {"desks-activate-desk-right", kDesks, trNoop("Switch to the desk on the right")},
        {"desks-move-active-item-left", kDesks, trNoop("Move the window to the desk on the left")},
        {"desks-move-active-item-right", kDesks, trNoop("Move the window to the desk on the right")},
        {"desks-new-desk", kDesks, trNoop("Add a desk")},
        {"desks-remove-current-desk", kDesks, trNoop("Remove the desk")},
        {"desks-activate-0", kDesks, trNoop("Switch to desk 1")},
        {"desks-activate-1", kDesks, trNoop("Switch to desk 2")},
        {"desks-activate-2", kDesks, trNoop("Switch to desk 3")},
        {"desks-activate-3", kDesks, trNoop("Switch to desk 4")},
        {"desks-activate-4", kDesks, trNoop("Switch to desk 5")},
        {"desks-activate-5", kDesks, trNoop("Switch to desk 6")},
        {"desks-activate-6", kDesks, trNoop("Switch to desk 7")},
        {"desks-activate-7", kDesks, trNoop("Switch to desk 8")},
        {"desks-toggle-assign-to-all-desks", kDesks, trNoop("Show the window on every desk")},

        {"take-screenshot", kScreenshots, trNoop("Save a picture of the screen")},
        {"take-screenshot-to-clipboard", kScreenshots, trNoop("Copy a picture of the screen")},
        {"take-partial-screenshot", kScreenshots, trNoop("Save a picture of the selected area")},
        {"take-partial-screenshot-to-clipboard", kScreenshots, trNoop("Copy a picture of the selected area")},
        {"start-capture-mode", kScreenshots, trNoop("Screenshot and recording options")},
        {"stop-screen-recording", kScreenshots, trNoop("Stop screen recording")},
};

// The keysyms of the keys the Qt version had a Qt key code for besides the
// printable ones and F1 to F35; it sent 0 for every other key.
static const std::map<uint32_t, uint32_t> kServiceKeysyms = {
        {XKB_KEY_Escape, 0xff1b},      {XKB_KEY_Tab, 0xff09},         {XKB_KEY_ISO_Left_Tab, 0xfe20},
        {XKB_KEY_BackSpace, 0xff08},   {XKB_KEY_Return, 0xff0d},      {XKB_KEY_KP_Enter, 0xff8d},
        {XKB_KEY_Insert, 0xff63},      {XKB_KEY_Delete, 0xffff},      {XKB_KEY_Pause, 0xff13},
        {XKB_KEY_Print, 0xff61},       {XKB_KEY_Home, 0xff50},        {XKB_KEY_End, 0xff57},
        {XKB_KEY_Left, 0xff51},        {XKB_KEY_Up, 0xff52},          {XKB_KEY_Right, 0xff53},
        {XKB_KEY_Down, 0xff54},        {XKB_KEY_Prior, 0xff55},       {XKB_KEY_Next, 0xff56},
        {XKB_KEY_Menu, 0xff67},        {XKB_KEY_Meta_L, 0xffeb},      {XKB_KEY_Meta_R, 0xffeb},
        {XKB_KEY_Super_L, 0xffeb},     {XKB_KEY_Super_R, 0xffec},
        {XKB_KEY_XF86AudioLowerVolume, 0x1008ff11}, {XKB_KEY_XF86AudioMute, 0x1008ff12},
        {XKB_KEY_XF86AudioRaiseVolume, 0x1008ff13}, {XKB_KEY_XF86AudioPlay, 0x1008ff14},
        {XKB_KEY_XF86AudioStop, 0x1008ff15},        {XKB_KEY_XF86AudioPrev, 0x1008ff16},
        {XKB_KEY_XF86AudioNext, 0x1008ff17},        {XKB_KEY_XF86MonBrightnessUp, 0x1008ff02},
        {XKB_KEY_XF86MonBrightnessDown, 0x1008ff03}, {XKB_KEY_XF86Calculator, 0x1008ff1d},
};

ShortcutsModel::ShortcutsModel()
    : m_service("org.lingmo.Shortcuts1")
{
    // The service's acceleratorsChanged, and its coming and going, end up here.
    m_serviceChanged = m_service.changed.connect([this] { reload(); });
    reload();
}

ShortcutsModel::~ShortcutsModel()
{
    setRecording(false);
}

std::vector<std::string> ShortcutsModel::categories()
{
    return {kGeneral, kWindows};
}

void ShortcutsModel::reload()
{
    std::map<std::string, json::Value> published;
    bool hasCustom = false;
    if (m_service.isAvailable()) {
        const dbus::Reply reply = m_service.proxy().call("accelerators");
        if (const std::optional<std::string> text = reply.value<std::string>()) {
            const json::Value document = json::Value::parse(*text).value_or(json::Value());
            for (const json::Value& action : document["actions"].toArray())
                published[action["name"].toString()] = action;
        }
        hasCustom = m_service.proxy().call("hasCustomAccelerators").value<bool>().value_or(false);
        m_commandKeys = m_service.stringValue("shortcutConvention") == "command";
    }

    m_actions.clear();
    for (const Layout& layout : kLayouts) {
        const auto it = published.find(layout.name);
        // An action the compositor does not publish is one it cannot perform.
        if (it == published.end())
            continue;
        Action action;
        action.name = layout.name;
        action.category = layout.category;
        action.description = tr(layout.description);
        for (const json::Value& value : it->second["accelerators"].toArray())
            action.accelerators.push_back(acceleratorFromJson(value));
        for (const json::Value& value : it->second["defaults"].toArray())
            action.defaults.push_back(acceleratorFromJson(value));
        m_actions.push_back(std::move(action));
    }
    m_hasCustomAccelerators = hasCustom;
    changed.emit();
}

ShortcutsModel::Accelerator ShortcutsModel::acceleratorFromJson(const json::Value& object) const
{
    Accelerator accelerator;
    accelerator.keysym = uint32_t(object["keysym"].toInt());
    accelerator.modifiers = uint32_t(object["modifiers"].toInt());
    accelerator.keyName = object["keyName"].toString();
    accelerator.type = object["type"].toString();
    accelerator.parts = modifierParts(accelerator.modifiers);
    accelerator.parts.push_back(keyDisplay(accelerator.keyName, object["keyText"].toString()));
    for (size_t i = 0; i < accelerator.parts.size(); ++i)
        accelerator.text += (i > 0 ? " + " : "") + accelerator.parts[i];
    return accelerator;
}

std::string ShortcutsModel::keyDisplay(const std::string& keyName, const std::string& keyText) const
{
    if (keyName == "Super_L" || keyName == "Super_R")
        return m_commandKeys ? trc("key", "Command") : trc("key", "Meta");
    const std::map<std::string, std::string> names = {
            {"Up", "↑"},
            {"Down", "↓"},
            {"Left", "←"},
            {"Right", "→"},
            {"Escape", trc("key", "Esc")},
            {"Return", trc("key", "Enter")},
            {"KP_Enter", trc("key", "Enter")},
            {"space", trc("key", "Space")},
            {"BackSpace", trc("key", "Backspace")},
            {"Delete", trc("key", "Delete")},
            {"Insert", trc("key", "Insert")},
            {"Home", trc("key", "Home")},
            {"End", trc("key", "End")},
            {"Prior", trc("key", "Page Up")},
            {"Next", trc("key", "Page Down")},
            {"Print", trc("key", "Print Screen")},
            {"Tab", trc("key", "Tab")},
            {"Menu", trc("key", "Menu")},
            {"XF86AudioRaiseVolume", trc("key", "Volume Up")},
            {"XF86AudioLowerVolume", trc("key", "Volume Down")},
            {"XF86AudioMute", trc("key", "Mute")},
            {"XF86MonBrightnessUp", trc("key", "Brightness Up")},
            {"XF86MonBrightnessDown", trc("key", "Brightness Down")},
            {"XF86AudioPlay", trc("key", "Play/Pause")},
            {"XF86AudioPause", trc("key", "Pause")},
            {"XF86AudioNext", trc("key", "Next Track")},
            {"XF86AudioPrev", trc("key", "Previous Track")},
            {"XF86AudioStop", trc("key", "Stop")},
            {"XF86Calculator", trc("key", "Calculator")},
            {"XF86Explorer", trc("key", "File Manager")},
            {"XF86Search", trc("key", "Search")},
            {"XF86HomePage", trc("key", "Home Page")},
            {"XF86Mail", trc("key", "Mail")},
    };
    if (!keyText.empty())
        return keyText;
    const auto it = names.find(keyName);
    if (it != names.end())
        return it->second;
    return keyName;
}

const ShortcutsModel::Action* ShortcutsModel::action(const std::string& name) const
{
    for (const Action& candidate : m_actions) {
        if (candidate.name == name)
            return &candidate;
    }
    return nullptr;
}

std::string ShortcutsModel::descriptionFor(const std::string& action) const
{
    const Action* found = this->action(action);
    return found ? found->description : action;
}

template<typename... Args>
ShortcutsModel::Result ShortcutsModel::call(const std::string& method, const Args&... args)
{
    if (!m_service.isAvailable())
        return {.result = "unavailable", .message = tr("The shortcut service is not available.")};
    const dbus::Reply reply = m_service.proxy().call(method, args...);
    if (reply.isError()) {
        fprintf(stderr, "Shortcut request %s failed: %s\n", method.c_str(), reply.error().message.c_str());
        return {.result = "error", .message = reply.error().message};
    }
    return resultFromJson(reply.value<std::string>().value_or(std::string()));
}

ShortcutsModel::Result ShortcutsModel::resultFromJson(const std::string& text) const
{
    const json::Value object = json::Value::parse(text).value_or(json::Value());
    Result result;
    result.result = object["result"].toString();
    result.conflictAction = object["conflictAction"].toString();
    result.conflictDescription = descriptionFor(result.conflictAction);
    result.message = messageFor(result.result, result.conflictDescription);
    return result;
}

std::string ShortcutsModel::messageFor(const std::string& result, const std::string& conflictDescription) const
{
    if (result == "success")
        return {};
    if (result == "conflict")
        return format(tr("This shortcut is already used by \"{action}\"."), {{"action", conflictDescription}});
    if (result == "conflict-can-override")
        return format(tr("This shortcut is used by \"{action}\". Press it again to reassign it."),
                      {{"action", conflictDescription}});
    if (result == "missing-modifier") {
        return m_commandKeys ? tr("Include Control, Option or Command, or use a function key.")
                             : tr("Include Ctrl, Alt or Meta, or use a function key.");
    }
    if (result == "shift-only-not-allowed") {
        return m_commandKeys ? tr("Shift cannot be the only modifier. Add Control, Option or Command.")
                             : tr("Shift cannot be the only modifier. Add Ctrl, Alt or Meta.");
    }
    if (result == "key-not-allowed" || result == "reserved-key-not-allowed")
        return tr("This key cannot be used in a shortcut.");
    if (result == "maximum-accelerators-reached")
        return tr("This action already has the maximum of five shortcuts.");
    if (result == "non-meta-accelerator-warning") {
        return m_commandKeys ? tr("Shortcuts without both Control and Command may be intercepted before "
                                  "applications see them. Press it again to keep it.")
                             : tr("Shortcuts without Meta may be intercepted before applications see them. Press it "
                                  "again to keep it.");
    }
    if (result == "restore-success-with-conflicts")
        return tr("Some default shortcuts are in use by other actions and were not restored.");
    if (result == "not-found")
        return tr("This shortcut no longer exists.");
    return tr("The shortcut could not be changed.");
}

ShortcutsModel::Result ShortcutsModel::addAccelerator(const std::string& action, uint32_t keycode, uint32_t keysym,
                                                      uint32_t modifiers)
{
    return call("addAccelerator", action, keycode, keysym, modifiers);
}

ShortcutsModel::Result ShortcutsModel::replaceAccelerator(const std::string& action, uint32_t oldKeysym,
                                                          uint32_t oldModifiers, uint32_t keycode, uint32_t keysym,
                                                          uint32_t modifiers)
{
    return call("replaceAccelerator", action, oldKeysym, oldModifiers, keycode, keysym, modifiers);
}

ShortcutsModel::Result ShortcutsModel::removeAccelerator(const std::string& action, uint32_t keysym,
                                                         uint32_t modifiers)
{
    return call("removeAccelerator", action, keysym, modifiers);
}

ShortcutsModel::Result ShortcutsModel::restoreDefault(const std::string& action)
{
    return call("restoreDefault", action);
}

ShortcutsModel::Result ShortcutsModel::restoreAllDefaults()
{
    return call("restoreAllDefaults");
}

void ShortcutsModel::setRecording(bool recording)
{
    if (m_service.isAvailable())
        m_service.proxy().send("preventProcessingAccelerators", recording);
}

std::vector<std::string> ShortcutsModel::modifierParts(uint32_t modifiers) const
{
    // Under the Command convention Meta is the Command key next to the space
    // bar, Alt is Option and Ctrl is Control.
    std::vector<std::string> parts;
    if (modifiers & ControlDown)
        parts.push_back(m_commandKeys ? trc("key", "Control") : trc("key", "Ctrl"));
    if (modifiers & AltDown)
        parts.push_back(m_commandKeys ? trc("key", "Option") : trc("key", "Alt"));
    if (modifiers & ShiftDown)
        parts.push_back(trc("key", "Shift"));
    if (modifiers & MetaDown)
        parts.push_back(m_commandKeys ? trc("key", "Command") : trc("key", "Meta"));
    return parts;
}

std::vector<std::string> ShortcutsModel::partsForKey(uint32_t keycode, uint32_t keysym, uint32_t modifiers)
{
    std::vector<std::string> parts = modifierParts(modifiers);
    if (isModifierKey(keysym))
        return parts;
    std::string display;
    if (m_service.isAvailable()) {
        const dbus::Reply reply = m_service.proxy().call("describeKey", keycode, serviceKeysym(keysym));
        if (const std::optional<std::string> text = reply.value<std::string>()) {
            const json::Value object = json::Value::parse(*text).value_or(json::Value());
            display = keyDisplay(object["keyName"].toString(), object["keyText"].toString());
        }
    }
    if (display.empty()) {
        // The key's own name, as Qt's key sequences spell the common ones.
        char name[64] = {};
        xkb_keysym_get_name(xkb_keysym_to_upper(keysym), name, sizeof(name));
        display = name;
    }
    parts.push_back(display);
    return parts;
}

bool ShortcutsModel::isModifierKey(uint32_t keysym)
{
    switch (keysym) {
    case XKB_KEY_Shift_L:
    case XKB_KEY_Shift_R:
    case XKB_KEY_Control_L:
    case XKB_KEY_Control_R:
    case XKB_KEY_Alt_L:
    case XKB_KEY_Alt_R:
    case XKB_KEY_ISO_Level3_Shift:
    case XKB_KEY_Meta_L:
    case XKB_KEY_Meta_R:
    case XKB_KEY_Super_L:
    case XKB_KEY_Super_R:
    case XKB_KEY_Hyper_L:
    case XKB_KEY_Hyper_R:
    case XKB_KEY_Caps_Lock:
    case XKB_KEY_Num_Lock:
    case XKB_KEY_Scroll_Lock:
    case XKB_KEY_NoSymbol:
        return true;
    default:
        return false;
    }
}

uint32_t ShortcutsModel::modifierForKey(uint32_t keysym)
{
    switch (keysym) {
    case XKB_KEY_Shift_L:
    case XKB_KEY_Shift_R:
        return ShiftDown;
    case XKB_KEY_Control_L:
    case XKB_KEY_Control_R:
        return ControlDown;
    case XKB_KEY_Alt_L:
    case XKB_KEY_Alt_R:
        return AltDown;
    case XKB_KEY_Meta_L:
    case XKB_KEY_Meta_R:
    case XKB_KEY_Super_L:
    case XKB_KEY_Super_R:
        return MetaDown;
    default:
        return 0;
    }
}

uint32_t ShortcutsModel::serviceKeysym(uint32_t keysym)
{
    if (const auto it = kServiceKeysyms.find(keysym); it != kServiceKeysyms.end())
        return it->second;
    if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F35)
        return keysym;
    // The keypad's digits are Qt's digit keys.
    if (keysym >= XKB_KEY_KP_0 && keysym <= XKB_KEY_KP_9)
        return '0' + (keysym - XKB_KEY_KP_0);
    // Printable Latin-1 keys are their character, upper-cased as Qt's key
    // codes are; the compositor lower-cases them.
    if (keysym == XKB_KEY_space)
        return 0x20;
    if (keysym > 0x20 && keysym <= 0xff)
        return xkb_keysym_to_upper(keysym);
    return 0;
}

} // namespace settings
