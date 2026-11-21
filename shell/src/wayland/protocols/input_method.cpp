// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/input_method.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <deque>
#include <set>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "base/timer.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "input-method-unstable-v2-server-protocol.h"
#include "input/seat.h"
#include "input/session_gate.h"
#include "scene/surface_node.h"
#include "virtual-keyboard-unstable-v1-server-protocol.h"
#include "wayland/protocols/text_input.h"
#include "wayland/protocols/wayland_keyboard_delegate.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Wayland {
namespace {
void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
} // namespace

bool validImeText(const std::string &text)
{
    return text.size() <= 4000;
}

bool validImeOffset(const std::string &text, int64_t offset)
{
    return offset >= 0 && uint64_t(offset) <= text.size()
            && (uint64_t(offset) == text.size() || (uint8_t(text[offset]) & 0xc0) != 0x80);
}

base::Point inputPopupPosition(const base::Rect &caret, const base::Size &size,
                               const base::Rect &area)
{
    base::Point position(caret.x(), caret.y() + caret.height());
    if (area.isEmpty() || size.isEmpty())
        return position;
    const int below = area.y() + area.height() - position.y();
    const int above = caret.y() - area.y();
    if (size.height() > below && above > below)
        position.setY(caret.y() - size.height());
    const base::Point last(area.x() + area.width() - size.width(),
                      area.y() + area.height() - size.height());
    position.setX(std::clamp(position.x(), area.x(), std::max(area.x(), last.x())));
    position.setY(std::clamp(position.y(), area.y(), std::max(area.y(), last.y())));
    return position;
}

bool validCursorRectangle(int32_t x, int32_t y, int32_t width, int32_t height)
{
    return width >= 0 && height >= 0 && int64_t(x) + width <= INT32_MAX
            && int64_t(y) + height <= INT32_MAX;
}

struct InputMethodManager::Impl : InputMethodRelay, input::SeatObserver {
    struct Edit {
        std::optional<std::string> commit;
        // Unset parts of the double-buffered state return to their initial values.
        std::string preedit;
        int32_t begin = 0;
        int32_t end = 0;
        std::optional<std::pair<uint32_t, uint32_t>> deletion;
    };

    struct Method {
        Impl *owner;
        wl_resource *resource;
        uint32_t serial = 0;
        Edit pending;
        std::string preedit;
        int32_t begin = 0;
        int32_t end = 0;
    };

    struct VirtualKeyboard {
        Impl *owner;
        wl_resource *resource;
        bool keymap = false;
    };

    struct Popup : compositor::SurfaceObserver {
        Impl *owner;
        wl_resource *resource;
        compositor::Surface *surface;
        Method *method;
        std::optional<base::Rect> sentRectangle;

        Popup(Impl *owner, wl_resource *resource, compositor::Surface *surface, Method *method)
            : owner(owner)
            , resource(resource)
            , surface(surface)
            , method(method)
        {
            surface->addSurfaceObserver(this);
        }

        ~Popup() override
        {
            if (surface) {
                const auto alive = surface->lifetimeToken();
                surface->removeSurfaceObserver(this);
                surface->setHidden(true);
                if (*alive)
                    (*alive)->reparentNode(nullptr);
            }
        }

        void onSurfaceDestroying(compositor::Surface *) override { surface = nullptr; }

        void onSurfaceCommit(compositor::Surface *) override { owner->updatePopups(); }
    };

    wl_display *display;
    WaylandSeat *m_waylandSeat;
    input::Seat *m_seat;
    scene::Node *popupLayer;
    base::Timer popupSync {core::mainLoop()};
    wl_global *textGlobal = nullptr;
    wl_global *textV1Global = nullptr;
    wl_global *methodGlobal = nullptr;
    wl_global *keyboardGlobal = nullptr;
    std::function<bool(const wl_client *)> authorizer;
    std::function<base::Rect(const base::Rect &)> placementArea;
    std::set<wl_resource *> resources;
    std::vector<TextInput *> texts;
    std::vector<Popup *> popups;
    TextInput *m_active = nullptr;
    Method *method = nullptr;
    wl_resource *grab = nullptr;
    wl_event_source *idle = nullptr;
    uint32_t grabGeneration = 0;
    uint32_t generation = 0;

