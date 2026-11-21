// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/keyboard.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "input/keyboard.h"

#include <chrono>
#include <utility>

#include "compositor/surface.h"
#include "input/seat.h"

namespace Lingmo::Input {

Keyboard::Keyboard(std::unique_ptr<KeyboardDelegate> delegate, Seat* seat)
    : m_delegate(std::move(delegate))
    , m_seat(seat)
{
    m_seat->addObserver(this);
    m_seat->addKeyboard(this);

    m_delegate->onKeyboardLayoutUpdated(m_seat->clientXkbTracker()->keymap());
    onKeyRepeatSettingsChanged();

    // Take the focus the seat already has, so a keyboard created after the user
    // clicked does not sit dark until the next click.
    onSurfaceFocused(m_seat->focusedSurface(), nullptr);
}

Keyboard::~Keyboard()
{
    if (m_focus)
        m_focus->removeSurfaceObserver(this);
    m_seat->removeKeyboard(this);
    m_seat->removeObserver(this);
}

void Keyboard::setFocus(compositor::Surface* surface)
{
    m_lent = false;
    m_borrowedKeys.clear();
    if (m_focus) {
        m_delegate->onKeyboardLeave(m_focus);
        m_focus->removeSurfaceObserver(this);
        m_focus = nullptr;
    }
    if (surface) {
        m_pressedKeys = m_seat->clientPressedKeys();
        // Modifiers before enter: the client applies them to the pressed keys
        // the enter event carries.
        m_delegate->onKeyboardModifiers(m_seat->clientXkbTracker()->modifiers());
        m_delegate->onKeyboardEnter(surface, m_pressedKeys);
        m_focus = surface;
        m_focus->addSurfaceObserver(this);
    }
}

void Keyboard::onSurfaceFocused(compositor::Surface* gainedFocus, compositor::Surface*)
{
    compositor::Surface* target =
        gainedFocus && m_delegate->canAcceptKeyboardEventsForSurface(gainedFocus) ? gainedFocus
                                                                                  : nullptr;
    if (!target && m_focus && m_seat->focusIsBorrowed()) {
        m_lent = true;
        return;
    }
    if (target != m_focus) {
        setFocus(target);
    } else if (std::exchange(m_lent, false)) {
        for (const auto& [code, state] : m_seat->clientPressedKeys()) {
            if (!m_pressedKeys.contains(code))
                m_borrowedKeys.insert(code);
        }
    }
}

void Keyboard::onKeyboardModifierUpdated()
{
    // The seat has already updated the XkbTracker.
    if (m_focus)
        m_delegate->onKeyboardModifiers(m_seat->clientXkbTracker()->modifiers());
}

void Keyboard::onKeyboardLayoutUpdated()
{
    // The seat has already updated the XkbTracker. A new keymap resets the
    // client's xkb state, so the modifiers follow it.
    m_delegate->onKeyboardLayoutUpdated(m_seat->clientXkbTracker()->keymap());
    if (m_focus)
        m_delegate->onKeyboardModifiers(m_seat->clientXkbTracker()->modifiers());
}

void Keyboard::onKeyRepeatSettingsChanged()
{
    const KeyRepeatSettings &settings = m_seat->keyRepeatSettings();
    m_delegate->onKeyRepeatSettingsChanged(settings.enabled, settings.delay, settings.interval);
}

void Keyboard::onSurfaceDestroying(compositor::Surface* surface)
{
    if (surface != m_focus)
        return;
    // The surface is mid-destruction: report the leave but do not touch it
    // afterwards, and let it drop the observer registration itself.
    m_delegate->onKeyboardLeave(m_focus);
    m_focus = nullptr;
    m_lent = false;
}

void Keyboard::onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    if (!m_focus || m_seat->wasShutdown())
        return;

    if (pressed) {
        if (m_borrowedKeys.contains(evdevCode))
            return;
        // A lent keyboard still gets the releases and modifiers of what it
        // holds, so nothing stays stuck, and no new presses.
        if (m_lent)
            return;
        // A second press without an intervening release is not forwarded; the
        // client is responsible for its own repeat.
        if (m_pressedKeys.find(evdevCode) != m_pressedKeys.end())
            return;
        m_delegate->onKeyboardKey(timestamp, evdevCode, true);
        m_pressedKeys.emplace(evdevCode, KeyState {evdevCode});
    } else {
        if (m_borrowedKeys.erase(evdevCode))
            return;
        auto it = m_pressedKeys.find(evdevCode);
        // Only release what this surface was told about. A key pressed before
        // it had focus is in the set because enter carried it; a key it never
        // saw is dropped.
        if (it == m_pressedKeys.end())
            return;
        // Exo reports the code that was delivered on press, not the code of the
        // current event, so a client tracking physical state stays consistent.
        m_delegate->onKeyboardKey(timestamp, it->second.code, false);
        m_pressedKeys.erase(it);
    }
}

} // namespace Lingmo::Input
