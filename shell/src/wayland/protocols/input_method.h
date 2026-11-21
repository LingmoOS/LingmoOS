// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/geometry.h"
#include <functional>
#include <memory>
#include <string>
#include <wayland-server-core.h>
namespace scene = Lingmo::Scene;

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::Wayland {
struct WaylandSeat;
class InputMethodRelay;

// Where a candidate popup of `size` goes for the text cursor `caret`, both in
// output coordinates: below the caret, above it when only there it fits, and
// kept inside `area`. Upstream: views::BubbleFrameView's
// adjust_if_offscreen, mirroring the arrow and then clamping to the work area.
base::Point inputPopupPosition(const base::Rect &caret, const base::Size &size,
                               const base::Rect &area);

// Owns the application/IME relay; resources must die before the seat and display.
class InputMethodManager
{
public:
    // Candidate popups are placed in `popupLayer`, in output coordinates, so they
    // stack above every window, including the application's own popups.
    InputMethodManager(wl_display *display, WaylandSeat *seat, scene::Node *popupLayer);
    ~InputMethodManager();
    InputMethodManager(const InputMethodManager &) = delete;
    InputMethodManager &operator=(const InputMethodManager &) = delete;

    // Keeps popups on the text cursor when windows move; call on scene changes.
    void sceneChanged();

    // Releases the globals; must run before the display is destroyed.
    void shutdown();

    // The work area a candidate popup stays inside, for a text cursor in output
    // coordinates. Unset, popups only follow the cursor.
    void setPlacementArea(std::function<base::Rect(const base::Rect &caret)> area);

    // Restricts input-method and virtual-keyboard binds; unset allows every client.
    void setAuthorizer(std::function<bool(const wl_client *)> authorizer);

    // What text inputs of the compositor's own register with.
    InputMethodRelay *relay() const;

    // A text field of the keyboard-focused client is enabled.
    bool hasFocusedTextInput() const;

    // The input method takes the keys typed into that field, to compose.
    bool grabsKeyboard() const;

    // Commits UTF-8 text to that field as an input method would, keeping the
    // input method's preedit. False, and nothing sent, without one.
    bool commitText(const std::string &text);

    wl_global *textInputGlobal() const;
    wl_global *textInputV1Global() const;
    wl_global *inputMethodGlobal() const;
    wl_global *virtualKeyboardGlobal() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace Lingmo::Wayland
