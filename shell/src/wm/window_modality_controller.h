// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/window_modality_controller.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: only window-modal transients exist; system and child
// modality have no Wayland or X11 source. Every Window is already a toplevel,
// so GetToplevelWindow disappears. The WindowManager's input sink calls in
// instead of a pre-target handler, and the bounce animation, touch
// cancellation and capture release are not ported.

#ifndef LINGMO_WM_WINDOW_MODALITY_CONTROLLER_H_
#define LINGMO_WM_WINDOW_MODALITY_CONTROLLER_H_

namespace Lingmo::WM {

class Window;
class WindowManager;

// Whether `window` blocks anything: modal, shown and with a transient parent.
bool isModalInEffect(const Window *window);

// The modal transient child blocking `window`, or null. When that child is
// blocked by a modal of its own, the innermost one is returned.
Window *modalTransient(Window *window);
const Window *modalTransient(const Window *window);

class WindowModalityController
{
public:
    explicit WindowModalityController(WindowManager *manager);
    WindowModalityController(const WindowModalityController &) = delete;
    WindowModalityController &operator=(const WindowModalityController &) = delete;

    // A pointer event aimed at `target`. Returns true when a modal transient
    // blocks it, in which case the event must not reach `target`. A press also
    // hands activation to the modal.
    bool processLocatedEvent(Window *target, bool pressed);

    // Something that decides whether `window` is in effect changed: its modal
    // flag, its transient parent, its visibility or its lifetime. A window
    // that just became modal takes activation, as upstream's property change
    // does; otherwise activation only moves off a window that is now blocked.
    void updateModality(Window *window, bool modalChanged);

private:
    WindowManager *m_manager = nullptr;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_MODALITY_CONTROLLER_H_
