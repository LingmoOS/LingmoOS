// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.sommelier file.
//
// Upstream: chromiumos/platform2/vm_tools/sommelier/sommelier.cc (selection handlers)
// Revision: aa8a57908bdcff297e229679dc3240b1b108f57e

#include "selection_bridge.h"

#include "base/strings.h"
#include "base/timer.h"
#include "core/main_loop.h"
#include "xwayland/fd_watch.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <xcb/xfixes.h>

#include "clipboard/stored_source.h"

namespace Lingmo::XWayland {
namespace {
constexpr uint32_t chunkSize = 64 * 1024;
constexpr size_t transferLimit = 64;
// How long a transfer may stall before it is given up.
constexpr std::chrono::milliseconds kTransferTimeout(10000);

// A stalled transfer's end, started.
std::unique_ptr<base::Timer> transferTimeout(std::function<void()> expired)
{
    auto timer = std::make_unique<base::Timer>(core::mainLoop());
    timer->setSingleShot(true);
    timer->onTimeout = std::move(expired);
    timer->start(kTransferTimeout);
    return timer;
}

} // namespace

struct SelectionBridge::Save {
    xcb_selection_request_event_t request {};
    std::shared_ptr<Source> source;
    std::shared_ptr<clipboard::StoredSource> stored = std::make_shared<clipboard::StoredSource>();
    std::vector<xcb_atom_t> requested;
    std::vector<std::string> formats;
    size_t index = 0;
    size_t total = 0;
    int fd = -1;
    std::string data;
    bool installing = false, received = false;
    std::unique_ptr<FdWatch> notifier;
    base::Timer timeout {core::mainLoop()};

    ~Save()
    {
        notifier.reset();
        if (fd >= 0)
            close(fd);
    }
};

struct SelectionBridge::Source : clipboard::DataSource {
    base::WeakPtr<SelectionBridge> bridge;
    uint64_t generation;
    std::map<std::string, xcb_atom_t> targets;

    void send(const std::string &mime, int fd) override
    {
        auto it = targets.find(mime);
        if (!alive() || !bridge || it == targets.end()) {
            close(fd);
            return;
        }
        bridge->receive(it->second, fd, generation);
    }
};

struct SelectionBridge::Transfer {
    std::function<void(bool)> completion;
    int fd = -1;
    bool incoming = false;
    bool incremental = false;
    bool pending = false;
    bool eof = false;
    bool notified = false;
    bool more = false;
    uint32_t propertyOffset = 0;
    uint64_t generation = 0;
    std::string data;
    size_t offset = 0;
    xcb_atom_t target = 0;
    xcb_selection_request_event_t request {};
    std::unique_ptr<FdWatch> notifier;
    std::unique_ptr<base::Timer> timer;

