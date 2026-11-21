// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The window manager's view of a server-side frame. Modelled on the split
// between ash::FrameViewAsh (which draws and hit-tests) and ash::WindowState
// (which owns the geometry), minus the Views hierarchy: the window manager only
// needs the frame's extents, its hit regions and its lifetime.
//
// decoration::Decoration implements this. Nothing here knows how the frame is
// drawn, and nothing in the decoration knows about Wayland.

#ifndef LINGMO_WM_WINDOW_FRAME_H_
#define LINGMO_WM_WINDOW_FRAME_H_

#include <memory>
#include <vector>

#include "base/geometry.h"
#include "input/cursor_shape.h"
#include "input/types.h"
#include "wm/types.h"

namespace Lingmo::WM {

class Window;

// What part of the frame a point falls in. The window manager routes on this;
// which button or which edge it was stays inside the frame.
enum class FrameArea {
    // Not the frame's: the client area, or outside the frame entirely.
    None,
    TitleBar,
    CaptionButton,
    ResizeBorder,
};

class WindowFrame
{
public:
    virtual ~WindowFrame() = default;

    // How much of the outer window bounds the frame occupies. Zero while the
    // window is fullscreen.
    virtual Insets extents() const = 0;

    // Everything the frame accepts pointer input in, in output coordinates.
    // Larger than the window bounds: the resize grips reach outside them.
    virtual base::Rect inputBounds() const = 0;

    virtual FrameArea areaAt(const base::PointF& location) const = 0;

    virtual input::CursorShape cursorShapeAt(const base::PointF&) const
    {
        return input::CursorShape::Default;
    }

    // Pointer events the window manager decided belong to the frame, in output
    // coordinates. Each returns true when the frame consumed the event.
    virtual bool onPointerMotion(input::Timestamp timestamp, const base::PointF& location) = 0;
    virtual bool onPointerButton(input::Timestamp timestamp, const base::PointF& location,
                                 bool pressed) = 0;
    // The pointer went somewhere the frame does not own.
    virtual void onPointerLeave() = 0;

    // True while the pointer holds a caption button, so the release belongs to
    // the frame wherever it lands.
    virtual bool isTrackingPress() const = 0;
    // Lets go of a held caption button, pointer or touch, without running it.
    virtual void cancelPress() { }

    // The title bar and the caption buttons in output coordinates, for the
    // window debug service. Empty for a frame that draws none.
    virtual base::Rect titleBarBounds() const { return base::Rect(); }
    virtual std::vector<base::Rect> captionButtonBounds() const { return {}; }

    // Touch points the window manager decided belong to the frame. A point
    // that went down here is the frame's until it is lifted; one that starts
    // a window drag is the drag's from then on.
    virtual bool onTouchDown(input::Timestamp, int32_t, const base::PointF&) { return false; }
    virtual bool onTouchMotion(input::Timestamp, int32_t, const base::PointF&) { return false; }
    virtual bool onTouchUp(input::Timestamp, int32_t) { return false; }
    virtual void onTouchCancel() { }
};

// Creates frames, and answers what a frame would cost before there is one -
// the initial configure has to size a maximized window before it is mapped.
class FrameProvider
{
public:
    virtual ~FrameProvider() = default;

    virtual std::unique_ptr<WindowFrame> createFrame(Window* window) = 0;

    virtual Insets defaultExtents() const = 0;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_FRAME_H_
