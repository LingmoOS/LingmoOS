// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_text_input_manager.cc
//           chromium/src/components/exo/text_input.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the text input is relayed to an input-method-v2 client
// instead of ui::InputMethod. zcr_extended_text_input_v1 and
// zcr_text_input_extension_v1 are ChromeOS extensions and are not ported.

#include <cstring>
#include <memory>
#include <xkbcommon/xkbcommon.h>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "text-input-unstable-v1-server-protocol.h"
#include "text-input-unstable-v3-server-protocol.h"
#include "wayland/protocols/text_input.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {

// The list of modifiers that this supports.
// This is consistent with X.h.
constexpr const char *kModifierNames[] = {
        XKB_MOD_NAME_SHIFT, XKB_MOD_NAME_CAPS,
        XKB_MOD_NAME_CTRL,  XKB_MOD_NAME_ALT,
        XKB_MOD_NAME_NUM,   "Mod3",
        XKB_MOD_NAME_LOGO,  "Mod5",
};

// v1 and v3 hint bits have the same meaning; v1 auto_correction is v3 spellcheck.
uint32_t contentHint(uint32_t hint)
{
    return hint
            & (ZWP_TEXT_INPUT_V1_CONTENT_HINT_AUTO_COMPLETION
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_AUTO_CORRECTION
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_AUTO_CAPITALIZATION
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_LOWERCASE | ZWP_TEXT_INPUT_V1_CONTENT_HINT_UPPERCASE
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_TITLECASE
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_HIDDEN_TEXT
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_SENSITIVE_DATA
               | ZWP_TEXT_INPUT_V1_CONTENT_HINT_LATIN | ZWP_TEXT_INPUT_V1_CONTENT_HINT_MULTILINE);
}

uint32_t contentPurpose(uint32_t purpose)
{
    switch (purpose) {
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_ALPHA:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_ALPHA;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_DIGITS:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DIGITS;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_NUMBER:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NUMBER;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_PHONE:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_PHONE;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_URL:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_URL;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_EMAIL:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_EMAIL;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_NAME:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NAME;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_PASSWORD:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_PASSWORD;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_DATE:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DATE;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_TIME:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_TIME;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_DATETIME:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DATETIME;
    case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_TERMINAL:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_TERMINAL;
    default:
        return ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;
    }
}

////////////////////////////////////////////////////////////////////////////////
// text_input_v1 interface:

class TextInputV1 final : public TextInput, public compositor::SurfaceObserver
{
public:
    using TextInput::TextInput;

