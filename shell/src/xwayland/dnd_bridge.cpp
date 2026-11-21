// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/base/x/x11_drag_drop_client.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the XDND messaging.
#include "xwayland/dnd_bridge.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <unistd.h>

#include "compositor/surface.h"
#include "core/logging.h"
#include "core/main_loop.h"
#include "input/drag.h"
#include "input/hit_test.h"
#include "input/seat.h"
#include "wm/window_manager.h"

namespace Lingmo::XWayland {
namespace {

// Upstream kMinXdndVersion and kMaxXdndVersion.
constexpr uint32_t kMinXdndVersion = 3;
constexpr uint32_t kMaxXdndVersion = 5;
// Upstream ends its move loop one second after the drop. The data may still be
// converting then, so the source waits as long as a selection transfer idles.
constexpr std::chrono::milliseconds kFinishTimeout(10000);
constexpr std::chrono::milliseconds kTrackInterval(16);

// wl_data_device_manager.dnd_action.
constexpr uint32_t kCopy = 1;
constexpr uint32_t kMove = 2;
constexpr uint32_t kAsk = 4;

// XdndStatus data.l[1] bits.
constexpr uint32_t kWillAcceptDrop = 1;
constexpr uint32_t kWantFurtherPosEvents = 2;

const char *const atomNames[] = {"XdndAware",      "XdndProxy",      "XdndEnter",
                                 "XdndPosition",   "XdndStatus",     "XdndLeave",
                                 "XdndDrop",       "XdndFinished",   "XdndTypeList",
                                 "XdndActionCopy", "XdndActionMove", "XdndActionAsk",
                                 "XdndActionLink", "XdndActionPrivate"};

// The Wayland drag's source, served under XdndSelection. The selection
// controller cancels a source it replaces, and that must not cancel the drag.
class ForwardingSource : public clipboard::DataSource
{
public:
    explicit ForwardingSource(std::shared_ptr<clipboard::DataSource> source)
        : m_source(std::move(source))
    {
        for (const auto &mime : m_source->formats())
            offer(mime);
    }

    void send(const std::string &mime, int fd) override
    {
        if (m_source->alive())
            m_source->send(mime, fd);
        else
            close(fd);
    }

private:
    std::shared_ptr<clipboard::DataSource> m_source;
};

} // namespace

struct DndBridge::Outgoing {
    uint64_t hover = 0;
    // The window under the drag, which every message names, and the window
    // they are sent to: its XdndProxy, or the window itself.
    xcb_window_t window = 0;
    xcb_window_t target = 0;
    uint32_t version = 0;
    std::shared_ptr<input::DragOffer> offer;
    bool entered = false;
    bool waiting = false;
    bool queued = false;
};

struct DndBridge::Incoming {
    xcb_window_t source = 0;
    uint32_t version = 0;
    std::shared_ptr<ImportedSource> data;
    std::optional<base::PointF> position;
    std::string mime;
    uint32_t action = 0;
    // The latest XdndPosition or XdndDrop timestamp.
    xcb_timestamp_t time = 0;
    bool dropped = false;
    bool finished = false;
};

// An X11 drag's data, converted from XdndSelection on demand. The drag's
// feedback flows back to the X11 source as XdndStatus and XdndFinished.
class DndBridge::ImportedSource : public clipboard::DataSource
{
public:
    base::WeakPtr<DndBridge> bridge;
    uint64_t generation = 0;
    std::map<std::string, xcb_atom_t> targets;

    void send(const std::string &mime, int fd) override
    {
        auto it = targets.find(mime);
        if (!alive() || !bridge || it == targets.end()) {
            close(fd);
            return;
        }
        // The XDND spec converts at the XdndDrop timestamp, and Qt finds the
        // drop a request belongs to by that time alone.
        auto *in = current();
        bridge->m_selection->receive(it->second, fd, generation, {}, in ? in->time : 0);
    }

    void target(const char *mime) override
    {
        if (auto *in = current()) {
            in->mime = mime ? mime : "";
            bridge->sendStatus();
        }
    }

    void action(uint32_t action) override
    {
        if (auto *in = current()) {
            in->action = action;
            bridge->sendStatus();
        }
    }

