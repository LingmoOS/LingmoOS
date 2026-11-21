// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/base/x/x11_drag_drop_client.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the XDND messaging. Chromium is an X11 client; here the
// window manager speaks XDND on behalf of Wayland drags and targets.
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <xcb/xcb.h>


#include "base/geometry.h"
#include "base/timer.h"
#include "base/weak_ptr.h"

#include "base/signal.h"
#include "clipboard/selection.h"
#include "xwayland/selection_bridge.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {
class DragOffer;
}

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::XWayland {

// Bridges drag-and-drop between X11 clients and Wayland.
//
// A Wayland drag over an X11 window: this is the XDND source. It owns
// XdndSelection through a SelectionBridge that serves the Wayland source, and
// drives an ordinary DragOffer from the target's XdndStatus replies.
//
// An X11 drag over a Wayland surface: the X11 client keeps its implicit grab
// and goes on seeing the pointer, so an XdndAware proxy window is mapped over
// the output whenever the pointer is off every X11 window. The source's
// XdndEnter starts an external Wayland drag, its XdndPosition moves it and its
// XdndDrop drops it.
class DndBridge
{
public:
    DndBridge(xcb_connection_t *connection, xcb_window_t root, wm::WindowManager *manager,
              SelectionBridge::Reply reply);
    DndBridge(const DndBridge &) = delete;
    DndBridge &operator=(const DndBridge &) = delete;
    ~DndBridge();
    void start(uint8_t xfixesEvent);
    bool event(xcb_generic_event_t *event);

    // The windows this bridge created; the window manager must not manage them.
    bool owns(xcb_window_t window) const;

    // The X11 window behind a surface, or 0 for a Wayland surface.
    std::function<xcb_window_t(compositor::Surface *)> windowForSurface;
    // Between logical positions and X11 root coordinates, and the X11 screen
    // size; the XWM owns the scale between them.
    std::function<base::Point(const base::Point &)> toRoot;
    std::function<base::Point(const base::Point &)> fromRoot;
    std::function<base::Size()> rootSize;

private:
    struct Outgoing;
    struct Incoming;
    class ImportedSource;

    void poll(unsigned sequence, std::function<void(void *)> callback);
    xcb_atom_t atom(const char *name) const;
    void send(xcb_window_t window, xcb_window_t target, const char *type,
              std::initializer_list<uint32_t> data);
    uint32_t actionAtom(uint32_t action) const;
    uint32_t actionFromAtom(xcb_atom_t atom) const;

    // Wayland drag, X11 target.
    void onDragEntered(compositor::Surface *surface);
    void onDragMoved();
    void onDragLeft();
    void onDragDropped(input::DragOffer *offer);
    void onDragEnded();
    void exportSource();
    void resolveTarget(uint64_t hover, xcb_window_t window);
    void enter();
    void sendPosition();
    void onStatus(const xcb_client_message_event_t &event);
    void onFinished(const xcb_client_message_event_t &event);
    void completeDrop(bool success);
    void clearExport();

    // X11 drag, Wayland target.
    void onOwnerChanged(xcb_window_t owner);
    void track();
    void setProxyMapped(bool mapped);
    void onEnter(const xcb_client_message_event_t &event);
    void startIncoming(const std::vector<xcb_atom_t> &types);
    void onPosition(const xcb_client_message_event_t &event);
    void onLeave(const xcb_client_message_event_t &event);
    void onDrop(const xcb_client_message_event_t &event);
    void sendStatus();
    void sendFinished(bool success);
    void resetIncoming();

    xcb_connection_t *m_connection;
    xcb_window_t m_root;
    wm::WindowManager *m_manager;
    SelectionBridge::Reply m_reply;
    std::map<std::string, xcb_atom_t> m_atoms;
    clipboard::SelectionController m_controller;
    std::unique_ptr<SelectionBridge> m_selection;
    xcb_window_t m_proxy = 0;
    bool m_proxyMapped = false;
    bool m_ready = false;

    std::unique_ptr<Outgoing> m_out;
    std::unique_ptr<Outgoing> m_dropped;
    uint64_t m_hover = 0;
    uint64_t m_export = 0;
    std::weak_ptr<clipboard::DataSource> m_exported;
    // Both must hold before XdndEnter: every type interned, and the ownership
    // timestamp the messages carry known.
    bool m_exportInterned = false;
    bool m_exportOwned = false;
    bool m_exportReady = false;
    std::vector<xcb_atom_t> m_exportTypes;
    base::Timer m_finishTimeout;

    xcb_window_t m_owner = 0;
    std::unique_ptr<Incoming> m_in;
    base::Timer m_tracker;
    std::vector<base::Subscription> m_dragConnections;
    base::WeakPtrFactory<DndBridge> m_weakFactory {this};
};

} // namespace Lingmo::XWayland
