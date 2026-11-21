// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/drag.h"
#include "ui/event.h"

namespace Lingmo::ui {

class EmbeddedWindow;

// What embedded windows ask of the process that hosts them, for what a
// Wayland client asks of the compositor: popups placed by xdg_positioner's
// rules, activation, text-input-v3, the blur behind a window and drags.
// One per process, see EmbeddedWindow::setHost(); a host draws the glass of
// popups (the popup glass, tint included) into their blurRect(), masked
// by blurShape(), so menus paint no body of their own while one is set.
// Every call comes on the UI thread, from inside toolkit calls, and none may
// destroy the window it names. Unimplemented calls refuse or ignore.
class EmbeddedHost
{
public:
    virtual ~EmbeddedHost() = default;

    // A popup of an embedded window maps, at popup->size(): place it against
    // popup->popupParent() by popupPlacement() (logical, the parent's
    // coordinates) as an xdg_popup, stack it above the parent, give it the
    // scale of the output it lands on and tell it where it went with
    // setPopupGeometry(). A grabbing popup (isGrabbingPopup()) holds an
    // xdg_popup grab: a press outside the chain of popups of its root
    // window dismisses them, innermost first, with closePopups() on the
    // root. Keys stay with the root window, which passes them to its
    // innermost grabbing popup itself.
    virtual void popupShown(EmbeddedWindow*) {}
    // A shown popup's size or popupPlacement() changed, as
    // xdg_popup.reposition: place it again.
    virtual void popupRepositioned(EmbeddedWindow*) {}
    // Off screen now, hidden or being destroyed: neither render it nor send
    // it input any more.
    virtual void popupHidden(EmbeddedWindow*) {}

    // Window::activate(): the window asks for focus and to be raised.
    virtual void activationRequested(EmbeddedWindow*) {}

    // text-input-v3 for the window's focused text view: a state enables it
    // (or updates it) with the surrounding text, cursor and anchor, content
    // purpose and cursor rectangle (window coordinates); null disables it.
    // Called only on a change, and only while the window has the keyboard.
    // The change came from the input method's last event when
    // `fromInputMethod`: text-input-v3's change cause. The input method's
    // events go back through EmbeddedWindow::textInput().
    virtual void textInputChanged(EmbeddedWindow*, const TextInputState*, bool /*fromInputMethod*/) {}

    // The blur behind the window changed: EmbeddedWindow::blurRect(),
    // blurRadius() and blurShape(), as lingmo_surface_blur_v1 takes them.
    virtual void blurChanged(EmbeddedWindow*) {}

    // A view starts a drag from a press in the window: the host's drag
    // controller takes the pointer, draws the icon and offers the data to
    // targets, and calls source.onFinished once. False refuses the drag.
    virtual bool startDrag(EmbeddedWindow*, DragSource) { return false; }
};

} // namespace Lingmo::ui