    ~Transfer()
    {
        notifier.reset();
        if (fd >= 0)
            close(fd);
    }
};

SelectionBridge::SelectionBridge(xcb_connection_t *connection, xcb_window_t root,
                                 clipboard::SelectionController *controller, Reply reply,
                                 Kind kind)
    : m_kind(kind)
    , m_connection(connection)
    , m_root(root)
    , m_controller(controller ? controller->weakPtr() : nullptr)
    , m_reply(std::move(reply))
{
}

SelectionBridge::~SelectionBridge()
{
    finishSave(false);
    if (m_controller) {
        m_selectionChanged.reset();
        m_controller->sourceDestroyed(m_source.get());
    }
    while (!m_transfers.empty()) {
        const auto key = m_transfers.begin()->first;
        finish(key.first, key.second);
    }
    if (m_window)
        xcb_destroy_window(m_connection, m_window);
}

void SelectionBridge::poll(unsigned sequence, std::function<void(void *)> callback)
{
    m_reply(sequence,
            [self = m_weakFactory.getWeakPtr(), callback = std::move(callback)](void *p) {
                if (self)
                    callback(p);
            });
    xcb_flush(m_connection);
}

xcb_atom_t SelectionBridge::atom(const char *name) const
{
    auto it = m_atoms.find(name);
    return it == m_atoms.end() ? xcb_atom_t(XCB_ATOM_NONE) : it->second;
}

xcb_atom_t SelectionBridge::selection() const
{
    switch (m_kind) {
    case Kind::Primary:
        return XCB_ATOM_PRIMARY;
    case Kind::Dnd:
        return atom("XdndSelection");
    case Kind::Clipboard:
        break;
    }
    return atom("CLIPBOARD");
}

xcb_window_t SelectionBridge::createWindow()
{
    auto id = xcb_generate_id(m_connection);
    uint32_t mask = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_create_window(m_connection, 0, id, m_root, 0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY,
                      XCB_COPY_FROM_PARENT, XCB_CW_EVENT_MASK, &mask);
    return id;
}

void SelectionBridge::start(uint8_t eventBase)
{
    m_eventBase = eventBase;
    static constexpr const char *names[] = {"CLIPBOARD",     "CLIPBOARD_MANAGER",
                                            "TARGETS",       "TIMESTAMP",
                                            "UTF8_STRING",   "TEXT",
                                            "INCR",          "_LINGMO_SELECTION",
                                            "SAVE_TARGETS",  "XdndSelection"};
    for (auto name : names) {
        poll(xcb_intern_atom(m_connection, 0, strlen(name), name).sequence, [this, name](void *p) {
            if (p)
                m_atoms[name] = static_cast<xcb_intern_atom_reply_t *>(p)->atom;
        });
    }
    poll(xcb_get_input_focus(m_connection).sequence, [this](void *p) {
        if (!p || m_atoms.size() != std::size(names) || !m_controller)
            return;
        m_window = createWindow();
        xcb_xfixes_select_selection_input(
                m_connection, m_window, selection(),
                XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER
                        | XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_WINDOW_DESTROY
                        | XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_CLIENT_CLOSE);
        if (m_kind == Kind::Clipboard) {
            xcb_xfixes_select_selection_input(m_connection, m_window, atom("CLIPBOARD_MANAGER"),
                                              XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER);
            xcb_set_selection_owner(m_connection, m_window, atom("CLIPBOARD_MANAGER"),
                                    XCB_CURRENT_TIME);
        }
        m_selectionChanged = m_controller->selectionChanged.connect([this] {
            changed();
        });
        changed();
        xcb_flush(m_connection);
    });
}

void SelectionBridge::changed()
{
    if (m_importing || !m_window || !m_controller)
        return;
    if (m_save && !m_save->installing)
        finishSave(false);
    ++m_generation;
    m_source.reset();
    m_exportTargets.clear();
    auto source = m_controller->source();
    const auto generation = m_generation;
    if (!source) {
        // Do not clear a replacement owned by an X client.
        poll(xcb_get_selection_owner(m_connection, selection()).sequence,
             [this, generation](void *p) {
                 if (generation == m_generation && p
                     && static_cast<xcb_get_selection_owner_reply_t *>(p)->owner == m_window)
                     xcb_set_selection_owner(m_connection, XCB_NONE, selection(), m_timestamp);
             });
        return;
    }
    for (const auto &mime : source->formats()) {
        poll(xcb_intern_atom(m_connection, 0, mime.size(), mime.data()).sequence,
             [this, generation, mime](void *p) {
                 if (generation == m_generation && p)
                     m_exportTargets[static_cast<xcb_intern_atom_reply_t *>(p)->atom] = mime;
             });
    }
    poll(xcb_get_input_focus(m_connection).sequence, [this, generation, source](void *p) {
        if (!p || generation != m_generation || !source->alive())
            return;
        for (auto mime : {"text/plain;charset=utf-8", "text/plain"}) {
            if (source->supports(mime)) {
                m_exportTargets[atom("UTF8_STRING")] = mime;
                m_exportTargets[atom("TEXT")] = mime;
                break;
            }
        }
        xcb_set_selection_owner(m_connection, m_window, selection(), XCB_CURRENT_TIME);
        xcb_flush(m_connection);
        if (m_save && m_save->installing)
            finishSave(true);
    });
}

void SelectionBridge::discover(xcb_timestamp_t time)
{
    const auto generation = ++m_generation;
    if (m_source && m_controller) {
        m_importing = true;
        m_controller->sourceDestroyed(m_source.get());
        m_importing = false;
    }
    m_source.reset();
    m_exportTargets.clear();
    if (m_transfers.size() >= transferLimit)
        return;
    auto window = createWindow();
    auto transfer = std::make_unique<Transfer>();
    transfer->incoming = true;
    transfer->generation = m_generation;
    transfer->target = atom("TARGETS");
    transfer->timer = transferTimeout([this, window] { finish(window); });
    m_transfers[{window, XCB_ATOM_NONE}] = std::move(transfer);
    // A unique requestor prevents delayed conversions from matching a newer owner.
    xcb_convert_selection(m_connection, window, selection(), atom("TARGETS"),
                          atom("_LINGMO_SELECTION"), time);
    auto source = std::make_shared<Source>();
    source->bridge = m_weakFactory.getWeakPtr();
    source->generation = generation;
    m_source = source;
    xcb_flush(m_connection);
}

void SelectionBridge::receive(xcb_atom_t target, int fd, uint64_t generation,
                              std::function<void(bool)> completion, xcb_timestamp_t time)
{
    if (generation != m_generation || m_transfers.size() >= transferLimit
        || fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
        close(fd);
        return;
    }
    auto window = createWindow();
    auto transfer = std::make_unique<Transfer>();
    transfer->fd = fd;
    transfer->completion = std::move(completion);
    transfer->incoming = true;
    transfer->generation = m_generation;
    transfer->target = target;
    transfer->notifier = std::make_unique<FdWatch>(fd, EPOLLOUT, [this, window] {
        writePipe(window);
    });
    transfer->timer = transferTimeout([this, window] { finish(window); });
    m_transfers[{window, XCB_ATOM_NONE}] = std::move(transfer);
    xcb_convert_selection(m_connection, window, selection(), target, atom("_LINGMO_SELECTION"),
                          time ? time : m_timestamp);
    xcb_flush(m_connection);
}

void SelectionBridge::finish(xcb_window_t window, xcb_atom_t property, bool success)
{
    auto it = m_transfers.find({window, property});
    if (it == m_transfers.end())
        return;
    auto transfer = std::move(it->second);
    m_transfers.erase(it);
    if (transfer->completion)
        transfer->completion(success);
    if (transfer->incoming)
        xcb_destroy_window(m_connection, window);
    else if (!transfer->notified)
        notify(transfer->request, false);
    xcb_flush(m_connection);
}

void SelectionBridge::readProperty(xcb_window_t window)
{
    auto it = m_transfers.find({window, XCB_ATOM_NONE});
    if (it == m_transfers.end() || it->second->pending || !it->second->data.empty())
        return;
    it->second->pending = true;
    const auto generation = it->second->generation;
    const auto propertyOffset = it->second->propertyOffset;
    poll(xcb_get_property(m_connection, 0, window, atom("_LINGMO_SELECTION"), XCB_ATOM_ANY,
                          propertyOffset, chunkSize / 4)
                 .sequence,
         [this, window, generation](void *p) {
             auto it = m_transfers.find({window, XCB_ATOM_NONE});
             if (it == m_transfers.end())
                 return;
             auto &t = *it->second;
             t.pending = false;
             auto *r = static_cast<xcb_get_property_reply_t *>(p);
             if (!r || r->type == XCB_ATOM_NONE) {
                 finish(window);
                 return;
             }
             t.timer->start(kTransferTimeout);
             if (t.target == atom("TARGETS")) {
                 auto source = m_source;
                 if (generation != m_generation || !source || r->type != XCB_ATOM_ATOM
                     || r->format != 32 || r->value_len > 128 || r->bytes_after) {
                     finish(window);
                     return;
                 }
                 auto *targets = static_cast<xcb_atom_t *>(xcb_get_property_value(r));
                 for (uint32_t i = 0; i < r->value_len; ++i) {
                     auto target = targets[i];
                     if (target == atom("TARGETS") || target == atom("TIMESTAMP")
                         || target == atom("INCR"))
                         continue;
                     poll(xcb_get_atom_name(m_connection, target).sequence,
                          [this, source, target](void *p) {
                              if (!p || source != m_source)
                                  return;
                              auto *r = static_cast<xcb_get_atom_name_reply_t *>(p);
                              std::string name(xcb_get_atom_name_name(r),
                                               xcb_get_atom_name_name_length(r));
                              if (name == "MULTIPLE" || name == "SAVE_TARGETS" || name == "DELETE"
                                  || name == "INSERT_SELECTION" || name == "INSERT_PROPERTY")
                                  return;
                              if (source->offer(name))
                                  source->targets[name] = target;
                              if (target == atom("UTF8_STRING") || target == atom("TEXT")) {
                                  for (auto mime : {"text/plain;charset=utf-8", "text/plain"}) {
                                      if (target == atom("UTF8_STRING")
                                          || !source->supports(mime)) {
                                          source->offer(mime);
                                          source->targets[mime] = target;
                                      }
                                  }
                              }
                          });
                 }
                 poll(xcb_get_input_focus(m_connection).sequence, [this, source](void *p) {
                     if (!p || source != m_source || !m_controller)
                         return;
                     m_importing = true;
                     m_controller->setSource(source);
                     m_importing = false;
                     saveNext();
                 });
                 finish(window);
                 return;
             }
             if (r->type == atom("INCR") && !t.incremental && r->format == 32
                 && r->value_len == 1) {
                 t.incremental = true;
                 xcb_delete_property(m_connection, window, atom("_LINGMO_SELECTION"));
                 return;
             }
             if (r->format != 8) {
                 finish(window);
                 return;
             }
             t.data.assign(static_cast<const char *>(xcb_get_property_value(r)),
                           size_t(xcb_get_property_value_length(r)));
             t.offset = 0;
             t.more = r->bytes_after != 0;
             t.propertyOffset += (t.data.size() + 3) / 4;
             if (t.target == atom("TEXT") && r->type == XCB_ATOM_STRING)
                 t.data = base::strings::latin1ToUtf8(t.data);
             if (t.incremental && t.data.empty()) {
                 finish(window, XCB_ATOM_NONE, true);
                 return;
             }
             writePipe(window);
         });
}

void SelectionBridge::writePipe(xcb_window_t window)
{
    auto it = m_transfers.find({window, XCB_ATOM_NONE});
    if (it == m_transfers.end())
        return;
    auto &t = *it->second;
    while (t.offset < t.data.size()) {
        auto count = write(t.fd, t.data.data() + t.offset, t.data.size() - t.offset);
        if (count < 0 && errno == EINTR)
            continue;
        if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            t.notifier->setEnabled(true);
            return;
        }
        if (count <= 0) {
            finish(window);
            return;
        }
        t.offset += size_t(count);
        t.timer->start(kTransferTimeout);
    }
    t.notifier->setEnabled(false);
    t.data.clear();
    if (t.more) {
        readProperty(window);
        return;
    }
    t.propertyOffset = 0;
    if (!t.incremental) {
        finish(window, XCB_ATOM_NONE, true);
        return;
    }
    xcb_delete_property(m_connection, window, atom("_LINGMO_SELECTION"));
    xcb_flush(m_connection);
}

