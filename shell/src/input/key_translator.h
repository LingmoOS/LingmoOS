// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_INPUT_KEY_TRANSLATOR_H_
#define LINGMO_INPUT_KEY_TRANSLATOR_H_

#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <vector>

#include <string_view>

#include "input/types.h"

namespace Lingmo::Input {

class XkbTracker;

// Whether a window's application id (xdg_toplevel.set_app_id, or the X11
// class) names a terminal, the one kind of application treated apart.
bool isTerminal(std::string_view applicationId);

// Turns the keys the compositor did not consume into the keys clients see.
//
// Under ShortcutConvention::Pc every key passes through unchanged, and so it
// does under the Command convention while keys must reach the client as they
// are (a window inhibits shortcuts, or the shortcut recorder is listening).
//
// Otherwise the keys arrive with Command as Meta, Option as Alt and Control as
// Ctrl, whatever the keyboard (InputManager arranges that by position), and:
// - Command reaches clients as Ctrl, so Command+C copies in every
//   application; Option stays Alt and Control stays Ctrl.
// - Command, Option and Control are held back until a key or a click needs
//   them. Pressing one alone reaches no client (no menu bar pops up on a lone
//   Alt).
// - In a terminal Command+C/V/T/W/N/F go out as Ctrl+Shift+C/V/T/W/N/F and
//   Command+1 to 9 as Alt+1 to 9, the terminal's own shortcuts, so Control+C
//   still interrupts the program. Any other Command+letter reaches no
//   terminal, since as Ctrl it would be a control character. Nothing else is
//   rewritten.
// - Control+click is a secondary click.
//
// Ash's KeyboardModifierEventRewriter does this job on ChromeOS; like it, each
// key remembers what it was sent as, so its release matches whatever changed
// in between.
class KeyTranslator
{
public:
    using DeliverKey = std::function<void(Timestamp, uint32_t evdevCode, bool pressed)>;

    // `keymap` resolves the keysyms of the terminal keys; it is the tracker
    // the delivered keys feed.
    KeyTranslator(const XkbTracker *keymap, DeliverKey deliver);

    ShortcutConvention convention() const { return m_convention; }
    void setConvention(ShortcutConvention convention) { m_convention = convention; }

    // True while a terminal has keyboard focus.
    void setTerminalProvider(std::function<bool()> provider)
    {
        m_terminal = std::move(provider);
    }

    // True while keys must reach the focused client untranslated.
    void setPassthroughProvider(std::function<bool()> provider)
    {
        m_passthrough = std::move(provider);
    }

    // True while a pointer button is held: a drag reads the modifiers as they
    // change, so none are held back then.
    void setPointerButtonsProvider(std::function<bool()> provider)
    {
        m_pointerButtonsHeld = std::move(provider);
    }

    void onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed);

    // Sends the held modifiers a click or a scroll needs, e.g. Ctrl for
    // Command+click.
    void flushModifiers(Timestamp timestamp);

    // The button a client should see for this physical one.
    uint32_t translateButton(uint32_t evdevButton, bool pressed);

    // The codes clients hold pressed, modifiers included.
    std::vector<uint32_t> deliveredKeys() const;

private:
    struct Press {
        uint32_t delivered = 0; // 0 when nothing stays pressed
        bool rule = false;
    };

    bool passthrough() const;
    bool isModifier(uint32_t code) const;
    bool isHeldBack(uint32_t code) const;
    uint32_t deliveredModifier(uint32_t code, bool passthrough) const;
    uint32_t heldRoles() const;
    std::set<uint32_t> wantedModifiers(bool passthrough, bool immediateOnly) const;

    void syncModifiers(Timestamp timestamp, const std::set<uint32_t> &target);
    void syncModifierKinds(Timestamp timestamp, uint32_t kinds);
    void pressKey(Timestamp timestamp, uint32_t code);
    void releaseKey(Timestamp timestamp, uint32_t code);
    bool applyRule(Timestamp timestamp, uint32_t code);

    const XkbTracker *m_keymap;
    DeliverKey m_deliver;
    ShortcutConvention m_convention = ShortcutConvention::Pc;
    std::function<bool()> m_terminal;
    std::function<bool()> m_passthrough;
    std::function<bool()> m_pointerButtonsHeld;

    // Physical modifier keys held, in the order they went down.
    std::vector<uint32_t> m_heldModifiers;
    // Modifier codes the clients hold pressed.
    std::set<uint32_t> m_deliveredModifiers;
    // Every other physical key held, and what it went out as.
    std::map<uint32_t, Press> m_presses;
    // Other delivered keys; two physical keys can map to the same one.
    std::map<uint32_t, int> m_deliveredKeys;
    bool m_secondaryClick = false;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_KEY_TRANSLATOR_H_