    void finished() override
    {
        if (current())
            bridge->sendFinished(true);
    }

protected:
    void cancelled() override
    {
        if (auto *in = current(); in && in->dropped)
            bridge->sendFinished(false);
    }

private:
    Incoming *current() const
    {
        return bridge && bridge->m_in && bridge->m_in->data.get() == this ? bridge->m_in.get()
                                                                          : nullptr;
    }
};

DndBridge::DndBridge(xcb_connection_t *connection, xcb_window_t root, wm::WindowManager *manager,
                     SelectionBridge::Reply reply)
    : m_connection(connection)
    , m_root(root)
    , m_manager(manager)
    , m_reply(std::move(reply))
    , m_finishTimeout(core::mainLoop())
    , m_tracker(core::mainLoop())
{
    m_finishTimeout.setSingleShot(true);
    m_finishTimeout.onTimeout = [this] { completeDrop(false); };
    m_tracker.setInterval(kTrackInterval);
    m_tracker.onTimeout = [this] { track(); };

    auto *drag = m_manager->seat()->drag();
    m_dragConnections.push_back(drag->entered.connect(
            [this](compositor::Surface *surface, const base::PointF &) { onDragEntered(surface); }));
    m_dragConnections.push_back(drag->moved.connect(
            [this](input::Timestamp, const base::PointF &) { onDragMoved(); }));
    m_dragConnections.push_back(drag->left.connect([this] { onDragLeft(); }));
    m_dragConnections.push_back(
            drag->dropped.connect([this](input::DragOffer *offer) { onDragDropped(offer); }));
    m_dragConnections.push_back(drag->ended.connect([this] { onDragEnded(); }));
}

DndBridge::~DndBridge()
{
    auto *drag = m_manager->seat()->drag();
    m_dragConnections.clear();
    if (m_in && drag->isExternal() && m_in->data && drag->source() == m_in->data)
        drag->cancel();
    m_in.reset();
    if (m_dropped && m_dropped->offer)
        m_dropped->offer->invalidate();
    m_selection.reset();
    if (m_proxy)
        xcb_destroy_window(m_connection, m_proxy);
}

void DndBridge::poll(unsigned sequence, std::function<void(void *)> callback)
{
    m_reply(sequence, [self = m_weakFactory.getWeakPtr(), callback = std::move(callback)](void *p) {
        if (self)
            callback(p);
    });
}

xcb_atom_t DndBridge::atom(const char *name) const
{
    auto it = m_atoms.find(name);
    return it == m_atoms.end() ? xcb_atom_t(XCB_ATOM_NONE) : it->second;
}

bool DndBridge::owns(xcb_window_t window) const
{
    return window && (window == m_proxy || (m_selection && window == m_selection->window()));
}

void DndBridge::start(uint8_t xfixesEvent)
{
    m_selection = std::make_unique<SelectionBridge>(m_connection, m_root, &m_controller, m_reply,
                                                    SelectionBridge::Kind::Dnd);
    m_selection->ownerChanged = [this](xcb_window_t owner) {
        onOwnerChanged(owner);
    };
    m_selection->acquired = [this] {
        if (!m_controller.source())
            return;
        m_exportOwned = true;
        m_exportReady = m_exportInterned;
        enter();
    };
    m_selection->start(xfixesEvent);
    for (auto *name : atomNames) {
        poll(xcb_intern_atom(m_connection, 0, strlen(name), name).sequence, [this, name](void *p) {
            if (p)
                m_atoms[name] = static_cast<xcb_intern_atom_reply_t *>(p)->atom;
        });
    }
    poll(xcb_get_input_focus(m_connection).sequence, [this](void *p) {
        if (!p || m_atoms.size() != std::size(atomNames))
            return;
        // Upstream uses an InputOnly override-redirect window as well, found by
        // the X11 source like any other toplevel under the pointer.
        m_proxy = xcb_generate_id(m_connection);
        const uint32_t overrideRedirect = 1;
        xcb_create_window(m_connection, 0, m_proxy, m_root, 0, 0, 1, 1, 0,
                          XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT,
                          XCB_CW_OVERRIDE_REDIRECT, &overrideRedirect);
        const uint32_t version = kMaxXdndVersion;
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_proxy, atom("XdndAware"),
                            XCB_ATOM_ATOM, 32, 1, &version);
        xcb_flush(m_connection);
        m_ready = true;
    });
}

void DndBridge::send(xcb_window_t window, xcb_window_t target, const char *type,
                     std::initializer_list<uint32_t> data)
{
    xcb_client_message_event_t e {};
    e.response_type = XCB_CLIENT_MESSAGE;
    e.format = 32;
    e.window = window;
    e.type = atom(type);
    std::copy(data.begin(), data.end(), e.data.data32);
    xcb_send_event(m_connection, 0, target, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char *>(&e));
    xcb_flush(m_connection);
}