    struct ForwardedKey {
        uint32_t time;
        uint32_t key;
        bool pressed;
    };

    std::deque<ForwardedKey> forwarded;
    std::set<uint32_t> forwardedDown;

    void releaseForwarded(bool dispatch = true)
    {
        auto keys = std::exchange(forwardedDown, {});
        if (dispatch && usable()) {
            for (auto key : keys)
                m_seat->dispatchKeyToClients(std::chrono::steady_clock::now(), key, false);
        }
    }

    static Impl *self(wl_resource *r) { return getUserDataAs<Impl>(r); }

    static Method *ime(wl_resource *r) { return getUserDataAs<Method>(r); }

    bool usable() const
    {
        return !m_seat->wasShutdown() && m_seat->focusedSurface()
                && (!m_seat->sessionGate() || !m_seat->sessionGate()->restrictsInput());
    }

    bool eligible(const TextInput *t) const
    {
        if (!t->resource())
            return t->attached() && usable() && t->ownsSurface(m_seat->focusedSurface());
        return t->attached() && focusedClient(wl_resource_get_client(t->resource()));
    }

    bool current(Method *m) const { return m == method && m_active && eligible(m_active); }

    bool authorized(wl_client *client) const { return !authorizer || authorizer(client); }

    input::Seat *seat() const override { return m_seat; }

    WaylandSeat *waylandSeat() const override { return m_waylandSeat; }

    compositor::Surface *focusedSurface() const override { return m_seat->focusedSurface(); }

    bool focusedClient(const wl_client *client) const override
    {
        auto *surface = getSurfaceResource(m_seat->focusedSurface());
        return usable() && surface && wl_resource_get_client(surface) == client;
    }

    TextInput *active() const override { return m_active; }

    TextInputEdit currentPreedit(const TextInput *t) const override
    {
        TextInputEdit edit;
        if (method && m_active == t && eligible(t)) {
            edit.preedit = method->preedit;
            edit.begin = method->begin;
            edit.end = method->end;
        }
        return edit;
    }

    void activate(TextInput *t, bool restart) override
    {
        if (m_active && m_active != t) {
            auto *previous = m_active;
            deactivateMethod();
            previous->detached();
        }
        restart = restart || m_active != t;
        m_active = t;
        sendState(restart);
    }

    void deactivate(TextInput *t) override
    {
        if (m_active == t)
            deactivateMethod();
    }

    void stateChanged(TextInput *t, bool batched) override
    {
        if (t != m_active)
            return;
        if (!batched) {
            sendState(false);
            return;
        }
        if (!idle) {
            idle = wl_event_loop_add_idle(
                    wl_display_get_event_loop(display),
                    [](void *data) {
                        auto *o = static_cast<Impl *>(data);
                        o->idle = nullptr;
                        o->sendState(false);
                    },
                    this);
        }
    }

    void add(TextInput *t) override { texts.push_back(t); }

    void remove(TextInput *t) override
    {
        std::erase(texts, t);
        if (m_active == t)
            deactivateMethod();
    }

    wl_resource *createResource(wl_client *client, const wl_interface *interface, uint32_t id,
                                const void *implementation, void *data,
                                wl_resource_destroy_func_t cleanup) override
    {
        auto *r = wl_resource_create(client, interface, 1, id);
        if (!r) {
            wl_client_post_no_memory(client);
            return nullptr;
        }
        resources.insert(r);
        wl_resource_set_implementation(r, implementation, data, cleanup);
        return r;
    }

    void forgetResource(wl_resource *r) override { resources.erase(r); }

