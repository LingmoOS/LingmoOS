// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <optional>
#include <memory>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/region.h"
#include "base/signal.h"
#include "input/cursor_shape.h"
#include "render/glass.h"
#include "ui/bitmap.h"
#include "ui/path.h"
#include "ui/window.h"
#include "wm/types.h"
#include "wm/window_manager.h"
#include "wm/window_observer.h"

namespace Lingmo::Compositor {
class Buffer;
class Surface;
}

namespace Lingmo::Render {
class Renderer;
}

namespace Lingmo::Toolkit {
class PanelTexture;
}

namespace Lingmo::ui {
class EmbeddedWindow;
class View;
} // namespace Lingmo::UI

namespace Lingmo::WM {
class ShellSurface;
class Window;
} // namespace Lingmo::WM

namespace Lingmo::Shell {

class PanelBuffer;
class Shell;

// A window of the shell drawn inside the compositor: a toolkit
// EmbeddedWindow whose frames are the contents of a compositor-side surface
// with no client, given a wm::ShellSurface and shell metadata, as
// lingmo_shell_surface_v1 gives the trusted desktop's surface. Stacking,
// work area, focus, activation, desks and overview treat it as any window of
// its role; input reaches it through the seat.
//
// Ash's equivalent is a views::Widget in a shell container: its aura::Window
// is what the window manager, event targeting and focus see, and its layer
// tree is drawn by the same compositor as everything else.
class ShellWindow : public wm::WindowObserver
{
public:
    // A toplevel of `role`, on no output in particular until setOutput().
    ShellWindow(Shell *shell, wm::WindowRole role);
    // Hosts `popup`, a popup the toolkit made with Window::createPopup()
    // whose popupParent() is `parent`'s window: placed by the compositor's
    // xdg positioner from its popupPlacement(), holding an xdg_popup grab
    // when isGrabbingPopup(). The toolkit owns `popup`; see ShellHost.
    ShellWindow(ShellWindow *parent, ui::EmbeddedWindow *popup);
    ShellWindow(const ShellWindow &) = delete;
    ShellWindow &operator=(const ShellWindow &) = delete;
    ~ShellWindow() override;

    Shell *shell() const { return m_shell; }
    // Null once a popup's toolkit window is gone; see release().
    ui::EmbeddedWindow *window() const { return m_window; }
    void setContentView(std::unique_ptr<ui::View> view);

    compositor::Surface *surface() const { return m_surface.get(); }
    wm::ShellSurface *shellSurface() const { return m_shellSurface.get(); }
    // Null while hidden.
    wm::Window *wmWindow() const;
    wm::WindowRole role() const { return m_metadata.role; }
    bool isPopup() const { return m_parent != nullptr; }

    // --- What lingmo_shell_surface_v1 says, applied at once ---

    void setFocusPolicy(wm::FocusPolicy policy);
    void setPanelBehavior(wm::PanelBehavior behavior);
    // The logical output, by wl_output name, the window belongs to: its
    // exclusive zones apply there and it takes that output's scale. Empty
    // follows the output under the window's position.
    void setOutput(const std::string &name);
    const std::string &outputName() const { return m_metadata.outputName; }
    // The window a Sheet hangs from, by wm::Window::id.
    void setParentWindow(uint64_t windowId);
    // Keeps computer control out of the window and its popups; see
    // wm::Window::isProtectedFromControl.
    void setProtectedFromControl(bool protectedFromControl);
    // A dialog of the shell's own, for a toplevel of the Normal role: off the
    // task list and the menu bar, and held at the size setSize() gives it, so
    // the user moves it but cannot resize, maximize or snap it.
    void setDialog(bool dialog);
    void setExclusiveZone(wm::ScreenEdge edge, int thickness, bool yieldsToMaximized = false);
    void clearExclusiveZone();
    // Takes the keyboard: activation, or for the Capture and Picker roles
    // the window manager's keyboard grab.
    void activate();

    // --- Geometry, in global logical coordinates ---

    // Authoritative, as a shell surface's set_position.
    void setPosition(const base::Point &position);
    void setSize(const base::Size &size);
    base::Size size() const { return m_size; }
    void setGeometry(const base::Rect &geometry);
    base::Rect geometry() const { return base::Rect(m_position, m_size); }
    // Where the window is on screen now: its window's bounds with the
    // presentation offset, or the requested geometry while hidden.
    base::Rect drawnBounds() const;
    // The output scale the window renders at.
    double scale() const { return m_scale; }
    // Whether its last frame was drawn by Skia on the renderer's GL context.
    bool lastFrameOnGpu() const;

    // Where the window takes input, in window coordinates; nothing for the
    // whole window, an empty region for none (an OSD).
    void setInputRegion(std::optional<base::Region> region);

