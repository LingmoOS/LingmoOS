// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "text-input-unstable-v3-server-protocol.h"
#include "wayland/protocols/text_input.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {
class TextInputV3 final : public TextInput
{
public:
    using TextInput::TextInput;

    static TextInputV3 *from(wl_resource *resource) { return getUserDataAs<TextInputV3>(resource); }

    static bool create(InputMethodRelay *relay, wl_client *client, uint32_t id)
    {
        auto *t = new TextInputV3(relay);
        t->m_resource = relay->createResource(client, &zwp_text_input_v3_interface, id,
                                              &implementation, t, [](wl_resource *r) {
                                                  std::unique_ptr<TextInputV3> t(from(r));
                                                  t->m_relay->forgetResource(r);
                                                  t->m_relay->remove(t.get());
                                              });
        if (!t->m_resource) {
            delete t;
            return false;
        }
        relay->add(t);
        t->enter();
        return true;
    }

    bool attached() const override { return m_entered; }

    void sendEdit(const TextInputEdit &edit) override
    {
        if (edit.deletion)
            zwp_text_input_v3_send_delete_surrounding_text(m_resource, edit.deletion->first,
                                                           edit.deletion->second);
        if (edit.commit)
            zwp_text_input_v3_send_commit_string(m_resource, edit.commit->c_str());
        // No preedit is null: GTK takes an empty string as an open composition and
        // swallows Enter (Ghostty).
        zwp_text_input_v3_send_preedit_string(m_resource,
                                              edit.preedit.empty() ? nullptr
                                                                   : edit.preedit.c_str(),
                                              edit.begin, edit.end);
        zwp_text_input_v3_send_done(m_resource, m_serial);
    }

    void focusChanged(compositor::Surface *, compositor::Surface *lost) override
    {
        if (m_entered) {
            if (auto *r = getSurfaceResource(lost))
                zwp_text_input_v3_send_leave(m_resource, r);
            m_entered = false;
        }
        m_pending = {};
        m_state = {};
        m_pendingEnabled = m_enabled = false;
        m_enable.reset();
        enter();
    }

private:
    void enter()
    {
        auto *surface = getSurfaceResource(m_relay->focusedSurface());
        if (surface && m_relay->focusedClient(wl_resource_get_client(m_resource))) {
            m_entered = true;
            zwp_text_input_v3_send_enter(m_resource, surface);
        }
    }

    void commit()
    {
        ++m_serial;
        applyCommit();
        // Clients hold further state until a done matches their commit count. The
        // current preedit is repeated because done replaces the pending preedit.
        sendEdit(m_relay->currentPreedit(this));
    }

    void applyCommit()
    {
        const auto enable = std::exchange(m_enable, {});
        if (!m_entered || !m_relay->focusedClient(wl_resource_get_client(m_resource))) {
            m_pending = {};
            return;
        }
        auto *active = m_relay->active();
        if (enable && *enable && active && active != this) {
            m_pending = m_state;
            m_pendingEnabled = m_enabled;
            return;
        }
        if (enable)
            m_pendingEnabled = *enable;
        m_state = m_pending;
        m_enabled = m_pendingEnabled;
        m_pending.cause = ZWP_TEXT_INPUT_V3_CHANGE_CAUSE_INPUT_METHOD;
        if (!m_enabled) {
            m_relay->deactivate(this);
            m_state = {};
            m_pending = {};
            return;
        }
        if (active != this || enable.value_or(false))
            m_relay->activate(this, true);
        else
            m_relay->stateChanged(this, false);
    }

    static const struct zwp_text_input_v3_interface implementation;

    TextInputState m_pending;
    bool m_pendingEnabled = false;
    bool m_enabled = false;
    std::optional<bool> m_enable;
    uint32_t m_serial = 0;
    bool m_entered = false;
};

const struct zwp_text_input_v3_interface TextInputV3::implementation = {
        [](wl_client *, wl_resource *r) {
            wl_resource_destroy(r);
        },
        [](wl_client *, wl_resource *r) {
            auto *t = from(r);
            t->m_pending = {};
            t->m_enable = true;
        },
        [](wl_client *, wl_resource *r) {
            from(r)->m_enable = false;
        },
        [](wl_client *, wl_resource *r, const char *value, int32_t cursor, int32_t anchor) {
            std::string s(value);
            if (!validImeText(s) || !validImeOffset(s, cursor) || !validImeOffset(s, anchor)) {
                wl_resource_post_error(r, 0, "Invalid UTF-8 surrounding text or offsets");
                return;
            }
            auto &pending = from(r)->m_pending;
            pending.surrounding = std::move(s);
            pending.cursor = cursor;
            pending.anchor = anchor;
        },
        [](wl_client *, wl_resource *r, uint32_t cause) {
            from(r)->m_pending.cause = cause;
        },
        [](wl_client *, wl_resource *r, uint32_t hint, uint32_t purpose) {
            from(r)->m_pending.content = {hint, purpose};
        },
        [](wl_client *, wl_resource *r, int32_t x, int32_t y, int32_t width, int32_t height) {
            if (!validCursorRectangle(x, y, width, height)) {
                wl_resource_post_error(r, 0, "Invalid cursor rectangle");
                return;
            }
            from(r)->m_pending.rectangle = clientRect(x, y, width, height);
        },
        [](wl_client *, wl_resource *r) {
            from(r)->commit();
        }};

const struct zwp_text_input_manager_v3_interface managerImplementation = {
        [](wl_client *, wl_resource *r) {
            wl_resource_destroy(r);
        },
        [](wl_client *c, wl_resource *r, uint32_t id, wl_resource *seatResource) {
            auto *relay = getUserDataAs<InputMethodRelay>(r);
            if (waylandSeatFromResource(seatResource) != relay->waylandSeat()) {
                wl_resource_post_error(r, 0, "Unknown seat");
                return;
            }
            TextInputV3::create(relay, c, id);
        }};
} // namespace

wl_global *createTextInputV3Global(wl_display *display, InputMethodRelay *relay)
{
    return wl_global_create(display, &zwp_text_input_manager_v3_interface, 1, relay,
                            [](wl_client *c, void *data, uint32_t, uint32_t id) {
                                auto *relay = static_cast<InputMethodRelay *>(data);
                                relay->createResource(
                                        c, &zwp_text_input_manager_v3_interface, id,
                                        &managerImplementation, relay, [](wl_resource *r) {
                                            getUserDataAs<InputMethodRelay>(r)->forgetResource(r);
                                        });
                            });
}
} // namespace Lingmo::Wayland