    void done()
    {
        ++method->serial;
        zwp_input_method_v2_send_done(method->resource);
    }

    void deactivateMethod(bool focusChanged = false)
    {
        if (!m_active)
            return;
        releaseForwarded(!focusChanged);
        m_active = nullptr;
        ++generation;
        forwarded.clear();
        if (method) {
            method->pending = {};
            zwp_input_method_v2_send_deactivate(method->resource);
            done();
        }
        updatePopups();
    }

    void sendState(bool activate)
    {
        if (!method || !m_active || !eligible(m_active))
            return;
        if (activate) {
            releaseForwarded();
            ++generation;
            forwarded.clear();
            method->pending = {};
            method->preedit.clear();
            method->begin = method->end = 0;
            zwp_input_method_v2_send_activate(method->resource);
        }
        const auto &s = m_active->state();
        if (s.surrounding)
            zwp_input_method_v2_send_surrounding_text(method->resource, s.surrounding->c_str(),
                                                      s.cursor, s.anchor);
        zwp_input_method_v2_send_text_change_cause(method->resource, s.cause);
        if (s.content)
            zwp_input_method_v2_send_content_type(method->resource, s.content->first,
                                                  s.content->second);
        done();
        updatePopups();
    }

    bool hasPopup(Popup *p) const
    {
        return std::find(popups.begin(), popups.end(), p) != popups.end();
    }

    void updatePopups()
    {
        popupSync.stop();
        for (auto *p : std::vector<Popup *>(popups)) {
            if (!hasPopup(p) || !p->surface)
                continue;
            const bool visible = m_active && method && p->method == method && eligible(m_active);
            const auto surface = p->surface->lifetimeToken();
            const auto rectangle = visible ? m_active->state().rectangle : std::nullopt;
            const auto *target = visible ? m_seat->focusedSurface()->node() : nullptr;
            const base::Point origin = target ? target->absolutePosition() : base::Point();
            p->surface->setHidden(!visible);
            if (!*surface || !hasPopup(p))
                continue;
            (*surface)->reparentNode(visible ? popupLayer : nullptr);
            if (!visible) {
                p->sentRectangle.reset();
                continue;
            }
            if (!*surface || !hasPopup(p))
                continue;
            const base::Rect caret = rectangle.value_or(base::Rect()).translated(origin);
            const base::Size size = (*surface)->contentSize();
            const base::Point position = inputPopupPosition(
                    caret, size, placementArea && rectangle ? placementArea(caret) : base::Rect());
            (*surface)->setPosition(position);
            // The cursor as the popup sees it, so the input method can tell
            // which side of it the popup ended up on.
            const base::Rect relative = caret.translated(-position);
            if (rectangle && *surface && hasPopup(p) && p->sentRectangle != relative) {
                p->sentRectangle = relative;
                zwp_input_popup_surface_v2_send_text_input_rectangle(
                        p->resource, relative.x(), relative.y(), relative.width(),
                        relative.height());
            }
        }
    }

    void onSurfaceFocused(compositor::Surface *gained, compositor::Surface *lost) override
    {
        deactivateMethod(true);
        for (auto *t : std::vector<TextInput *>(texts)) {
            if (std::find(texts.begin(), texts.end(), t) != texts.end())
                t->focusChanged(gained, lost);
        }
    }

    bool sendGrabKeymap()
    {
        const auto &keymap = m_seat->clientXkbTracker()->keymap();
        // The trailing NUL is part of the advertised size.
        const size_t size = keymap.size() + 1;
        const int fd = memfd_create("lingmo-keymap", MFD_CLOEXEC);
        size_t written = 0;
        while (fd >= 0 && written < size) {
            const ssize_t count = write(fd, keymap.c_str() + written, size - written);
            if (count < 0 && errno == EINTR)
                continue;
            if (count <= 0)
                break;
            written += size_t(count);
        }
        if (written != size) {
            if (fd >= 0)
                close(fd);
            wl_client_post_no_memory(wl_resource_get_client(grab));
            return false;
        }
        zwp_input_method_keyboard_grab_v2_send_keymap(grab, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                                                      fd, size);
        close(fd);
        return true;
    }