void SelectionBridge::notify(const xcb_selection_request_event_t &r, bool success)
{
    xcb_selection_notify_event_t e {};
    e.response_type = XCB_SELECTION_NOTIFY;
    e.requestor = r.requestor;
    e.selection = r.selection;
    e.target = r.target;
    e.time = r.time;
    e.property = success ? r.property : xcb_atom_t(XCB_ATOM_NONE);
    xcb_send_event(m_connection, 0, r.requestor, 0, reinterpret_cast<const char *>(&e));
    xcb_flush(m_connection);
}

void SelectionBridge::finishSave(bool success)
{
    auto saved = std::move(m_save);
    if (saved)
        notify(saved->request, success);
}

void SelectionBridge::saveNext()
{
    if (!m_save || m_save->fd >= 0 || m_save->installing)
        return;
    auto &save = *m_save;
    if (!save.source) {
        if (!m_source || !m_source->used() || !m_controller || m_controller->source() != m_source)
            return;
        save.source = m_source;
        std::map<xcb_atom_t, std::string> targets;
        for (const auto &[mime, target] : save.source->targets) {
            if (save.requested.empty()
                || std::find(save.requested.begin(), save.requested.end(), target)
                        != save.requested.end())
                targets[target] = mime;
        }
        for (const auto &[target, mime] : targets)
            save.formats.push_back(mime);
    }
    if (!save.source->alive()) {
        finishSave(false);
        return;
    }
    if (save.index == save.formats.size()) {
        if (save.formats.empty()) {
            finishSave(false);
            return;
        }
        save.installing = true;
        m_controller->setSource(save.stored);
        return;
    }
    int fds[2];
    if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) < 0) {
        finishSave(false);
        return;
    }
    save.fd = fds[0];
    save.notifier = std::make_unique<FdWatch>(fds[0], EPOLLIN, [this] { readSave(); });
    save.notifier->setEnabled(true);
    save.received = false;
    receive(save.source->targets.at(save.formats[save.index]), fds[1], save.source->generation,
            [this, source = save.source, index = save.index](bool success) {
                if (m_save && m_save->source == source && m_save->index == index)
                    m_save->received = success;
            });
}

