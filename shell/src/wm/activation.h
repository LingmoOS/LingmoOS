// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/focus_controller.* and base_focus_rules.*
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the separation of "which window is active" from "where input
// goes", the activation-change ordering, what happens when the active window
// is destroyed or hidden, and redirecting activation to a modal transient.
// What is dropped: aura's focused-window-inside-active-window distinction
// (Lingmo has no nested focusable widgets) and the FocusRules indirection.

#ifndef LINGMO_WM_ACTIVATION_H_
#define LINGMO_WM_ACTIVATION_H_

#include <vector>

namespace Lingmo::WM {

class Window;
class WindowManager;

class ActivationObserver
{
public:
    virtual ~ActivationObserver() = default;

    // Called before a change, while no other activation change can start.
    // Upstream: wm::ActivationChangeObserver::OnWindowActivating.
    virtual void onWindowActivating(Window *gainingActive, Window *losingActive) { }

    // Called once per change, after both windows have been told. Either may be
    // null.
    virtual void onWindowActivated(Window *gainedActive, Window *lostActive) = 0;
};

class Activation
{
public:
    explicit Activation(WindowManager *manager);
    Activation(const Activation &) = delete;
    Activation &operator=(const Activation &) = delete;
    ~Activation();

    Window *activeWindow() const { return m_activeWindow; }

    // Null clears activation. A window that cannot be activated is ignored,
    // except that a modal transient or a transient child's ancestor chain is
    // followed: see activatableFor().
    void activate(Window *window);

    // Drops activation from `window` if it holds it, moving to the next
    // activatable window. Does nothing otherwise.
    void deactivate(Window *window);

    bool canActivate(const Window *window) const;

    // The window that should actually become active when `window` is asked
    // for: its modal transient if it is blocked by one, otherwise the nearest
    // transient ancestor that can be activated. Matches
    // BaseFocusRules::GetActivatableWindow.
    Window *activatableFor(Window *window) const;

    void addObserver(ActivationObserver *observer);
    void removeObserver(ActivationObserver *observer);

    // Called by WindowManager just before `window` is destroyed.
    void onWindowRemoving(Window *window);

private:
    void setActiveWindow(Window *window);
    Window *nextActivatableWindow(Window *ignore) const;

    WindowManager *m_manager = nullptr;
    Window *m_activeWindow = nullptr;
    std::vector<ActivationObserver *> m_observers;
    bool m_updating = false;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_ACTIVATION_H_