    ~TextInputV1() override
    {
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    static TextInputV1 *from(wl_resource *resource) { return getUserDataAs<TextInputV1>(resource); }

    static void create(InputMethodRelay *relay, wl_client *client, uint32_t id)
    {
        auto *t = new TextInputV1(relay);
        t->m_resource = relay->createResource(client, &zwp_text_input_v1_interface, id,
                                              &implementation, t, [](wl_resource *r) {
                                                  std::unique_ptr<TextInputV1> t(from(r));
                                                  t->m_relay->forgetResource(r);
                                                  t->m_relay->remove(t.get());
                                              });
        if (!t->m_resource) {
            delete t;
            return;
        }
        relay->add(t);
    }

    bool attached() const override { return m_attached; }

    void activate(compositor::Surface *surface)
    {
        if (m_surface == surface)
            return;

        deactivate();
        m_surface = surface;
        m_surface->addSurfaceObserver(this);
        if (m_relay->focusedSurface() == m_surface)
            attachInputMethod();
    }

    void deactivate()
    {
        detachInputMethod();
        if (m_surface) {
            m_surface->removeSurfaceObserver(this);
            m_surface = nullptr;
        }
    }

    void detached() override
    {
        if (!m_attached)
            return;
        m_attached = false;
        zwp_text_input_v1_send_leave(m_resource);
        deactivate();
    }

    void focusChanged(compositor::Surface *gained, compositor::Surface *lost) override
    {
        if (!m_surface || gained == lost)
            return;

        if (gained == m_surface)
            attachInputMethod();
        else if (lost == m_surface)
            deactivate();
    }

    void onSurfaceDestroying(compositor::Surface *) override { deactivate(); }

    void sendEdit(const TextInputEdit &edit) override
    {
        if (edit.deletion) {
            // Index is the byte offset of the deletion in the surrounding text,
            // matching exo and Chromium's v1 client rather than the cursor-relative spec.
            zwp_text_input_v1_send_delete_surrounding_text(
                    m_resource, m_state.cursor - int32_t(edit.deletion->first),
                    edit.deletion->first + edit.deletion->second);
        }
        if (edit.commit)
            zwp_text_input_v1_send_commit_string(m_resource, m_serial, edit.commit->c_str());
        sendPreeditStyle(edit);
        if (edit.begin >= 0)
            zwp_text_input_v1_send_preedit_cursor(m_resource, edit.begin);
        zwp_text_input_v1_send_preedit_string(m_resource, m_serial, edit.preedit.c_str(), "");
    }

private:
    void attachInputMethod()
    {
        if (m_attached || !m_relay->focusedClient(wl_resource_get_client(m_resource)))
            return;
        m_attached = true;
        zwp_text_input_v1_send_enter(m_resource, getSurfaceResource(m_surface));
        m_relay->activate(this, true);
    }

    void detachInputMethod()
    {
        if (!m_attached)
            return;
        m_attached = false;
        m_relay->deactivate(this);
        zwp_text_input_v1_send_leave(m_resource);
    }

    void sendPreeditStyle(const TextInputEdit &edit)
    {
        if (edit.preedit.empty())
            return;
        zwp_text_input_v1_send_preedit_styling(m_resource, 0, edit.preedit.size(),
                                               ZWP_TEXT_INPUT_V1_PREEDIT_STYLE_UNDERLINE);
        if (edit.begin >= 0 && edit.end > edit.begin)
            zwp_text_input_v1_send_preedit_styling(m_resource, edit.begin, edit.end - edit.begin,
                                                   ZWP_TEXT_INPUT_V1_PREEDIT_STYLE_HIGHLIGHT);
    }

    void changed() { m_relay->stateChanged(this, true); }

    static void textInputActivate(wl_client *, wl_resource *resource, wl_resource *seatResource,
                                  wl_resource *surfaceResource)
    {
        auto *t = from(resource);
        auto *surface = surfaceFromResource(surfaceResource);
        if (!surface || waylandSeatFromResource(seatResource) != t->m_relay->waylandSeat())
            return;
        t->activate(surface);

        // Sending modifiers.
        wl_array modifiers;
        wl_array_init(&modifiers);
        for (const char *modifier : kModifierNames) {
            auto *p = static_cast<char *>(wl_array_add(&modifiers, std::strlen(modifier) + 1));
            if (p)
                std::strcpy(p, modifier);
        }
        zwp_text_input_v1_send_modifiers_map(resource, &modifiers);
        wl_array_release(&modifiers);
    }

    static void textInputDeactivate(wl_client *, wl_resource *resource, wl_resource *)
    {
        from(resource)->deactivate();
    }

    static void textInputShowInputPanel(wl_client *, wl_resource *) { }

    static void textInputHideInputPanel(wl_client *, wl_resource *) { }

    static void textInputReset(wl_client *, wl_resource *resource)
    {
        auto *t = from(resource);
        if (t->m_attached && t->m_relay->active() == t)
            t->m_relay->activate(t, true);
    }

    static void textInputSetSurroundingText(wl_client *, wl_resource *resource, const char *text,
                                            uint32_t cursor, uint32_t anchor)
    {
        auto *t = from(resource);
        std::string s(text);
        if (!validImeText(s) || !validImeOffset(s, cursor) || !validImeOffset(s, anchor)) {
            t->m_state.surrounding.reset();
        } else {
            t->m_state.surrounding = std::move(s);
            t->m_state.cursor = int32_t(cursor);
            t->m_state.anchor = int32_t(anchor);
        }
        t->changed();
    }

    static void textInputSetContentType(wl_client *, wl_resource *resource, uint32_t hint,
                                        uint32_t purpose)
    {
        auto *t = from(resource);
        t->m_state.content = {contentHint(hint), contentPurpose(purpose)};
        t->changed();
    }

    static void textInputSetCursorRectangle(wl_client *, wl_resource *resource, int32_t x,
                                            int32_t y, int32_t width, int32_t height)
    {
        if (!validCursorRectangle(x, y, width, height))
            return;
        auto *t = from(resource);
        t->m_state.rectangle = clientRect(x, y, width, height);
        t->changed();
    }

    static void textInputSetPreferredLanguage(wl_client *, wl_resource *, const char *)
    {
        // Nothing needs to be done.
    }

    static void textInputCommitState(wl_client *, wl_resource *resource, uint32_t serial)
    {
        auto *t = from(resource);
        t->m_serial = serial;
        t->m_relay->stateChanged(t, false);
    }

    static void textInputInvokeAction(wl_client *, wl_resource *, uint32_t, uint32_t)
    {
        // input-method-v2 has no preedit actions.
    }

    static constexpr struct zwp_text_input_v1_interface implementation = {
            textInputActivate,
            textInputDeactivate,
            textInputShowInputPanel,
            textInputHideInputPanel,
            textInputReset,
            textInputSetSurroundingText,
            textInputSetContentType,
            textInputSetCursorRectangle,
            textInputSetPreferredLanguage,
            textInputCommitState,
            textInputInvokeAction,
    };

    compositor::Surface *m_surface = nullptr;
    bool m_attached = false;
    uint32_t m_serial = 0;
};

////////////////////////////////////////////////////////////////////////////////
// text_input_manager_v1 interface:

void textInputManagerCreateTextInput(wl_client *client, wl_resource *resource, uint32_t id)
{
    TextInputV1::create(getUserDataAs<InputMethodRelay>(resource), client, id);
}

constexpr struct zwp_text_input_manager_v1_interface textInputManagerImplementation = {
        textInputManagerCreateTextInput,
};
} // namespace

wl_global *createTextInputV1Global(wl_display *display, InputMethodRelay *relay)
{
    return wl_global_create(display, &zwp_text_input_manager_v1_interface, 1, relay,
                            [](wl_client *c, void *data, uint32_t, uint32_t id) {
                                auto *relay = static_cast<InputMethodRelay *>(data);
                                relay->createResource(
                                        c, &zwp_text_input_manager_v1_interface, id,
                                        &textInputManagerImplementation, relay, [](wl_resource *r) {
                                            getUserDataAs<InputMethodRelay>(r)->forgetResource(r);
                                        });
                            });
}
} // namespace Lingmo::Wayland