uint32_t DndBridge::actionAtom(uint32_t action) const
{
    switch (action) {
    case kCopy:
        return atom("XdndActionCopy");
    case kMove:
        return atom("XdndActionMove");
    case kAsk:
        return atom("XdndActionAsk");
    default:
        return XCB_ATOM_NONE;
    }
}

uint32_t DndBridge::actionFromAtom(xcb_atom_t action) const
{
    if (action == atom("XdndActionMove"))
        return kMove;
    if (action == atom("XdndActionAsk"))
        return kAsk;
    // Link and Private have no Wayland counterpart; both still move data.
    return action ? kCopy : 0;
}

bool DndBridge::event(xcb_generic_event_t *generic)
{
    if (m_selection && m_selection->event(generic))
        return true;
    if ((generic->response_type & 0x7f) != XCB_CLIENT_MESSAGE || !m_ready)
        return false;
    auto *e = reinterpret_cast<xcb_client_message_event_t *>(generic);
    if (e->format != 32)
        return false;
    if (m_selection && e->window == m_selection->window()) {
        if (e->type == atom("XdndStatus"))
            onStatus(*e);
        else if (e->type == atom("XdndFinished"))
            onFinished(*e);
        return true;
    }
    if (e->window != m_proxy)
        return false;
    if (e->type == atom("XdndEnter"))
        onEnter(*e);
    else if (e->type == atom("XdndPosition"))
        onPosition(*e);
    else if (e->type == atom("XdndLeave"))
        onLeave(*e);
    else if (e->type == atom("XdndDrop"))
        onDrop(*e);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Wayland drag, X11 target.

void DndBridge::onDragEntered(compositor::Surface *surface)
{
    auto *drag = m_manager->seat()->drag();
    const xcb_window_t window = windowForSurface ? windowForSurface(surface) : 0;
    // An external drag is an X11 source: X11 targets are its own business.
    if (!m_ready || !window || drag->isExternal() || !drag->source())
        return;
    m_out = std::make_unique<Outgoing>();
    m_out->hover = ++m_hover;
    m_out->window = window;
    exportSource();
    resolveTarget(m_out->hover, window);
}

void DndBridge::exportSource()
{
    auto source = m_manager->seat()->drag()->source();
    if (!source || !source->alive() || (m_controller.source() && m_exported.lock() == source))
        return;
    // A new drag supersedes a drop still waiting for XdndFinished: the
    // selection can only serve one of them.
    if (m_dropped)
        completeDrop(false);
    const auto generation = ++m_export;
    m_exported = source;
    m_exportInterned = m_exportOwned = m_exportReady = false;
    m_exportTypes.clear();
    m_controller.setSource(std::make_shared<ForwardingSource>(source));
    for (const auto &mime : source->formats()) {
        poll(xcb_intern_atom(m_connection, 0, mime.size(), mime.data()).sequence,
             [this, generation](void *p) {
                 if (p && generation == m_export)
                     m_exportTypes.push_back(static_cast<xcb_intern_atom_reply_t *>(p)->atom);
             });
    }
    poll(xcb_get_input_focus(m_connection).sequence, [this, generation](void *p) {
        if (!p || generation != m_export)
            return;
        m_exportInterned = true;
        m_exportReady = m_exportOwned;
        enter();
    });
}

void DndBridge::resolveTarget(uint64_t hover, xcb_window_t window)
{
    // XdndProxy is only honoured when the proxy names itself, so a stale
    // property on a reused window id is not followed.
    poll(xcb_get_property(m_connection, 0, window, atom("XdndProxy"), XCB_ATOM_WINDOW, 0, 1)
                 .sequence,
         [this, hover, window](void *p) {
             auto *r = static_cast<xcb_get_property_reply_t *>(p);
             const bool proxied = r && r->type == XCB_ATOM_WINDOW && r->format == 32
                     && r->value_len == 1;
             const xcb_window_t proxy =
                     proxied ? *static_cast<xcb_window_t *>(xcb_get_property_value(r)) : 0;
             auto aware = [this, hover](xcb_window_t target) {
                 poll(xcb_get_property(m_connection, 0, target, atom("XdndAware"), XCB_ATOM_ATOM,
                                       0, 1)
                              .sequence,
                      [this, hover, target](void *p) {
                          auto *r = static_cast<xcb_get_property_reply_t *>(p);
                          if (!m_out || m_out->hover != hover || !r || r->type != XCB_ATOM_ATOM
                              || r->format != 32 || r->value_len != 1)
                              return;
                          const uint32_t version =
                                  *static_cast<uint32_t *>(xcb_get_property_value(r));
                          auto *drag = m_manager->seat()->drag();
                          if (version < kMinXdndVersion || !drag->active())
                              return;
                          m_out->target = target;
                          m_out->version = std::min(version, kMaxXdndVersion);
                          LM_DEBUG(lingmoWayland) << "XDND target" << m_out->window << "via" << target
                                             << "version" << m_out->version;
                          m_out->offer = drag->createOffer(false);
                          enter();
                      });
             };
             if (!proxy) {
                 aware(window);
                 return;
             }
             poll(xcb_get_property(m_connection, 0, proxy, atom("XdndProxy"), XCB_ATOM_WINDOW, 0,
                                   1)
                          .sequence,
                  [this, hover, window, proxy, aware](void *p) {
                      auto *r = static_cast<xcb_get_property_reply_t *>(p);
                      const bool valid = r && r->type == XCB_ATOM_WINDOW && r->format == 32
                              && r->value_len == 1
                              && *static_cast<xcb_window_t *>(xcb_get_property_value(r)) == proxy;
                      if (m_out && m_out->hover == hover)
                          aware(valid ? proxy : window);
                  });
         });
}

void DndBridge::enter()
{
    if (!m_out || !m_out->offer || m_out->entered || !m_exportReady)
        return;
    m_out->entered = true;
    const xcb_window_t source = m_selection->window();
    uint32_t flags = m_out->version << 24;
    std::array<uint32_t, 3> inline_ {};
    if (m_exportTypes.size() > inline_.size()) {
        flags |= 1;
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, source, atom("XdndTypeList"),
                            XCB_ATOM_ATOM, 32, m_exportTypes.size(), m_exportTypes.data());
    } else {
        std::copy(m_exportTypes.begin(), m_exportTypes.end(), inline_.begin());
    }
    LM_DEBUG(lingmoWayland) << "XDND enter" << m_out->window << "types" << m_exportTypes.size();
    send(m_out->window, m_out->target, "XdndEnter",
         {source, flags, inline_[0], inline_[1], inline_[2]});
    sendPosition();
}

