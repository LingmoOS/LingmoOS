// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/key_translator.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>
#include <string>
#include <utility>

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "input/xkb_tracker.h"

namespace Lingmo::Input {
namespace {

// The modifiers held, by role.
enum Role : uint32_t {
    Command = 1u << 0,
    Option = 1u << 1,
    Control = 1u << 2,
    Shift = 1u << 3,
};

// The modifiers sent with a key, by kind.
enum ModifierKind : uint32_t {
    NoKind = 0,
    CtrlKind = 1u << 0,
    AltKind = 1u << 1,
    ShiftKind = 1u << 2,
    MetaKind = 1u << 3,
    Level3Kind = 1u << 4,
};

// Terminals take their own shortcuts with Ctrl+Shift, leaving Ctrl to the
// program: copy, paste, new tab, close tab, new window, find.
constexpr std::array<uint32_t, 6> kTerminalCtrlShiftKeys = {XKB_KEY_c, XKB_KEY_v, XKB_KEY_t,
                                                           XKB_KEY_w, XKB_KEY_n, XKB_KEY_f};

uint32_t defaultCode(ModifierKind kind)
{
    switch (kind) {
    case CtrlKind:
        return KEY_LEFTCTRL;
    case AltKind:
        return KEY_LEFTALT;
    case ShiftKind:
        return KEY_LEFTSHIFT;
    default:
        return 0;
    }
}

} // namespace

bool isTerminal(std::string_view applicationId)
{
    // Lower-cased application ids and X11 classes, after Toshy's lists.
    static const std::set<std::string, std::less<>> terminals = {
            "lingmo-terminal",
            "alacritty",
            "kitty",
            "foot",
            "footclient",
            "org.gnome.terminal",
            "gnome-terminal",
            "gnome-terminal-server",
            "org.gnome.console",
            "kgx",
            "org.gnome.ptyxis",
            "org.kde.konsole",
            "konsole",
            "org.kde.yakuake",
            "yakuake",
            "xfce4-terminal",
            "terminator",
            "tilix",
            "com.gexperts.tilix",
            "org.wezfurlong.wezterm",
            "wezterm",
            "wezterm-gui",
            "com.mitchellh.ghostty",
            "ghostty",
            "io.elementary.terminal",
            "com.system76.cosmicterm",
            "com.raggesilver.blackbox",
            "deepin-terminal",
            "dev.warp.warp",
            "contour",
            "qterminal",
            "lxterminal",
            "mate-terminal",
            "terminology",
            "guake",
            "tilda",
            "sakura",
            "roxterm",
            "tabby",
            "hyper",
            "xterm",
            "uxterm",
            "urxvt",
            "st",
            "st-256color",
    };

    std::string lower(applicationId);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return terminals.contains(lower);
}

KeyTranslator::KeyTranslator(const XkbTracker *keymap, DeliverKey deliver)
    : m_keymap(keymap)
    , m_deliver(std::move(deliver))
{
}

bool KeyTranslator::passthrough() const
{
    return m_convention == ShortcutConvention::Pc || (m_passthrough && m_passthrough());
}

bool KeyTranslator::isModifier(uint32_t code) const
{
    switch (code) {
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
        return true;
    default:
        return false;
    }
}

// Command, Option and Control wait for a key; Shift and AltGr pick the
// characters typed, so clients need them at once.
bool KeyTranslator::isHeldBack(uint32_t code) const
{
    switch (code) {
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
    case KEY_LEFTALT:
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
        return true;
    case KEY_RIGHTALT:
        return m_keymap->baseKeysym(KEY_RIGHTALT) != XKB_KEY_ISO_Level3_Shift;
    default:
        return false;
    }
}

uint32_t KeyTranslator::deliveredModifier(uint32_t code, bool passthrough) const
{
    if (passthrough)
        return code;
    if (code == KEY_LEFTMETA)
        return KEY_LEFTCTRL;
    if (code == KEY_RIGHTMETA)
        return KEY_RIGHTCTRL;
    return code;
}

uint32_t KeyTranslator::heldRoles() const
{
    uint32_t roles = 0;
    for (const uint32_t code : m_heldModifiers) {
        switch (code) {
        case KEY_LEFTMETA:
        case KEY_RIGHTMETA:
            roles |= Command;
            break;
        case KEY_LEFTALT:
            roles |= Option;
            break;
        case KEY_RIGHTALT:
            if (isHeldBack(code))
                roles |= Option;
            break;
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL:
            roles |= Control;
            break;
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT:
            roles |= Shift;
            break;
        }
    }
    return roles;
}

std::set<uint32_t> KeyTranslator::wantedModifiers(bool passthrough, bool immediateOnly) const
{
    std::set<uint32_t> wanted;
    for (const uint32_t code : m_heldModifiers) {
        if (!immediateOnly || passthrough || !isHeldBack(code))
            wanted.insert(deliveredModifier(code, passthrough));
    }
    return wanted;
}

void KeyTranslator::syncModifiers(Timestamp timestamp, const std::set<uint32_t> &target)
{
    const std::set<uint32_t> delivered = m_deliveredModifiers;
    for (const uint32_t code : delivered) {
        if (!target.contains(code)) {
            m_deliveredModifiers.erase(code);
            m_deliver(timestamp, code, false);
        }
    }
    // In the order the keys went down, then whatever a rule adds.
    std::vector<uint32_t> order;
    const auto add = [&](uint32_t code) {
        if (target.contains(code) && std::find(order.begin(), order.end(), code) == order.end())
            order.push_back(code);
    };
    for (const uint32_t code : m_heldModifiers) {
        add(deliveredModifier(code, false));
        add(code);
    }
    for (const uint32_t code : target)
        add(code);
    for (const uint32_t code : order) {
        if (m_deliveredModifiers.insert(code).second)
            m_deliver(timestamp, code, true);
    }
}

void KeyTranslator::syncModifierKinds(Timestamp timestamp, uint32_t kinds)
{
    const auto kindOf = [this](uint32_t code) -> ModifierKind {
        switch (code) {
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL:
            return CtrlKind;
        case KEY_LEFTALT:
            return AltKind;
        case KEY_RIGHTALT:
            return isHeldBack(code) ? AltKind : Level3Kind;
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT:
            return ShiftKind;
        case KEY_LEFTMETA:
        case KEY_RIGHTMETA:
            return MetaKind;
        default:
            return NoKind;
        }
    };

    std::set<uint32_t> target;
    uint32_t covered = NoKind;
    for (const uint32_t code : m_deliveredModifiers) {
        if (kinds & kindOf(code)) {
            target.insert(code);
            covered |= kindOf(code);
        }
    }
    for (const ModifierKind kind : {CtrlKind, AltKind, ShiftKind}) {
        if ((kinds & kind) && !(covered & kind))
            target.insert(defaultCode(kind));
    }
    syncModifiers(timestamp, target);
}

void KeyTranslator::pressKey(Timestamp timestamp, uint32_t code)
{
    if (m_deliveredKeys[code]++ == 0)
        m_deliver(timestamp, code, true);
}

void KeyTranslator::releaseKey(Timestamp timestamp, uint32_t code)
{
    auto it = m_deliveredKeys.find(code);
    if (it == m_deliveredKeys.end())
        return;
    if (--it->second == 0) {
        m_deliveredKeys.erase(it);
        m_deliver(timestamp, code, false);
    }
}

bool KeyTranslator::applyRule(Timestamp timestamp, uint32_t code)
{
    const uint32_t roles = heldRoles();
    if (!(roles & Command) || !m_terminal || !m_terminal())
        return false;

    uint32_t kinds = NoKind;
    const uint32_t keysym = m_keymap->baseKeysym(code);
    if (roles == Command
        && std::find(kTerminalCtrlShiftKeys.begin(), kTerminalCtrlShiftKeys.end(), keysym)
                != kTerminalCtrlShiftKeys.end()) {
        kinds = CtrlKind | ShiftKind;
    } else if (roles == Command && code >= KEY_1 && code <= KEY_9) {
        // Terminals pick a tab with Alt+digit. By position, since some layouts
        // type symbols on the digit row.
        kinds = AltKind;
    } else if (keysym >= XKB_KEY_a && keysym <= XKB_KEY_z) {
        // As Ctrl a letter is a control character to the program: Command+D
        // would end the shell and Command+Z suspend it. Other keys go out.
        m_presses.emplace(code, Press {0, true});
        return true;
    } else {
        return false;
    }

    syncModifierKinds(timestamp, kinds);
    pressKey(timestamp, code);
    m_presses.emplace(code, Press {code, true});
    return true;
}

void KeyTranslator::onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    const bool passthrough = this->passthrough();

