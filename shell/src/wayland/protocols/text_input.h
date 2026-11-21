// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <wayland-server-core.h>

#include "base/geometry.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {
class Seat;
}

namespace Lingmo::Wayland {
struct WaylandSeat;

// Application text state in text-input-v3 / input-method-v2 units: UTF-8 byte
// offsets and v3 content-type values.
struct TextInputState {
    std::optional<std::string> surrounding;
    int32_t cursor = 0;
    int32_t anchor = 0;
    uint32_t cause = 0;
    std::optional<std::pair<uint32_t, uint32_t>> content;
    std::optional<base::Rect> rectangle;
};

// One input-method-v2 commit, already validated against the text input's state.
struct TextInputEdit {
    std::optional<std::pair<uint32_t, uint32_t>> deletion;
    std::optional<std::string> commit;
    std::string preedit;
    int32_t begin = 0;
    int32_t end = 0;
};

class InputMethodRelay;

// One application text field, modelled on exo::TextInput with the protocol
// adapter as its delegate.
class TextInput
{
public:
    explicit TextInput(InputMethodRelay *relay)
        : m_relay(relay)
    {
    }

    virtual ~TextInput() = default;
    TextInput(const TextInput &) = delete;
    TextInput &operator=(const TextInput &) = delete;

    wl_resource *resource() const { return m_resource; }

    const TextInputState &state() const { return m_state; }

    // Bound to the focused surface; the relay additionally checks seat focus.
    virtual bool attached() const = 0;
    // For a text input with no resource, the compositor's own: whether
    // `surface`, the keyboard-focused one, is its window's. A resource's
    // client decides for the others.
    virtual bool ownsSurface(const compositor::Surface *) const { return false; }
    virtual void sendEdit(const TextInputEdit &edit) = 0;

    // Another text input took over the input method.
    virtual void detached() { }

    // Runs after the relay has dropped the input-method context.
    virtual void focusChanged(compositor::Surface *gained, compositor::Surface *lost) = 0;

protected:
    InputMethodRelay *const m_relay;
    wl_resource *m_resource = nullptr;
    TextInputState m_state;
};

// The input-method side seen by application text-input adapters.
class InputMethodRelay
{
public:
    virtual input::Seat *seat() const = 0;
    virtual WaylandSeat *waylandSeat() const = 0;
    virtual compositor::Surface *focusedSurface() const = 0;
    // `client` owns the keyboard-focused surface and the session accepts input.
    virtual bool focusedClient(const wl_client *client) const = 0;
    virtual TextInput *active() const = 0;
    // The preedit the input method currently shows in `textInput`; empty when inactive.
    virtual TextInputEdit currentPreedit(const TextInput *textInput) const = 0;
    // Makes `textInput` active; `restart` starts a fresh input-method context.
    virtual void activate(TextInput *textInput, bool restart) = 0;
    virtual void deactivate(TextInput *textInput) = 0;
    // Sends the active text input's state, now or batched at the next idle.
    virtual void stateChanged(TextInput *textInput, bool batched) = 0;
    virtual void add(TextInput *textInput) = 0;
    virtual void remove(TextInput *textInput) = 0;
    // The resource is destroyed with the relay if the client outlives it.
    virtual wl_resource *createResource(wl_client *client, const wl_interface *interface,
                                        uint32_t id, const void *implementation, void *data,
                                        wl_resource_destroy_func_t destroy) = 0;
    virtual void forgetResource(wl_resource *resource) = 0;

protected:
    ~InputMethodRelay() = default;
};

bool validImeText(const std::string &text);
bool validImeOffset(const std::string &text, int64_t offset);
bool validCursorRectangle(int32_t x, int32_t y, int32_t width, int32_t height);

wl_global *createTextInputV3Global(wl_display *display, InputMethodRelay *relay);
wl_global *createTextInputV1Global(wl_display *display, InputMethodRelay *relay);

} // namespace Lingmo::Wayland
