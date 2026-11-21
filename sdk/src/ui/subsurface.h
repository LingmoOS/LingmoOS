// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <functional>
#include <memory>

#include "base/geometry.h"
#include "ui/geometry.h"

struct wl_display;

namespace Lingmo::ui {

class Window;
struct SubsurfacePrivate;

// A child surface of a window that another thread draws with OpenGL ES 3
// through EGL, such as a video. It sits in the window's logical
// coordinates, below or above the window's own surface, and takes no input:
// the window keeps every pointer event. Its EGL context is its own and
// shares nothing with the window's.
//
// Wayland only. Headless and embedded windows have none: isValid() is false
// and beginFrame() returns an empty size, so such callers draw in software.
class Subsurface
{
public:
    enum class Stacking {
        Below,
        Above,
    };

    // A toplevel's, or a popup's once shown.
    explicit Subsurface(Window* window, Stacking stacking = Stacking::Below);
    // On the UI thread, after the drawing thread called doneCurrent() for
    // the last time.
    ~Subsurface();

    Subsurface(const Subsurface&) = delete;
    Subsurface& operator=(const Subsurface&) = delete;

    bool isValid() const;

    // UI thread. Both land with the window's next frame.
    void setGeometry(const RectF& rect);
    RectF geometry() const;
    // An opaque surface (the default) lets the compositor skip what is
    // under it. Applies with the next buffer.
    void setOpaque(bool opaque);

    // Buffers show as soon as they are swapped, except across a size change:
    // the window frame that changes the subsurface's size first calls this
    // on the UI thread, then waits up to `resizeTimeoutMs` for the drawing
    // thread to swap a buffer of that size, so the window and the subsurface
    // change size in the same commit. The drawing thread must not wait for
    // the UI thread meanwhile. Without it, a new size shows with the first
    // buffer drawn after that frame.
    std::function<void(base::Size deviceSize)> onResize;
    int resizeTimeoutMs = 100;

    // Any thread.
    static void* getProcAddress(const char* name);
    // The EGLDisplay, for an EGL-based renderer.
    void* eglDisplay() const;
    // For zero-copy decoders that share buffers with the compositor.
    static wl_display* waylandDisplay();

    // Drawing thread. makeCurrent() also binds OpenGL ES on the thread.
    bool makeCurrent();
    void doneCurrent();
    // Makes the context current and takes the size set last: the device
    // pixels to draw the default framebuffer at, empty when there is nothing
    // to draw (no size, no surface).
    base::Size beginFrame();
    // Presents what was drawn since beginFrame(), without waiting for the
    // compositor, and asks for a frame callback.
    bool endFrame();
    // Blocks until the compositor wants the next frame after the last
    // endFrame(), or the timeout, or wakeUp(); false unless the frame came,
    // as while the window is hidden. A synchronised subsurface hears of no
    // frame until the window commits.
    bool waitForFrame(int timeoutMs);
    // Any thread: ends a waitForFrame() now or the next one, as for new
    // content or a resize.
    void wakeUp();

private:
    std::unique_ptr<SubsurfacePrivate> d;
};

} // namespace Lingmo::ui
