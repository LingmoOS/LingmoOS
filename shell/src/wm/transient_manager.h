// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ui/wm/core/transient_window_manager.*
// (revision 7acd40ddefff49c9d36144999aafdbb674dbd07a), reduced to the
// parent/child relationship itself. WindowManager applies popup stacking;
// modal blocking lives in window_modality_controller.h.

#ifndef LINGMO_WM_TRANSIENT_MANAGER_H_
#define LINGMO_WM_TRANSIENT_MANAGER_H_

namespace Lingmo::WM {

class Window;

class TransientManager
{
public:
    TransientManager() = default;
    TransientManager(const TransientManager &) = delete;
    TransientManager &operator=(const TransientManager &) = delete;

    // Null `parent` clears the relationship. Returns false, changing nothing,
    // if `child` is null, if `parent` is `child`, or if the link would close a
    // cycle.
    bool setTransientParent(Window *child, Window *parent);

    // Detaches `window` from its parent and orphans its children. Lingmo does
    // not destroy transient children with their parent: the client owns their
    // lifetime, and xdg-shell must not have surfaces disappear behind its back.
    void windowDestroying(Window *window);

private:
    static bool wouldCreateCycle(const Window *child, const Window *parent);
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_TRANSIENT_MANAGER_H_