void SelectionBridge::readSave()
{
    if (!m_save || m_save->fd < 0)
        return;
    auto &save = *m_save;
    char data[65536];
    auto count = read(save.fd, data, sizeof(data));
    if (count < 0 && (errno == EINTR || errno == EAGAIN))
        return;
    if (count < 0 || save.total + std::max<ssize_t>(0, count) > 64 * 1024 * 1024) {
        finishSave(false);
        return;
    }
    save.timeout.start(kTransferTimeout);
    if (count > 0) {
        save.data.append(data, size_t(count));
        save.total += count;
        return;
    }
    if (!save.received) {
        finishSave(false);
        return;
    }
    const auto target = save.source->targets.at(save.formats[save.index]);
    for (const auto &[mime, atom] : save.source->targets) {
        if (atom == target) {
            save.stored->offer(mime);
            save.stored->payloads[mime] = save.data;
        }
    }
    save.data.clear();
    save.notifier.reset();
    close(std::exchange(save.fd, -1));
    ++save.index;
    saveNext();
}

void SelectionBridge::request(xcb_selection_request_event_t r)
{
    if (!r.property)
        r.property = r.target;
    if (m_kind == Kind::Clipboard && r.selection == atom("CLIPBOARD_MANAGER")) {
        if (r.time && int32_t(r.time - m_managerTimestamp) < 0) {
            notify(r, false);
            return;
        }
        if (r.target == atom("TARGETS")) {
            const uint32_t targets[] = {atom("TARGETS"), atom("TIMESTAMP"), atom("SAVE_TARGETS")};
            xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, r.requestor, r.property,
                                XCB_ATOM_ATOM, 32, std::size(targets), targets);
            notify(r, true);
        } else if (r.target == atom("TIMESTAMP")) {
            xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, r.requestor, r.property,
                                XCB_ATOM_INTEGER, 32, 1, &m_managerTimestamp);
            notify(r, true);
        } else if (r.target == atom("SAVE_TARGETS") && !m_save) {
            m_save = std::make_unique<Save>();
            m_save->request = r;
            m_save->timeout.setSingleShot(true);
            m_save->timeout.onTimeout = [this] { finishSave(false); };
            m_save->timeout.start(kTransferTimeout);
            poll(xcb_get_property(m_connection, 0, r.requestor, r.property, XCB_ATOM_ATOM, 0, 128)
                         .sequence,
                 [this, r](void *p) {
                     if (!m_save || m_save->request.requestor != r.requestor)
                         return;
                     auto *property = static_cast<xcb_get_property_reply_t *>(p);
                     if (!property || property->bytes_after) {
                         finishSave(false);
                         return;
                     }
                     if (property->type == XCB_ATOM_ATOM && property->format == 32) {
                         auto *atoms = static_cast<xcb_atom_t *>(xcb_get_property_value(property));
                         m_save->requested.assign(atoms, atoms + property->value_len);
                     }
                     saveNext();
                 });
        } else
            notify(r, false);
        return;
    }
    if (!m_controller || r.selection != selection()
        || (r.time && int32_t(r.time - m_timestamp) < 0)) {
        notify(r, false);
        return;
    }
    auto source = m_controller->source();
    if (!source || source == m_source) {
        notify(r, false);
        return;
    }
    if (r.target == atom("TARGETS")) {
        std::vector<xcb_atom_t> targets {atom("TARGETS"), atom("TIMESTAMP")};
        for (auto &[target, mime] : m_exportTargets)
            targets.push_back(target);
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, r.requestor, r.property,
                            XCB_ATOM_ATOM, 32, targets.size(), targets.data());
        notify(r, true);
    } else if (r.target == atom("TIMESTAMP")) {
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, r.requestor, r.property,
                            XCB_ATOM_INTEGER, 32, 1, &m_timestamp);
        notify(r, true);
    } else {
        auto target = m_exportTargets.find(r.target);
        if (target == m_exportTargets.end() || m_transfers.size() >= transferLimit
            || m_transfers.contains({r.requestor, r.property})) {
            notify(r, false);
            return;
        }
        int fds[2];
        if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) < 0) {
            notify(r, false);
            return;
        }
        auto t = std::make_unique<Transfer>();
        t->fd = fds[0];
        t->request = r;
        t->target = r.target == atom("TEXT") ? atom("UTF8_STRING") : r.target;
        t->notifier = std::make_unique<FdWatch>(fds[0], EPOLLIN, [this, r] {
            readPipe(r.requestor, r.property);
        });
        t->timer = transferTimeout([this, r] { finish(r.requestor, r.property); });
        m_transfers[{r.requestor, r.property}] = std::move(t);
        // Preserve this connection's existing subscriptions on managed windows.
        poll(xcb_get_window_attributes(m_connection, r.requestor).sequence, [this, r](void *p) {
            if (!m_transfers.contains({r.requestor, r.property}))
                return;
            if (!p) {
                finish(r.requestor, r.property);
                return;
            }
            auto mask = static_cast<xcb_get_window_attributes_reply_t *>(p)->your_event_mask
                    | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
            xcb_change_window_attributes(m_connection, r.requestor, XCB_CW_EVENT_MASK, &mask);
            m_transfers.at({r.requestor, r.property})->notifier->setEnabled(true);
            xcb_flush(m_connection);
        });
        source->send(target->second, fds[1]);
    }
}