    void sendGrabRepeatInfo()
    {
        const auto &settings = m_seat->keyRepeatSettings();
        zwp_input_method_keyboard_grab_v2_send_repeat_info(
                grab, waylandRepeatRate(settings.enabled, settings.interval),
                static_cast<int32_t>(settings.delay.count()));
    }

    // The keymap and repeat info describe the grab, not the current key
    // stream, so an inactive grab is kept up to date as well.
    void onKeyboardLayoutUpdated() override
    {
        if (grab && sendGrabKeymap())
            onKeyboardModifierUpdated();
    }

    void onKeyRepeatSettingsChanged() override
    {
        if (grab)
            sendGrabRepeatInfo();
    }

    void onKeyboardModifierUpdated() override
    {
        if (!grab || !method || !current(method) || grabGeneration != generation)
            return;
        const auto &m = m_seat->clientXkbTracker()->modifiers();
        zwp_input_method_keyboard_grab_v2_send_modifiers(grab, wl_display_next_serial(display),
                                                         m.depressed, m.latched, m.locked, m.group);
    }

    bool filter(input::Timestamp timestamp, uint32_t key, bool pressed)
    {
        if (!grab || !method || !current(method) || grabGeneration != generation)
            return false;
        auto time = timePointToMilliseconds(timestamp);
        // Bound unacknowledged forwarding without retaining typed text or an unbounded key log.
        if (forwarded.size() == 256)
            forwarded.pop_front();
        forwarded.push_back({time, key, pressed});
        zwp_input_method_keyboard_grab_v2_send_key(grab, wl_display_next_serial(display), time, key,
                                                   pressed);
        return true;
    }

    void commitMethod(Method *m, uint32_t serial)
    {
        auto e = std::exchange(m->pending, {});
        if (!current(m) || serial != m->serial)
            return;
        TextInputEdit edit;
        if (e.deletion) {
            const auto &s = m_active->state();
            if (!s.surrounding
                || !validImeOffset(*s.surrounding, int64_t(s.cursor) - e.deletion->first)
                || !validImeOffset(*s.surrounding, int64_t(s.cursor) + e.deletion->second))
                return;
            edit.deletion = e.deletion;
        }
        edit.commit = std::move(e.commit);
        m->preedit = std::move(e.preedit);
        m->begin = e.begin;
        m->end = e.end;
        edit.preedit = m->preedit;
        edit.begin = m->begin;
        edit.end = m->end;
        m_active->sendEdit(edit);
    }

    static const struct zwp_input_method_v2_interface methodImplementation;
    static const struct zwp_input_method_manager_v2_interface methodManagerImplementation;
    static const struct zwp_virtual_keyboard_v1_interface keyboardImplementation;
    static const struct zwp_virtual_keyboard_manager_v1_interface keyboardManagerImplementation;

