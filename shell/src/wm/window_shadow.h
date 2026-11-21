// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The window manager's view of a compositor-drawn shadow, split off the same
// way wm::WindowFrame is split off the decoration: the controller decides
// whether there is a shadow and how strong it is, and something below the
// Wayland layer draws it.
//
// Upstream this is ui::Shadow (ui/compositor_extra/shadow.h), which
// ui::wm::ShadowController owns directly. Lingmo's decoration library sits
// *above* the window manager, so the concrete shadow arrives through a
// provider rather than being constructed here.

#ifndef LINGMO_WM_WINDOW_SHADOW_H_
#define LINGMO_WM_WINDOW_SHADOW_H_

#include <memory>

#include "base/geometry.h"
#include "wm/shadow_types.h"

namespace Lingmo::WM {

class Window;

class WindowShadow
{
public:
    virtual ~WindowShadow() = default;

    // ShadowElevation::None hides the shadow without destroying it, which is
    // what ui::Shadow::SetElevation does with kNone.
    virtual void setElevation(ShadowElevation elevation) = 0;
    virtual ShadowElevation elevation() const = 0;

    // The outer window bounds the shadow is cast by, in output coordinates.
    virtual void setContentBounds(const base::Rect& bounds) = 0;
    virtual base::Rect contentBounds() const = 0;

    // Everything the shadow paints, in output coordinates: contentBounds()
    // grown by the style's exterior margins. Empty when nothing is drawn.
    virtual base::Rect shadowBounds() const = 0;

    // Puts the shadow back underneath everything else the window draws.
    // Upstream: window->layer()->StackAtBottom(shadow->layer()).
    virtual void stackAtBottom() = 0;
};

// Creates shadows, and is installed on the ShadowController the way a
// FrameProvider is installed on the WindowManager.
class ShadowProvider
{
public:
    virtual ~ShadowProvider() = default;

    // May return null for a window with no scene node, which is what the
    // protocol-only tests have.
    virtual std::unique_ptr<WindowShadow> createShadow(Window* window) = 0;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_SHADOW_H_