void DndBridge::onDragMoved()
{
    if (m_out && m_out->entered)
        sendPosition();
}

void DndBridge::sendPosition()
{
    if (!m_out || !m_out->entered)
        return;
    if (m_out->waiting) {
        m_out->queued = true;
        return;
    }
    auto *drag = m_manager->seat()->drag();
    const uint32_t actions = m_out->offer ? m_out->offer->sourceActions() : 0;
    const uint32_t modifier = drag->modifierAction();
    const uint32_t action = (actions & modifier) ? modifier
            : (actions & kCopy)                  ? kCopy
            : (actions & kMove)                  ? kMove
                                                 : actions & kAsk;
    const base::Point root = toRoot(m_manager->seat()->pointerLocation().toPoint());
    m_out->waiting = true;
    m_out->queued = false;
    // The ownership time: a target converting at CurrentTime may substitute
    // a time of its own from before the selection was taken.
    send(m_out->window, m_out->target, "XdndPosition",
         {m_selection->window(), 0, uint32_t(root.x() << 16) | uint32_t(root.y() & 0xffff),
          m_selection->timestamp(), actionAtom(action)});
}

void DndBridge::onStatus(const xcb_client_message_event_t &event)
{
    if (!m_out || !m_out->entered || !m_out->offer)
        return;
    m_out->waiting = false;
    const auto &offer = m_out->offer;
    if (event.data.data32[1] & kWillAcceptDrop) {
        const uint32_t action = actionFromAtom(event.data.data32[4]);
        offer->setActions(action, action);
        offer->accept(offer->formats().empty() ? nullptr : offer->formats().front().c_str());
    } else {
        offer->accept(nullptr);
    }
    if (m_out && m_out->queued)
        sendPosition();
}

void DndBridge::onDragLeft()
{
    auto out = std::move(m_out);
    if (out && out->entered)
        send(out->window, out->target, "XdndLeave", {m_selection->window()});
}

void DndBridge::onDragDropped(input::DragOffer *offer)
{
    if (!m_out || m_out->offer.get() != offer || !m_out->entered)
        return;
    // The drag is already over, so the previous drop's export must stay.
    if (auto previous = std::move(m_dropped); previous && previous->offer)
        previous->offer->invalidate();
    m_dropped = std::move(m_out);
    LM_DEBUG(lingmoWayland) << "XDND drop" << m_dropped->window;
    send(m_dropped->window, m_dropped->target, "XdndDrop",
         {m_selection->window(), 0, m_selection->timestamp()});
    m_finishTimeout.start(kFinishTimeout);
}