    Impl(wl_display *display, WaylandSeat *waylandSeat, scene::Node *popupLayer)
        : display(display)
        , m_waylandSeat(waylandSeat)
        , m_seat(waylandSeat->seat)
        , popupLayer(popupLayer)
    {
        popupSync.setSingleShot(true);
        popupSync.onTimeout = [this] {
            updatePopups();
        };
        textGlobal = createTextInputV3Global(display, this);
        textV1Global = createTextInputV1Global(display, this);
        methodGlobal = wl_global_create(
                display, &zwp_input_method_manager_v2_interface, 1, this,
                [](wl_client *c, void *data, uint32_t, uint32_t id) {
                    auto *o = static_cast<Impl *>(data);
                    if (!o->authorized(c)) {
                        wl_client_post_implementation_error(c, "Input method not authorized");
                        return;
                    }
                    o->createResource(c, &zwp_input_method_manager_v2_interface, id,
                                      &methodManagerImplementation, o, [](wl_resource *r) {
                                          self(r)->resources.erase(r);
                                      });
                });
        keyboardGlobal = wl_global_create(
                display, &zwp_virtual_keyboard_manager_v1_interface, 1, this,
                [](wl_client *c, void *data, uint32_t, uint32_t id) {
                    auto *o = static_cast<Impl *>(data);
                    if (!o->authorized(c)) {
                        wl_client_post_implementation_error(c, "Virtual keyboard not authorized");
                        return;
                    }
                    o->createResource(c, &zwp_virtual_keyboard_manager_v1_interface, id,
                                      &keyboardManagerImplementation, o, [](wl_resource *r) {
                                          self(r)->resources.erase(r);
                                      });
                });
        m_seat->addObserver(this);
        m_seat->setInputMethodFilter([this](input::Timestamp t, uint32_t key, bool pressed) {
            return filter(t, key, pressed);
        });
    }

    ~Impl() override
    {
        m_seat->setInputMethodFilter({});
        m_seat->removeObserver(this);
        while (!resources.empty())
            wl_resource_destroy(*resources.begin());
        if (idle)
            wl_event_source_remove(idle);
        for (auto *global : {textGlobal, textV1Global, methodGlobal, keyboardGlobal}) {
            if (global)
                wl_global_destroy(global);
        }
    }
};

const struct zwp_input_method_v2_interface InputMethodManager::Impl::methodImplementation = {
        [](wl_client *, wl_resource *r, const char *value) {
            if (!validImeText(value)) {
                wl_resource_post_error(r, 0, "Invalid commit text");
                return;
            }
            ime(r)->pending.commit = value;
        },
        [](wl_client *, wl_resource *r, const char *value, int32_t begin, int32_t end) {
            if (!validImeText(value) || (begin != -1 && !validImeOffset(value, begin))
                || (end != -1 && !validImeOffset(value, end))) {
                wl_resource_post_error(r, 0, "Invalid preedit text or offsets");
                return;
            }
            auto &e = ime(r)->pending;
            e.preedit = value;
            e.begin = begin;
            e.end = end;
        },
        [](wl_client *, wl_resource *r, uint32_t before, uint32_t after) {
            ime(r)->pending.deletion = {before, after};
        },
        [](wl_client *, wl_resource *r, uint32_t serial) {
            auto *m = ime(r);
            m->owner->commitMethod(m, serial);
        },
        [](wl_client *c, wl_resource *r, uint32_t id, wl_resource *surfaceResource) {
            auto *m = ime(r);
            auto *o = m->owner;
            auto *s = surfaceFromResource(surfaceResource);
            if (!s || s->hasRole()) {
                wl_resource_post_error(r, ZWP_INPUT_METHOD_V2_ERROR_ROLE,
                                       "Surface already has a role");
                return;
            }
            static const struct zwp_input_popup_surface_v2_interface implementation = {destroy};
            auto *p = new Popup(o, nullptr, s, m);
            p->resource = o->createResource(c, &zwp_input_popup_surface_v2_interface, id,
                                            &implementation, p, [](wl_resource *r) {
                                                std::unique_ptr<Popup> p(getUserDataAs<Popup>(r));
                                                p->owner->resources.erase(r);
                                                std::erase(p->owner->popups, p.get());
                                            });
            if (!p->resource) {
                delete p;
                return;
            }
            s->setHasRole(true);
            o->popups.push_back(p);
            o->updatePopups();
        },
        [](wl_client *c, wl_resource *r, uint32_t id) {
            auto *m = ime(r);
            auto *o = m->owner;
            static const struct zwp_input_method_keyboard_grab_v2_interface implementation = {
                    destroy};
            auto *grab = o->createResource(c, &zwp_input_method_keyboard_grab_v2_interface, id,
                                           &implementation, o, [](wl_resource *r) {
                                               auto *o = self(r);
                                               o->resources.erase(r);
                                               if (o->grab == r) {
                                                   o->grab = nullptr;
                                                   o->releaseForwarded();
                                               }
                                           });
            if (!grab)
                return;
            if (m != o->method)
                return;
            if (o->grab) {
                wl_resource_post_error(r, 0, "Keyboard already grabbed");
                return;
            }
            o->grab = grab;
            o->grabGeneration = o->generation;
            if (!o->sendGrabKeymap())
                return;
            o->sendGrabRepeatInfo();
            o->onKeyboardModifierUpdated();
        },
        destroy};

