// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_INPUT_SHORTCUTS_INHIBITION_H_
#define LINGMO_INPUT_SHORTCUTS_INHIBITION_H_

#include <map>

#include "compositor/surface_observer.h"

namespace Lingmo::Input {

class Seat;

class ShortcutsInhibitorDelegate
{
public:
    // The surface to inhibit shortcuts for; null once it is destroyed.
    virtual compositor::Surface* inhibitedSurface() const = 0;

    virtual void onInhibitorActivated() = 0;
    virtual void onInhibitorDeactivated() = 0;

protected:
    virtual ~ShortcutsInhibitorDelegate() = default;
};

// Surfaces that asked for the compositor's keyboard shortcuts - a remote
// desktop, a virtual machine, an X11 client grabbing the keyboard. While one
// has keyboard focus, the accelerators it may take go to it instead.
//
// Exo only marks the surface, and Ash then lets its window consume system keys.
// Here the inhibition is also active or inactive with keyboard focus, and the
// user can take it back until the next click on the window.
class ShortcutsInhibition : public compositor::SurfaceObserver
{
public:
    explicit ShortcutsInhibition(Seat* seat);
    ShortcutsInhibition(const ShortcutsInhibition&) = delete;
    ShortcutsInhibition& operator=(const ShortcutsInhibition&) = delete;
    ~ShortcutsInhibition() override;

    // False when the surface already has an inhibitor.
    bool add(ShortcutsInhibitorDelegate* delegate);

    // The delegate is being destroyed.
    void remove(ShortcutsInhibitorDelegate* delegate);

    bool isActive() const { return m_active; }

    // Keyboard focus changed.
    void update();

    // Deactivates the inhibitor until a click on its window permits it again.
    // Returns whether one was active.
    bool breakByUserAction();

    void onPointerPressed(compositor::Surface* surface);

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface* surface) override;

private:
    Seat* m_seat;
    std::map<compositor::Surface*, ShortcutsInhibitorDelegate*> m_inhibitors;
    ShortcutsInhibitorDelegate* m_active = nullptr;
    bool m_permitted = true;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_SHORTCUTS_INHIBITION_H_