void SelectionBridge::readPipe(xcb_window_t window, xcb_atom_t property)
{
    auto it = m_transfers.find({window, property});
    if (it == m_transfers.end())
        return;
    auto &t = *it->second;
    if (t.pending) {
        t.notifier->setEnabled(false);
        return;
    }
    while (!t.eof && t.data.size() < chunkSize) {
        char data[8192];
        auto count = read(t.fd, data, std::min<size_t>(sizeof(data), chunkSize - t.data.size()));
        if (count < 0 && errno == EINTR)
            continue;
        if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return;
        if (count < 0) {
            finish(window, property);
            return;
        }
        if (!count)
            t.eof = true;
        else
            t.data.append(data, size_t(count));
        t.timer->start(kTransferTimeout);
    }
    if (!t.incremental && !t.eof) {
        t.incremental = true;
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, window, property, atom("INCR"), 32,
                            1, &chunkSize);
        t.pending = true;
        t.notified = true;
        notify(t.request, true);
    } else {
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, window, property, t.target, 8,
                            t.data.size(), t.data.data());
        const bool done = t.eof && (t.data.empty() || !t.incremental);
        t.data.clear();
        if (!t.notified) {
            t.notified = true;
            notify(t.request, true);
        }
        if (done) {
            finish(window, property);
            return;
        }
        t.pending = true;
    }
    t.notifier->setEnabled(false);
    xcb_flush(m_connection);
}