const struct zwp_input_method_manager_v2_interface
        InputMethodManager::Impl::methodManagerImplementation = {
                [](wl_client *c, wl_resource *r, wl_resource *seatResource, uint32_t id) {
                    auto *o = self(r);
                    if (waylandSeatFromResource(seatResource) != o->m_waylandSeat) {
                        wl_resource_post_error(r, 0, "Unknown seat");
                        return;
                    }
                    auto *m = new Method {o, nullptr};
                    m->resource = o->createResource(
                            c, &zwp_input_method_v2_interface, id, &methodImplementation, m,
                            [](wl_resource *r) {
                                std::unique_ptr<Method> m(ime(r));
                                auto *o = m->owner;
                                o->resources.erase(r);
                                if (o->method == m.get()) {
                                    o->method = nullptr;
                                    o->releaseForwarded();
                                    o->forwarded.clear();
                                    if (o->grab)
                                        wl_resource_destroy(o->grab);
                                }
                                for (auto *p : std::vector<Popup *>(o->popups)) {
                                    if (std::find(o->popups.begin(), o->popups.end(), p)
                                                != o->popups.end()
                                        && p->method == m.get())
                                        wl_resource_destroy(p->resource);
                                }
                            });
                    if (!m->resource) {
                        delete m;
                        return;
                    }
                    if (o->method) {
                        zwp_input_method_v2_send_unavailable(m->resource);
                        return;
                    }
                    o->method = m;
                    o->sendState(true);
                },
                destroy};

const struct zwp_virtual_keyboard_v1_interface InputMethodManager::Impl::keyboardImplementation = {
        [](wl_client *, wl_resource *r, uint32_t format, int32_t fd, uint32_t size) {
            auto *v = getUserDataAs<VirtualKeyboard>(r);
            v->keymap = false;
            const auto &expected = v->owner->m_seat->clientXkbTracker()->keymap();
            if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 && size == expected.size() + 1) {
                std::string bytes(size, '\0');
                if (pread(fd, bytes.data(), size, 0) == ssize_t(size))
                    v->keymap = std::memcmp(bytes.data(), expected.c_str(), size) == 0;
            }
            close(fd);
            if (!v->keymap)
                wl_resource_post_error(r, 0, "IME forwarding requires the seat keymap");
        },
        [](wl_client *c, wl_resource *r, uint32_t time, uint32_t key, uint32_t state) {
            auto *v = getUserDataAs<VirtualKeyboard>(r);
            auto *o = v->owner;
            if (!v->keymap) {
                wl_resource_post_error(r, ZWP_VIRTUAL_KEYBOARD_V1_ERROR_NO_KEYMAP, "No keymap");
                return;
            }
            if (state > 1 || !o->method || !o->current(o->method)
                || wl_resource_get_client(o->method->resource) != c)
                return;
            auto it = std::find_if(
                    o->forwarded.begin(), o->forwarded.end(), [=](const ForwardedKey &k) {
                        return k.time == time && k.key == key && k.pressed == bool(state);
                    });
            if (it == o->forwarded.end())
                return;
            o->forwarded.erase(it);
            if (state)
                o->forwardedDown.insert(key);
            else
                o->forwardedDown.erase(key);
            o->m_seat->dispatchKeyToClients(input::Timestamp(std::chrono::milliseconds(time)), key,
                                            state);
        },
        [](wl_client *, wl_resource *r, uint32_t, uint32_t, uint32_t, uint32_t) {
            if (!getUserDataAs<VirtualKeyboard>(r)->keymap)
                wl_resource_post_error(r, ZWP_VIRTUAL_KEYBOARD_V1_ERROR_NO_KEYMAP, "No keymap");
            // Physical xkb state already reaches application keyboards through Seat.
        },
        destroy};

