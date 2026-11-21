// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_keyboard_delegate.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Adaptation: keys are evdev codes already, so the DomCode conversion is gone,
// and the keymap travels through a memfd rather than a Chromium shared memory
// region.

#include "wayland/protocols/wayland_keyboard_delegate.h"

#include <sys/mman.h>
#include <unistd.h>
#include <wayland-server-protocol.h>

#include <algorithm>
#include <cmath>
#include <cstring>

#include "core/logging.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

WaylandKeyboardDelegate::WaylandKeyboardDelegate(wl_resource *keyboardResource,
                                                 SerialTracker *serialTracker,
                                                 clipboard::SelectionController *selection)
    : m_keyboardResource(keyboardResource)
    , m_serialTracker(serialTracker)
    , m_selection(selection ? selection->weakPtr() : nullptr)
{
}

WaylandKeyboardDelegate::~WaylandKeyboardDelegate() = default;

wl_client *WaylandKeyboardDelegate::client() const
{
    return wl_resource_get_client(m_keyboardResource);
}

bool WaylandKeyboardDelegate::canAcceptKeyboardEventsForSurface(compositor::Surface *surface) const
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    return surfaceResource && wl_resource_get_client(surfaceResource) == client();
}

void WaylandKeyboardDelegate::onKeyboardEnter(compositor::Surface *surface,
                                              const PressedKeys &pressedKeys)
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    if (!surfaceResource)
        return;

    wl_array keys;
    wl_array_init(&keys);
    for (const auto &entry : pressedKeys) {
        auto *value = static_cast<uint32_t *>(wl_array_add(&keys, sizeof(uint32_t)));
        if (value)
            *value = entry.second.code;
    }
    const auto serial = m_serialTracker->nextSerial(SerialTracker::OTHER_EVENT);
    if (m_selection)
        m_selection->recordSerial(client(), serial);
    wl_keyboard_send_enter(m_keyboardResource, serial, surfaceResource, &keys);
    wl_array_release(&keys);
    wl_client_flush(client());
}

void WaylandKeyboardDelegate::onKeyboardLeave(compositor::Surface *surface)
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    if (!surfaceResource)
        return;

    wl_keyboard_send_leave(m_keyboardResource,
                           m_serialTracker->nextSerial(SerialTracker::OTHER_EVENT),
                           surfaceResource);
    wl_client_flush(client());
}

uint32_t WaylandKeyboardDelegate::onKeyboardKey(input::Timestamp timestamp, uint32_t evdevCode,
                                                bool pressed)
{
    const uint32_t serial = m_serialTracker->maybeNextKeySerial();
    m_serialTracker->resetKeySerial();
    if (pressed)
        m_serialTracker->recordActivationSerial(serial, m_keyboardResource);
    if (m_selection)
        m_selection->recordSerial(client(), serial);
    wl_keyboard_send_key(m_keyboardResource, serial, timePointToMilliseconds(timestamp), evdevCode,
                         pressed ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
    wl_client_flush(client());
    return serial;
}

void WaylandKeyboardDelegate::onKeyboardModifiers(const input::KeyboardModifiers &modifiers)
{
    // Send the update only when they differ.
    if (m_currentModifiers == modifiers)
        return;
    m_currentModifiers = modifiers;
    sendKeyboardModifiers();
}

void WaylandKeyboardDelegate::sendKeyboardModifiers()
{
    wl_keyboard_send_modifiers(m_keyboardResource,
                               m_serialTracker->nextSerial(SerialTracker::OTHER_EVENT),
                               m_currentModifiers.depressed, m_currentModifiers.latched,
                               m_currentModifiers.locked, m_currentModifiers.group);
    wl_client_flush(client());
}

void WaylandKeyboardDelegate::onKeyboardLayoutUpdated(const std::string &keymap)
{
    // The client mmaps this, so it needs its own file description. The trailing
    // NUL is part of the advertised size.
    const size_t size = keymap.size() + 1;
    const int fd = memfd_create("lingmo-keymap", MFD_CLOEXEC);
    if (fd < 0) {
        LM_WARNING(lingmoWayland) << "Failed to create a keymap memfd";
        return;
    }
    if (ftruncate(fd, static_cast<off_t>(size)) != 0) {
        LM_WARNING(lingmoWayland) << "Failed to size the keymap memfd";
        close(fd);
        return;
    }

    void *memory = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        LM_WARNING(lingmoWayland) << "Failed to map the keymap memfd";
        close(fd);
        return;
    }
    std::memcpy(memory, keymap.data(), keymap.size());
    static_cast<char *>(memory)[keymap.size()] = '\0';
    munmap(memory, size);

    wl_keyboard_send_keymap(m_keyboardResource, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd,
                            static_cast<uint32_t>(size));
    wl_client_flush(client());
    close(fd);
    // The client starts the new keymap from an empty state.
    m_currentModifiers = {};
}

// Convert a key repeat interval to Wayland's repeat rate. An interval of 500ms
// is a rate of 1000/500 = 2 Hz.
//
// Known issue: a 2000ms interval is 0.5 Hz, which rounds to 1 Hz, twice as
// fast. That is not fixable without a change to the Wayland specification.
int32_t waylandRepeatRate(bool enabled, std::chrono::milliseconds interval)
{
    if (!enabled)
        return 0; // Disables key repeat, as documented in the Wayland spec.
    if (interval.count() <= 0)
        return 0;

    const int32_t rate =
            static_cast<int32_t>(std::lround(1000.0 / static_cast<double>(interval.count())));
    // Avoid disabling key repeat for intervals longer than 2000ms.
    return std::max(1, rate);
}

void WaylandKeyboardDelegate::onKeyRepeatSettingsChanged(bool enabled,
                                                         std::chrono::milliseconds delay,
                                                         std::chrono::milliseconds interval)
{
    if (wl_resource_get_version(m_keyboardResource) < WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        return;

    wl_keyboard_send_repeat_info(m_keyboardResource, waylandRepeatRate(enabled, interval),
                                 static_cast<int32_t>(std::max<int64_t>(0, delay.count())));
    wl_client_flush(client());
}

} // namespace Lingmo::Wayland