void DndBridge::onFinished(const xcb_client_message_event_t &event)
{
    if (!m_dropped)
        return;
    LM_DEBUG(lingmoWayland) << "XDND finished" << event.data.data32[1];
    // Version 5 says whether the drop was taken; older targets only finish.
    completeDrop(m_dropped->version < 5 || (event.data.data32[1] & 1));
}

void DndBridge::completeDrop(bool success)
{
    m_finishTimeout.stop();
    auto dropped = std::move(m_dropped);
    if (dropped && dropped->offer) {
        if (!success || !dropped->offer->finish())
            dropped->offer->invalidate();
    }
    if (!m_manager->seat()->drag()->active())
        clearExport();
}

void DndBridge::onDragEnded()
{
    m_out.reset();
    if (!m_dropped)
        clearExport();
}

void DndBridge::clearExport()
{
    if (!m_exportReady && !m_controller.source())
        return;
    ++m_export;
    m_exported.reset();
    m_exportInterned = m_exportOwned = m_exportReady = false;
    m_exportTypes.clear();
    m_controller.setSource(nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// X11 drag, Wayland target.

void DndBridge::onOwnerChanged(xcb_window_t owner)
{
    m_owner = owner && !owns(owner) ? owner : 0;
    LM_DEBUG(lingmoWayland) << "XdndSelection owner" << owner;
    // A source that lost its selection before the drop has nothing left to give.
    if (m_in && !m_in->dropped)
        resetIncoming();
    if (m_owner && !m_tracker.isActive())
        m_tracker.start();
    track();
}

void DndBridge::track()
{
    auto *seat = m_manager->seat();
    compositor::Surface *grab = seat->pointerGrabSurface();
    const bool dragging = m_in
            || (m_owner && seat->pressedButtons() && grab && windowForSurface
                && windowForSurface(grab));
    if (!m_ready || !dragging) {
        m_tracker.stop();
        setProxyMapped(false);
        return;
    }
    base::PointF local;
    auto *surface = input::findTargetSurface(seat->compositor(), seat->pointerLocation(), &local);
    setProxyMapped(surface && !windowForSurface(surface));
}

void DndBridge::setProxyMapped(bool mapped)
{
    if (!m_proxy || mapped == m_proxyMapped)
        return;
    m_proxyMapped = mapped;
    LM_DEBUG(lingmoWayland) << "XDND proxy" << (mapped ? "mapped" : "unmapped");
    if (mapped) {
        const base::Size size = rootSize();
        const uint32_t values[] = {0, 0, uint32_t(size.width()), uint32_t(size.height()),
                                   XCB_STACK_MODE_ABOVE};
        xcb_configure_window(m_connection, m_proxy,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH
                                     | XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_STACK_MODE,
                             values);
        xcb_map_window(m_connection, m_proxy);
    } else {
        xcb_unmap_window(m_connection, m_proxy);
    }
    xcb_flush(m_connection);
}

void DndBridge::onEnter(const xcb_client_message_event_t &event)
{
    const auto *d = event.data.data32;
    const uint32_t version = d[1] >> 24;
    // Upstream discards a version it does not speak without answering.
    LM_DEBUG(lingmoWayland) << "XDND source enter" << d[0] << "version" << version;
    // Only a drag the pointer is really making: the proxy is never under the
    // pointer otherwise, but any client can name it in a message.
    auto *seat = m_manager->seat();
    if (version < kMinXdndVersion || !m_owner || !m_proxyMapped || !seat->pressedButtons())
        return;
    resetIncoming();
    m_in = std::make_unique<Incoming>();
    m_in->source = d[0];
    m_in->version = std::min(version, kMaxXdndVersion);
    if (!(d[1] & 1)) {
        std::vector<xcb_atom_t> types;
        for (int i = 2; i < 5; ++i) {
            if (d[i])
                types.push_back(d[i]);
        }
        startIncoming(types);
        return;
    }
    const xcb_window_t source = d[0];
    poll(xcb_get_property(m_connection, 0, source, atom("XdndTypeList"), XCB_ATOM_ATOM, 0, 128)
                 .sequence,
         [this, source](void *p) {
             auto *r = static_cast<xcb_get_property_reply_t *>(p);
             if (!m_in || m_in->source != source)
                 return;
             std::vector<xcb_atom_t> types;
             if (r && r->type == XCB_ATOM_ATOM && r->format == 32) {
                 auto *atoms = static_cast<xcb_atom_t *>(xcb_get_property_value(r));
                 types.assign(atoms, atoms + r->value_len);
             }
             startIncoming(types);
         });
}

void DndBridge::startIncoming(const std::vector<xcb_atom_t> &types)
{
    auto source = std::make_shared<ImportedSource>();
    source->bridge = m_weakFactory.getWeakPtr();
    source->generation = m_selection->generation();
    const xcb_window_t window = m_in->source;
    for (const xcb_atom_t type : types) {
        poll(xcb_get_atom_name(m_connection, type).sequence, [source, type](void *p) {
            if (!p)
                return;
            auto *r = static_cast<xcb_get_atom_name_reply_t *>(p);
            const std::string name(xcb_get_atom_name_name(r), xcb_get_atom_name_name_length(r));
            // The same text aliases as the clipboard bridge offers.
            if (name == "UTF8_STRING") {
                for (auto mime : {"text/plain;charset=utf-8", "text/plain"}) {
                    if (!source->supports(mime) && source->offer(mime))
                        source->targets[mime] = type;
                }
            } else if (source->offer(name)) {
                source->targets[name] = type;
            }
        });
    }
    poll(xcb_get_input_focus(m_connection).sequence, [this, source, window](void *p) {
        if (!p || !m_in || m_in->source != window || m_in->data)
            return;
        auto *seat = m_manager->seat();
        auto *drag = seat->drag();
        compositor::Surface *origin = seat->pointerGrabSurface();
        if (source->formats().empty() || !origin || !windowForSurface(origin) || drag->busy())
            return;
        m_in->data = source;
        drag->startExternal(origin, source, kCopy | kMove | kAsk);
        if (drag->isExternal() && m_in && m_in->position)
            drag->motion(input::Timestamp {}, *m_in->position);
        sendStatus();
    });
}

void DndBridge::onPosition(const xcb_client_message_event_t &event)
{
    const auto *d = event.data.data32;
    if (!m_in || d[0] != m_in->source)
        return;
    m_in->position = base::PointF(fromRoot(base::Point(int16_t(d[2] >> 16), int16_t(d[2] & 0xffff))));
    m_in->time = d[3];
    auto *drag = m_manager->seat()->drag();
    if (m_in->data && drag->isExternal() && drag->source() == m_in->data)
        drag->motion(input::Timestamp {}, *m_in->position);
    // Every position is answered, with whatever the Wayland target said last.
    sendStatus();
}

void DndBridge::sendStatus()
{
    if (!m_in || m_in->finished)
        return;
    const bool accepted = !m_in->mime.empty() && m_in->action;
    send(m_in->source, m_in->source, "XdndStatus",
         {m_proxy, accepted ? kWillAcceptDrop | kWantFurtherPosEvents : kWantFurtherPosEvents, 0,
          0, accepted ? actionAtom(m_in->action) : XCB_ATOM_NONE});
}

void DndBridge::onLeave(const xcb_client_message_event_t &event)
{
    if (m_in && event.data.data32[0] == m_in->source)
        resetIncoming();
}

void DndBridge::onDrop(const xcb_client_message_event_t &event)
{
    if (!m_in || event.data.data32[0] != m_in->source)
        return;
    m_in->dropped = true;
    m_in->time = event.data.data32[2];
    LM_DEBUG(lingmoWayland) << "XDND source drop" << m_in->source << "accepted" << m_in->mime.c_str();
    auto *drag = m_manager->seat()->drag();
    if (m_in->data && drag->isExternal() && drag->source() == m_in->data)
        drag->drop();
    else
        sendFinished(false);
}

void DndBridge::sendFinished(bool success)
{
    if (!m_in || m_in->finished)
        return;
    m_in->finished = true;
    send(m_in->source, m_in->source, "XdndFinished",
         {m_proxy, success ? 1u : 0u, success ? actionAtom(m_in->action) : XCB_ATOM_NONE});
    // The Wayland drag holds the source for as long as the target reads it.
    m_in.reset();
    track();
}

void DndBridge::resetIncoming()
{
    auto in = std::move(m_in);
    auto *drag = m_manager->seat()->drag();
    if (in && in->data && drag->isExternal() && drag->source() == in->data)
        drag->cancel();
}

} // namespace Lingmo::XWayland