const struct zwp_virtual_keyboard_manager_v1_interface
        InputMethodManager::Impl::keyboardManagerImplementation = {
                [](wl_client *c, wl_resource *r, wl_resource *seatResource, uint32_t id) {
                    auto *o = self(r);
                    if (waylandSeatFromResource(seatResource) != o->m_waylandSeat) {
                        wl_resource_post_error(r,
                                               ZWP_VIRTUAL_KEYBOARD_MANAGER_V1_ERROR_UNAUTHORIZED,
                                               "Unknown seat");
                        return;
                    }
                    auto *v = new VirtualKeyboard {o, nullptr};
                    v->resource = o->createResource(c, &zwp_virtual_keyboard_v1_interface, id,
                                                    &keyboardImplementation, v, [](wl_resource *r) {
                                                        std::unique_ptr<VirtualKeyboard> v(
                                                                getUserDataAs<VirtualKeyboard>(r));
                                                        v->owner->resources.erase(r);
                                                    });
                    if (!v->resource)
                        delete v;
                }};

InputMethodManager::InputMethodManager(wl_display *display, WaylandSeat *seat,
                                       scene::Node *popupLayer)
    : m_impl(std::make_unique<Impl>(display, seat, popupLayer))
{
}

void InputMethodManager::sceneChanged()
{
    // Scene change callbacks must not mutate the scene, so follow up later.
    if (m_impl && !m_impl->popups.empty())
        m_impl->popupSync.start();
}

InputMethodManager::~InputMethodManager() = default;

void InputMethodManager::shutdown()
{
    m_impl.reset();
}

void InputMethodManager::setPlacementArea(std::function<base::Rect(const base::Rect &caret)> area)
{
    if (m_impl)
        m_impl->placementArea = std::move(area);
}

void InputMethodManager::setAuthorizer(std::function<bool(const wl_client *)> authorizer)
{
    if (m_impl)
        m_impl->authorizer = std::move(authorizer);
}

InputMethodRelay *InputMethodManager::relay() const
{
    return m_impl.get();
}

bool InputMethodManager::hasFocusedTextInput() const
{
    return m_impl && m_impl->m_active && m_impl->eligible(m_impl->m_active);
}

bool InputMethodManager::grabsKeyboard() const
{
    return hasFocusedTextInput() && m_impl->grab && m_impl->method
            && m_impl->current(m_impl->method) && m_impl->grabGeneration == m_impl->generation;
}

bool InputMethodManager::commitText(const std::string &text)
{
    if (!hasFocusedTextInput() || !validImeText(text))
        return false;
    TextInputEdit edit = m_impl->currentPreedit(m_impl->m_active);
    edit.commit = text;
    m_impl->m_active->sendEdit(edit);
    return true;
}

wl_global *InputMethodManager::textInputGlobal() const
{
    return m_impl ? m_impl->textGlobal : nullptr;
}

wl_global *InputMethodManager::textInputV1Global() const
{
    return m_impl ? m_impl->textV1Global : nullptr;
}

wl_global *InputMethodManager::inputMethodGlobal() const
{
    return m_impl ? m_impl->methodGlobal : nullptr;
}

wl_global *InputMethodManager::virtualKeyboardGlobal() const
{
    return m_impl ? m_impl->keyboardGlobal : nullptr;
}
} // namespace Lingmo::Wayland