    if (isModifier(evdevCode)) {
        if (pressed) {
            if (std::find(m_heldModifiers.begin(), m_heldModifiers.end(), evdevCode)
                != m_heldModifiers.end())
                return;
            m_heldModifiers.push_back(evdevCode);
            if (passthrough || (m_pointerButtonsHeld && m_pointerButtonsHeld())) {
                syncModifiers(timestamp, wantedModifiers(passthrough, false));
            } else if (!isHeldBack(evdevCode)) {
                std::set<uint32_t> target = m_deliveredModifiers;
                target.insert(deliveredModifier(evdevCode, passthrough));
                syncModifiers(timestamp, target);
            }
            return;
        }
        m_heldModifiers.erase(
                std::remove(m_heldModifiers.begin(), m_heldModifiers.end(), evdevCode),
                m_heldModifiers.end());
        // A key's release takes back what it sent, whatever the mode is now.
        std::set<uint32_t> wanted = wantedModifiers(false, false);
        wanted.merge(wantedModifiers(true, false));
        std::set<uint32_t> target;
        for (const uint32_t code : m_deliveredModifiers) {
            if (wanted.contains(code))
                target.insert(code);
        }
        syncModifiers(timestamp, target);
        return;
    }

    if (pressed) {
        if (m_presses.contains(evdevCode))
            return;
        if (!passthrough && applyRule(timestamp, evdevCode))
            return;
        syncModifiers(timestamp, wantedModifiers(passthrough, false));
        pressKey(timestamp, evdevCode);
        m_presses.emplace(evdevCode, Press {evdevCode, false});
        return;
    }