    // A cursor of the component's own over its views' shapes: a theme shape
    // the toolkit has no name for, or a picture of `image.width() / scale`
    // logical pixels with `hotspot` (logical) under the pointer. Unset
    // follows the views.
    struct Cursor {
        input::CursorShape shape = input::CursorShape::Default;
        ui::Bitmap image;
        int scale = 1;
        base::Point hotspot;
    };
    void setCursor(std::optional<Cursor> cursor);
    const std::optional<Cursor> &cursor() const { return m_cursor; }
    // The surface showing the cursor's picture; null for a shape.
    compositor::Surface *cursorSurface() const { return m_cursorSurface.get(); }

    // Background blur and glass behind the window, in window coordinates, as
    // the surface blur protocol sets them for a client.
    // `shape`, in window coordinates, masks the glass, outline and shadow,
    // as a bubble's arrow takes them; empty for the rounded rect.
    void setBackgroundBlur(const base::Rect &rect, int radius,
                           const render::GlassParams &glass = {}, const ui::Path &shape = {});

    // --- Showing ---

    // Maps the window, which then has a wm::Window; its first frame is
    // rendered before the next output frame is composed. A popup whose
    // parent is hidden stays unmapped.
    void show();
    void hide();
    bool isShown() const { return m_shown; }

    // The window manager asked to close the window (a toplevel). A popup's
    // dismissal goes to the toolkit instead, as closePopups() on its root.
    std::function<void()> onCloseRequested;
    // The window was mapped or unmapped, its wm::Window created or destroyed.
    base::Signal<> mappedChanged;
    // A frame of the window went to its surface, ahead of the output frame
    // that composes it.
    std::function<void()> onFrameProduced;
    // The seat's keyboard came to the window or left it.
    std::function<void(bool focused)> onKeyboardFocusChanged;

    // A frame at the next output frame, which commits the surface even when
    // nothing in the window changed: the synchronized subsurfaces of other
    // clients in it show what they committed.
    void requestFrame() { scheduleRender(); }

    // Places the popup again from its toolkit window's placement and size.
    void place();
    // The popup's bounds relative to its parent's, as the positioner put it.
    base::Rect popupBounds() const;
    // The toolkit window and its root, the non-popup window it hangs from.
    ShellWindow *root();
    // Unmaps the window and lets go of its toolkit window, which is being
    // hidden or destroyed: nothing reaches it from here on.
    void release();

    // Called by WindowRenderer; not part of the public API.
    void produceFrame(render::Renderer &renderer, uint64_t frameTimeNs);
    // Called by ShellInput; not part of the public API.
    void setKeyboardFocus(bool focused);

    // --- wm::WindowObserver ---

    void onWindowActivationChanged(wm::Window *window, bool active) override;
    void onWindowDestroying(wm::Window *window) override;

private:
    // Makes a toolkit window of its own for a null `popup`.
    ShellWindow(Shell *shell, ShellWindow *parent, ui::EmbeddedWindow *popup, wm::WindowRole role);
    void applyMetadata();
    void updateScale();
    // The buffer of the window's size and scale, committed; maps the window
    // when shown and not yet mapped.
    void commit();
    void scheduleRender();
    void observeWindow();
    void setPopupBounds(const base::Rect &bounds);
    void updateSizeLimits();
    base::Rect placePopup(const ui::PopupPlacement &placement, const base::Size &size) const;

    Shell *m_shell;
    ShellWindow *m_parent = nullptr;
    std::vector<ShellWindow *> m_popups;
    ui::EmbeddedWindow *m_window = nullptr;
    // Null for a popup, whose window the toolkit owns.
    std::unique_ptr<ui::EmbeddedWindow> m_ownedWindow;
    std::unique_ptr<compositor::Surface> m_surface;
    std::unique_ptr<wm::ShellSurface> m_shellSurface;
    std::unique_ptr<toolkit::PanelTexture> m_texture;
    std::shared_ptr<PanelBuffer> m_buffer;
    std::optional<Cursor> m_cursor;
    std::unique_ptr<compositor::Surface> m_cursorSurface;
    // The surface only refers to its buffer, as to a client's wl_buffer.
    std::shared_ptr<compositor::Buffer> m_cursorBuffer;
    wm::ShellWindowMetadata m_metadata;
    base::Point m_position;
    base::Size m_size;
    double m_scale = 1;
    bool m_shown = false;
    bool m_grab = false;
    bool m_dialog = false;
    bool m_keyboardFocus = false;
    // Inside place(): the geometry it reports may resize the popup.
    bool m_placing = false;
    // Inside produceFrame(): state set while the toolkit renders goes out
    // with the frame's own commit.
    bool m_producing = false;
    wm::Window *m_observed = nullptr;
    base::Subscription m_outputsChanged;
    // False from the top of the destructor, for callbacks the window manager
    // holds.
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell

