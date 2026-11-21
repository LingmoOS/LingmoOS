// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desk.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: a desk owns no container. Its windows keep their
// places in the window manager's one stacking order, and a desk that is not
// active hides them with Window::setDeskHidden, as upstream hides its
// containers. Metrics, saved desks, coral and per-user state are not carried
// over.

#ifndef LINGMO_WM_DESKS_DESK_H_
#define LINGMO_WM_DESKS_DESK_H_

#include <cstdint>
#include <string>
#include <vector>

namespace Lingmo::WM {

class DesksController;
class Window;

class Desk
{
public:
    explicit Desk(DesksController *controller);
    Desk(const Desk &) = delete;
    Desk &operator=(const Desk &) = delete;
    ~Desk();

    // Stable for the desk's lifetime and never reused, as upstream's uuid.
    uint32_t id() const { return m_id; }

    // The roots of the transient trees on this desk, in the order they came.
    // Their transient descendants go wherever the root goes.
    const std::vector<Window *> &windows() const { return m_windows; }

    bool contains(const Window *window) const;

    // Empty while the desk has its default name, which the shell words itself.
    const std::string &name() const { return m_name; }
    bool isNameSetByUser() const { return m_nameSetByUser; }
    void setName(const std::string &name, bool setByUser);

    bool isActive() const { return m_active; }

    // Shows this desk's windows. With `updateWindowActivation`, the one most
    // recently used before the user left the desk becomes active again.
    void activate(bool updateWindowActivation);

    // Hides this desk's windows; the active one among them loses activation.
    void deactivate(bool updateWindowActivation);

    // Upstream MoveWindowsToDesk, for removing this desk: every window goes to
    // `target` below the windows already there, and last in the MRU order, since
    // the user is done with this desk.
    void moveWindowsToDesk(Desk *target);

    // Moves the transient tree of `window`, which must be on this desk.
    void moveWindowToDesk(Window *window, Desk *target);

private:
    friend class DesksController;

    void addWindow(Window *window);
    void removeWindow(Window *window);

    DesksController *m_controller = nullptr;
    const uint32_t m_id;
    std::vector<Window *> m_windows;
    std::string m_name;
    bool m_nameSetByUser = false;
    bool m_active = false;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_DESKS_DESK_H_