    const auto it = m_presses.find(evdevCode);
    if (it == m_presses.end())
        return;
    const Press press = it->second;
    m_presses.erase(it);
    if (press.delivered)
        releaseKey(timestamp, press.delivered);
    if (press.rule) {
        // Back to what the held keys send: a modifier only the rule wanted
        // goes, one it lifted stays held back until needed, Shift returns.
        std::set<uint32_t> target = wantedModifiers(passthrough, true);
        for (const uint32_t code : wantedModifiers(passthrough, false)) {
            if (m_deliveredModifiers.contains(code))
                target.insert(code);
        }
        syncModifiers(timestamp, target);
    }
}

void KeyTranslator::flushModifiers(Timestamp timestamp)
{
    // A secondary click made of Control+click carries no Ctrl.
    if (m_secondaryClick)
        return;
    syncModifiers(timestamp, wantedModifiers(passthrough(), false));
}

uint32_t KeyTranslator::translateButton(uint32_t evdevButton, bool pressed)
{
    if (evdevButton != BTN_LEFT)
        return evdevButton;
    if (!pressed) {
        if (!std::exchange(m_secondaryClick, false))
            return evdevButton;
        return BTN_RIGHT;
    }
    if (passthrough() || heldRoles() != Control)
        return evdevButton;
    m_secondaryClick = true;
    return BTN_RIGHT;
}

std::vector<uint32_t> KeyTranslator::deliveredKeys() const
{
    std::vector<uint32_t> keys(m_deliveredModifiers.begin(), m_deliveredModifiers.end());
    for (const auto &[code, count] : m_deliveredKeys)
        keys.push_back(code);
    return keys;
}

} // namespace Lingmo::Input