bool SelectionBridge::event(xcb_generic_event_t *generic)
{
    auto type = generic->response_type & 0x7f;
    if (!m_window)
        return false;
    if (type == m_eventBase + XCB_XFIXES_SELECTION_NOTIFY && !(generic->response_type & 0x80)) {
        auto *e = reinterpret_cast<xcb_xfixes_selection_notify_event_t *>(generic);
        if (m_kind == Kind::Clipboard && e->selection == atom("CLIPBOARD_MANAGER")) {
            if (e->owner == m_window)
                m_managerTimestamp = e->selection_timestamp;
            return true;
        }
        if (e->selection != selection())
            return false;
        m_timestamp = e->selection_timestamp;
        if (e->owner == m_window) {
            if (acquired)
                acquired();
            return true;
        }
        if (m_kind == Kind::Dnd) {
            ++m_generation;
            if (ownerChanged)
                ownerChanged(e->owner);
            return true;
        }
        if (!e->owner) {
            ++m_generation;
            if (m_controller && m_source) {
                m_importing = true;
                m_controller->sourceDestroyed(m_source.get());
                m_importing = false;
            }
            m_source.reset();
        } else
            discover(e->selection_timestamp);
        return true;
    }
    if (type == XCB_SELECTION_REQUEST) {
        auto *e = reinterpret_cast<xcb_selection_request_event_t *>(generic);
        if (e->owner != m_window)
            return false;
        request(*e);
        return true;
    }
    if (type == XCB_SELECTION_NOTIFY) {
        auto *e = reinterpret_cast<xcb_selection_notify_event_t *>(generic);
        auto it = m_transfers.find({e->requestor, XCB_ATOM_NONE});
        if (it == m_transfers.end())
            return false;
        if (e->selection != selection() || e->target != it->second->target)
            return true;
        if (e->property == atom("_LINGMO_SELECTION"))
            readProperty(e->requestor);
        else
            finish(e->requestor);
        return true;
    }
    if (type == XCB_PROPERTY_NOTIFY) {
        auto *e = reinterpret_cast<xcb_property_notify_event_t *>(generic);
        auto it = m_transfers.find({e->window, XCB_ATOM_NONE});
        if (it != m_transfers.end() && e->atom == atom("_LINGMO_SELECTION")) {
            if (e->state == XCB_PROPERTY_NEW_VALUE && it->second->incremental)
                readProperty(e->window);
            return true;
        }
        it = m_transfers.find({e->window, e->atom});
        if (it != m_transfers.end() && e->state == XCB_PROPERTY_DELETE && it->second->pending) {
            it->second->pending = false;
            it->second->notifier->setEnabled(true);
            readPipe(e->window, e->atom);
            return true;
        }
    }
    if (type == XCB_DESTROY_NOTIFY) {
        auto window = reinterpret_cast<xcb_destroy_notify_event_t *>(generic)->window;
        for (auto it = m_transfers.begin(); it != m_transfers.end();) {
            auto key = it++->first;
            if (key.first == window)
                finish(key.first, key.second);
        }
    }
    return false;
}

} // namespace Lingmo::XWayland
