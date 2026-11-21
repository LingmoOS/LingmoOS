// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <optional>
#include <string>

#include "base/geometry.h"
#include "base/thread_pool.h"
#include "shell/shell.h"
#include "ui/geometry.h"
#include "ui/view.h"

namespace Lingmo::Shell {

class CaptureToolbar;
class ScreenCapture;
class ShellWindow;

// The capture interface on one output: a Capture window over the whole output,
// never in what is captured, with the dim around the selection, its frame and
// handles, the window a click would take, the crosshair's selection and its
// size, the countdown, and on the toolbar's output the toolbar.
class CaptureOverlay
{
public:
    // What the pointer is drawn as.
    enum class Shape {
        Arrow,
        Crosshair,
        Camera,
        OpenHand,
        ClosedHand,
        ResizeHorizontal,
        ResizeVertical,
        ResizeDiagonal,
        ResizeAntiDiagonal,
    };

    class View;

    CaptureOverlay(ScreenCapture *capture, Shell *shell, const Shell::Output &output);
    ~CaptureOverlay();

    const std::string &outputName() const { return m_output.name; }
    base::Rect bounds() const { return m_output.bounds; }
    void setOutput(const Shell::Output &output);

    // Shown over the output; `focus` takes the keyboard too.
    void present(bool focus);
    void dismiss();
    // While the timer counts down, input goes to what is under it.
    void setPassThrough(bool passThrough);
    // Takes the keyboard back, as after the Options menu.
    void activate();
    // The capture's state changed.
    void update();

    ShellWindow *window() const { return m_window.get(); }
    CaptureToolbar *toolbar() const;
    Shape shape() const { return m_shape; }
    // The toolbar's rect in the overlay's coordinates, empty while it is not
    // shown here.
    ui::RectF toolbarRect() const;
    // The crosshair's position or size by the pointer; empty while hidden.
    std::string coordinates() const;

private:
    void updateCursor();
    void updateGlass();

    ScreenCapture *m_capture;
    Shell *m_shell;
    Shell::Output m_output;
    std::unique_ptr<ShellWindow> m_window;
    View *m_view = nullptr;
    Shape m_shape = Shape::Arrow;
    std::optional<Shape> m_appliedShape;
    double m_appliedScale = 0;
    int m_cursorSize = 24;
    base::Job m_cursorSizeLoad;
    ui::RectF m_glassRect;
    bool m_glassDark = false;
};

} // namespace Lingmo::Shell

