// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/geometry.h"
#include "scene/window_visual_node.h"
#include "wm/window_observer.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Decoration {
struct WindowVisualStyle;
}

namespace Lingmo::WM {

class WindowVisualController : public WindowObserver
{
public:
    explicit WindowVisualController(Window* window);
    ~WindowVisualController() override;
    static scene::WindowVisualState evaluate(const Window& window,
                                             const decoration::WindowVisualStyle& style);
    // Same, against the default style; kept out of line so the defaults stay
    // in window_visual_style.cpp.
    static scene::WindowVisualState evaluate(const Window& window);

    scene::WindowVisualNode* node() const { return m_node; }

    void update();

    // Re-evaluates `window` from outside its own controller; a no-op for null.
    static void refresh(Window* window);

    void onWindowBoundsChanged(Window*, const base::Rect&) override { update(); }

    void onWindowContentChanged(Window*) override { update(); }

    void onWindowFrameChanged(Window*) override { update(); }

    void onWindowClientDecorationChanged(Window*) override { update(); }

    void onWindowStateTypeChanged(Window*, WindowStateType) override { update(); }

    void onWindowActivationChanged(Window*, bool) override { update(); }

    void onWindowResponsiveChanged(Window*) override { update(); }

    // The parent's dim follows whether this window is a modal in effect.
    void onWindowVisibilityChanged(Window* window, bool) override;

    void onWindowModalChanged(Window* window) override;

    void onWindowTransientParentChanged(Window* window, Window* oldParent) override;

private:
    Window* m_window;
    scene::WindowVisualNode* m_node = nullptr;
};

} // namespace Lingmo::WM
