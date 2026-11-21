// Copyright 2012 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/shadow_controller.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_WM_SHADOW_CONTROLLER_H_
#define LINGMO_WM_SHADOW_CONTROLLER_H_

#include <map>
#include <memory>


#include "base/geometry.h"
#include "base/signal.h"
#include "wm/shadow_types.h"
#include "wm/window_observer.h"
#include "wm/window_shadow.h"

namespace Lingmo::WM {

class ShadowControllerDelegate;
class Window;
class WindowManager;

// Owns every window's shadow and decides, from one place, whether it is shown
// and how strong it is. Shadows are a presentation concern of the window
// manager: no decoration, frame or client surface owns one.
//
// Upstream splits this into ShadowController and ShadowController::Impl, where
// the Impl is a refcounted singleton observing aura::Env for every window in
// the process. Lingmo has one WindowManager, so there is one controller and
// no Impl. The observation set is the same: windows appearing and disappearing,
// activation, show state, bounds, visibility and drags.
//
// Must be destroyed before the WindowManager it observes.
class ShadowController : public WindowObserver
{
public:
    // `provider` may be null, which leaves every window without a shadow but
    // keeps the policy observable; that is what protocol-only tests want.
    ShadowController(WindowManager* manager, ShadowProvider* provider,
                     ShadowControllerDelegate* delegate);
    ~ShadowController() override;

    // Null until the window has one. Upstream: ShadowController::GetShadowForWindow.
    WindowShadow* shadowForWindow(const Window* window) const;

    ShadowElevation elevationForWindow(const Window* window) const;

    // Re-asks the delegate and re-applies bounds. Every observer below funnels
    // into this; upstream calls it HandlePossibleShadowVisibilityChange.
    void handlePossibleShadowVisibilityChange(Window* window);

    // --- WindowObserver ---

    void onWindowBoundsChanged(Window* window, const base::Rect& oldBounds) override;
    void onWindowStateTypeChanged(Window* window, WindowStateType oldType) override;
    void onWindowActivationChanged(Window* window, bool active) override;
    void onWindowVisibilityChanged(Window* window, bool visible) override;
    void onWindowContentChanged(Window* window) override;
    void onWindowDragStateChanged(Window* window, bool dragged) override;
    void onWindowClientDecorationChanged(Window* window) override;
    void onWindowFrameChanged(Window* window) override;

private:
    void onWindowAdded(Lingmo::WM::Window* window);
    void onWindowRemoved(Lingmo::WM::Window* window);

private:
    WindowShadow* createShadowForWindow(Window* window);

    WindowManager* m_manager = nullptr;
    ShadowProvider* m_provider = nullptr;
    ShadowControllerDelegate* m_delegate = nullptr;
    std::map<Window*, std::unique_ptr<WindowShadow>> m_shadows;
    base::Subscription m_windowAdded;
    base::Subscription m_windowRemoved;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_SHADOW_CONTROLLER_H_
