// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.sommelier file.
//
// Upstream: chromiumos/platform2/vm_tools/sommelier/sommelier.cc (selection handlers)
// Revision: aa8a57908bdcff297e229679dc3240b1b108f57e

#pragma once

#include "base/weak_ptr.h"
#include "clipboard/selection.h"
#include <functional>
#include <map>
#include <memory>
#include <xcb/xcb.h>

namespace Lingmo::XWayland {

class SelectionBridge
{
public:
    using Reply = std::function<void(unsigned, std::function<void(void *)>)>;
    // Dnd bridges XdndSelection. It exports the controller's source like the
    // others, but an X11 owner is only reported through ownerChanged: an XDND
    // source announces its types in XdndEnter, not through TARGETS.
    enum class Kind { Clipboard, Primary, Dnd };
    SelectionBridge(xcb_connection_t *connection, xcb_window_t root,
                    clipboard::SelectionController *controller, Reply reply,
                    Kind kind = Kind::Clipboard);
    SelectionBridge(const SelectionBridge &) = delete;
    SelectionBridge &operator=(const SelectionBridge &) = delete;
    ~SelectionBridge();
    void start(uint8_t eventBase);
    bool event(xcb_generic_event_t *event);

    // The window that owns the selection while it is exported.
    xcb_window_t window() const { return m_window; }

    // The current owner's selection timestamp.
    xcb_timestamp_t timestamp() const { return m_timestamp; }

    // Changes with every owner; receive() drops a stale generation.
    uint64_t generation() const { return m_generation; }

    // Converts the selection's `target` for the owner at `generation` into fd,
    // at `time`, or at the owner's timestamp when that is 0.
    void receive(xcb_atom_t target, int fd, uint64_t generation,
                 std::function<void(bool)> completion = {}, xcb_timestamp_t time = 0);

    std::function<void(xcb_window_t owner)> ownerChanged;
    // The export took the selection; timestamp() is now the ownership time.
    std::function<void()> acquired;

private:
    struct Source;
    struct Transfer;
    struct Save;
    void saveNext();
    void readSave();
    void finishSave(bool success);
    xcb_window_t createWindow();
    void changed();
    void discover(xcb_timestamp_t time);
    void readProperty(xcb_window_t window);
    void writePipe(xcb_window_t window);
    void readPipe(xcb_window_t window, xcb_atom_t property);
    void finish(xcb_window_t window, xcb_atom_t property = XCB_ATOM_NONE, bool success = false);
    void request(xcb_selection_request_event_t request);
    void notify(const xcb_selection_request_event_t &request, bool success);
    void poll(unsigned sequence, std::function<void(void *)> callback);
    xcb_atom_t atom(const char *name) const;
    Kind m_kind;
    xcb_atom_t selection() const;
    xcb_connection_t *m_connection;
    xcb_window_t m_root;
    xcb_window_t m_window = 0;
    base::WeakPtr<clipboard::SelectionController> m_controller;
    base::Subscription m_selectionChanged;
    Reply m_reply;
    uint8_t m_eventBase = 0;
    uint64_t m_generation = 0;
    xcb_timestamp_t m_timestamp = 0;
    xcb_timestamp_t m_managerTimestamp = 0;
    bool m_importing = false;
    std::map<std::string, xcb_atom_t> m_atoms;
    std::map<xcb_atom_t, std::string> m_exportTargets;
    std::shared_ptr<Source> m_source;
    std::unique_ptr<Save> m_save;
    std::map<std::pair<xcb_window_t, xcb_atom_t>, std::unique_ptr<Transfer>> m_transfers;
    base::WeakPtrFactory<SelectionBridge> m_weakFactory {this};
};

} // namespace Lingmo::XWayland
