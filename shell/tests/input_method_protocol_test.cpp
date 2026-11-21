// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "clipboard_test_util.h"
#include "input-method-unstable-v2-client-protocol.h"
#include "input/session_gate.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"
#include "text-input-unstable-v1-client-protocol.h"
#include "text-input-unstable-v3-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"
#include "wayland/protocols/input_method.h"

using namespace clipboard_test;

namespace {
struct Endpoint {
    Harness &h;
    Client &client;
    wl_registry *registry;
    zwp_text_input_manager_v3 *textManager = nullptr;
    zwp_text_input_manager_v1 *textManagerV1 = nullptr;
    zwp_input_method_manager_v2 *methodManager = nullptr;
    zwp_virtual_keyboard_manager_v1 *keyboardManager = nullptr;
    zwp_text_input_v3 *text = nullptr;
    zwp_text_input_v1 *textV1 = nullptr;
    zwp_input_method_v2 *method = nullptr;
    zwp_input_method_keyboard_grab_v2 *grab = nullptr;
    zwp_virtual_keyboard_v1 *keyboard = nullptr;
    uint32_t serial = 0;
    uint32_t textSerial = 0;
    int enters = 0;
    int leaves = 0;
    int activates = 0;
    int deactivates = 0;
    int unavailable = 0;
    int done = 0;
    std::string commit;
    std::string preedit;
    bool preeditNull = false;
    std::string surrounding;
    uint32_t before = 0;
    uint32_t after = 0;
    uint32_t purpose = 0;
    int modifierMaps = 0;
    int32_t preeditCursor = -1;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> styles;
    int32_t deleteIndex = -1;
    uint32_t deleteLength = 0;
    uint32_t eventSerial = 0;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> keys;
    bool forward = false;
    bool keymapReceived = false;
    int keymaps = 0;
    int32_t repeatRate = -1;
    int32_t repeatDelay = -1;

    bool bindMethod = true;

    Endpoint(Harness &h, Client &client, bool bindMethod = true)
        : h(h)
        , client(client)
        , bindMethod(bindMethod)
    {
        registry = wl_display_get_registry(client.display);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *r, uint32_t name, const char *interface, uint32_t) {
                    auto *e = static_cast<Endpoint *>(data);
                    if (!strcmp(interface, "zwp_text_input_manager_v3"))
                        e->textManager = static_cast<zwp_text_input_manager_v3 *>(
                                wl_registry_bind(r, name, &zwp_text_input_manager_v3_interface, 1));
                    if (!strcmp(interface, "zwp_text_input_manager_v1"))
                        e->textManagerV1 = static_cast<zwp_text_input_manager_v1 *>(
                                wl_registry_bind(r, name, &zwp_text_input_manager_v1_interface, 1));
                    if (!strcmp(interface, "zwp_input_method_manager_v2") && e->bindMethod)
                        e->methodManager =
                                static_cast<zwp_input_method_manager_v2 *>(wl_registry_bind(
                                        r, name, &zwp_input_method_manager_v2_interface, 1));
                    if (!strcmp(interface, "zwp_virtual_keyboard_manager_v1") && e->bindMethod)
                        e->keyboardManager =
                                static_cast<zwp_virtual_keyboard_manager_v1 *>(wl_registry_bind(
                                        r, name, &zwp_virtual_keyboard_manager_v1_interface, 1));
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        h.sync(client);
    }

    void addText()
    {
        text = zwp_text_input_manager_v3_get_text_input(textManager, client.seat);
        static const zwp_text_input_v3_listener listener = {
                [](void *d, zwp_text_input_v3 *, wl_surface *) {
                    ++static_cast<Endpoint *>(d)->enters;
                },
                [](void *d, zwp_text_input_v3 *, wl_surface *) {
                    ++static_cast<Endpoint *>(d)->leaves;
                },
                [](void *d, zwp_text_input_v3 *, const char *s, int32_t, int32_t) {
                    static_cast<Endpoint *>(d)->preedit = s ? s : "";
                    static_cast<Endpoint *>(d)->preeditNull = !s;
                },
                [](void *d, zwp_text_input_v3 *, const char *s) {
                    static_cast<Endpoint *>(d)->commit += s ? s : "";
                },
                [](void *d, zwp_text_input_v3 *, uint32_t before, uint32_t after) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->before = before;
                    e->after = after;
                },
                [](void *d, zwp_text_input_v3 *, uint32_t serial) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->textSerial = serial;
                    ++e->done;
                }};
        zwp_text_input_v3_add_listener(text, &listener, this);
        h.sync(client);
    }

    void addTextV1()
    {
        textV1 = zwp_text_input_manager_v1_create_text_input(textManagerV1);
        static const zwp_text_input_v1_listener listener = {
                [](void *d, zwp_text_input_v1 *, wl_surface *) {
                    ++static_cast<Endpoint *>(d)->enters;
                },
                [](void *d, zwp_text_input_v1 *) {
                    ++static_cast<Endpoint *>(d)->leaves;
                },
                [](void *d, zwp_text_input_v1 *, wl_array *) {
                    ++static_cast<Endpoint *>(d)->modifierMaps;
                },
                [](void *, zwp_text_input_v1 *, uint32_t) {},
                [](void *d, zwp_text_input_v1 *, uint32_t serial, const char *s, const char *) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->preedit = s ? s : "";
                    e->eventSerial = serial;
                    ++e->done;
                },
                [](void *d, zwp_text_input_v1 *, uint32_t index, uint32_t length, uint32_t style) {
                    static_cast<Endpoint *>(d)->styles.emplace_back(index, length, style);
                },
                [](void *d, zwp_text_input_v1 *, int32_t index) {
                    static_cast<Endpoint *>(d)->preeditCursor = index;
                },
                [](void *d, zwp_text_input_v1 *, uint32_t serial, const char *s) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->commit += s ? s : "";
                    e->eventSerial = serial;
                },
                [](void *, zwp_text_input_v1 *, int32_t, int32_t) {},
                [](void *d, zwp_text_input_v1 *, int32_t index, uint32_t length) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->deleteIndex = index;
                    e->deleteLength = length;
                },
                [](void *, zwp_text_input_v1 *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
                },
                [](void *, zwp_text_input_v1 *, uint32_t, const char *) {},
                [](void *, zwp_text_input_v1 *, uint32_t, uint32_t) {}};
        zwp_text_input_v1_add_listener(textV1, &listener, this);
        h.sync(client);
    }

    void activateV1()
    {
        zwp_text_input_v1_activate(textV1, client.seat, client.surface);
        zwp_text_input_v1_set_surrounding_text(textV1, "a中b", 4, 4);
        zwp_text_input_v1_set_content_type(textV1, 0, ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_DATE);
        zwp_text_input_v1_set_cursor_rectangle(textV1, 10, 20, 2, 15);
        h.sync(client);
    }

    void addMethod()
    {
        method = zwp_input_method_manager_v2_get_input_method(methodManager, client.seat);
        static const zwp_input_method_v2_listener listener = {
                [](void *d, zwp_input_method_v2 *) {
                    ++static_cast<Endpoint *>(d)->activates;
                },
                [](void *d, zwp_input_method_v2 *) {
                    ++static_cast<Endpoint *>(d)->deactivates;
                },
                [](void *d, zwp_input_method_v2 *, const char *s, uint32_t, uint32_t) {
                    static_cast<Endpoint *>(d)->surrounding = s;
                },
                [](void *, zwp_input_method_v2 *, uint32_t) {},
                [](void *d, zwp_input_method_v2 *, uint32_t, uint32_t purpose) {
                    static_cast<Endpoint *>(d)->purpose = purpose;
                },
                [](void *d, zwp_input_method_v2 *) {
                    ++static_cast<Endpoint *>(d)->serial;
                },
                [](void *d, zwp_input_method_v2 *) {
                    ++static_cast<Endpoint *>(d)->unavailable;
                }};
        zwp_input_method_v2_add_listener(method, &listener, this);
        h.sync(client);
    }

    void addGrab()
    {
        keyboard = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(keyboardManager,
                                                                           client.seat);
        grab = zwp_input_method_v2_grab_keyboard(method);
        static const zwp_input_method_keyboard_grab_v2_listener listener = {
                [](void *d, zwp_input_method_keyboard_grab_v2 *, uint32_t format, int32_t fd,
                   uint32_t size) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->keymapReceived = true;
                    ++e->keymaps;
                    zwp_virtual_keyboard_v1_keymap(e->keyboard, format, fd, size);
                    close(fd);
                },
                [](void *d, zwp_input_method_keyboard_grab_v2 *, uint32_t, uint32_t time,
                   uint32_t key, uint32_t state) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->keys.emplace_back(time, key, state);
                    if (e->forward)
                        zwp_virtual_keyboard_v1_key(e->keyboard, time, key, state);
                },
                [](void *, zwp_input_method_keyboard_grab_v2 *, uint32_t, uint32_t, uint32_t,
                   uint32_t, uint32_t) {},
                [](void *d, zwp_input_method_keyboard_grab_v2 *, int32_t rate, int32_t delay) {
                    auto *e = static_cast<Endpoint *>(d);
                    e->repeatRate = rate;
                    e->repeatDelay = delay;
                }};
        zwp_input_method_keyboard_grab_v2_add_listener(grab, &listener, this);
        h.sync(client);
        h.sync(client);
    }

    void enable()
    {
        zwp_text_input_v3_enable(text);
        zwp_text_input_v3_set_surrounding_text(text, "a中b", 4, 4);
        zwp_text_input_v3_set_content_type(text, 0, 0);
        zwp_text_input_v3_set_cursor_rectangle(text, 10, 20, 2, 15);
        zwp_text_input_v3_commit(text);
        h.sync(client);
    }

    ~Endpoint()
    {
        if (grab)
            zwp_input_method_keyboard_grab_v2_release(grab);
        if (keyboard)
            zwp_virtual_keyboard_v1_destroy(keyboard);
        if (method)
            zwp_input_method_v2_destroy(method);
        if (text)
            zwp_text_input_v3_destroy(text);
        if (textV1)
            wl_proxy_destroy(reinterpret_cast<wl_proxy *>(textV1));
        if (textManagerV1)
            wl_proxy_destroy(reinterpret_cast<wl_proxy *>(textManagerV1));
        if (textManager)
            zwp_text_input_manager_v3_destroy(textManager);
        if (methodManager)
            zwp_input_method_manager_v2_destroy(methodManager);
        if (keyboardManager)
            zwp_virtual_keyboard_manager_v1_destroy(keyboardManager);
        wl_registry_destroy(registry);
        wl_display_flush(client.display);
    }
};

struct ImeHarness : Harness {
    scene::Node *popupLayer =
            scene.root()->addChild(std::make_unique<scene::Node>("input-method"));
    wayland::InputMethodManager manager {server.display(), &waylandSeat, popupLayer};
};
} // namespace

TEST(InputMethodProtocol, FocusDoubleBufferingUnicodeAndStaleSerial)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    app.enable();
    EXPECT_EQ(0, app.enters);
    h.focus(app.client);
    EXPECT_EQ(1, app.enters);
    zwp_text_input_v3_enable(app.text);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(0, ime.activates);
    app.enable();
    h.sync(ime.client);
    EXPECT_EQ(1, ime.activates);
    EXPECT_EQ("a中b", ime.surrounding);
    zwp_input_method_v2_set_preedit_string(ime.method, "你好", 6, 6);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_TRUE(app.preedit.empty());
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("你好", app.preedit);
    zwp_input_method_v2_delete_surrounding_text(ime.method, 3, 1);
    zwp_input_method_v2_commit_string(ime.method, "世界");
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("世界", app.commit);
    EXPECT_EQ(3u, app.before);
    EXPECT_EQ(1u, app.after);
    EXPECT_EQ(2u, app.textSerial);
    const auto stale = ime.serial;
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    h.sync(ime.client);
    zwp_input_method_v2_commit_string(ime.method, "stale");
    zwp_input_method_v2_commit(ime.method, stale);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("世界", app.commit);
    h.focus(ime.client);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(1, app.leaves);
    EXPECT_EQ(1, ime.deactivates);
    zwp_input_method_v2_commit_string(ime.method, "wrong focus");
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("世界", app.commit);
    h.focus(app.client);
    h.sync(ime.client);
    EXPECT_EQ(1, ime.activates);
}

TEST(InputMethodProtocol, EveryCommitIsAcknowledgedWithTheCurrentPreedit)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    // Chromium sends its cursor rectangle only once done matches its commit count.
    zwp_text_input_v3_enable(app.text);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    EXPECT_EQ(1u, app.textSerial);
    EXPECT_EQ(1, app.done);
    // GTK reads an empty string as an open composition and swallows Enter.
    EXPECT_TRUE(app.preeditNull);
    zwp_text_input_v3_set_cursor_rectangle(app.text, 10, 20, 2, 15);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    EXPECT_EQ(2u, app.textSerial);

    h.sync(ime.client);
    zwp_input_method_v2_set_preedit_string(ime.method, "ni", 2, 2);
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("ni", app.preedit);
    EXPECT_FALSE(app.preeditNull);
    zwp_text_input_v3_set_cursor_rectangle(app.text, 12, 20, 2, 15);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    EXPECT_EQ(3u, app.textSerial);
    EXPECT_EQ("ni", app.preedit);

    h.focus(ime.client);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    EXPECT_EQ(4u, app.textSerial);
    EXPECT_TRUE(app.preeditNull);
}

TEST(InputMethodProtocol, CommitWithoutPreeditClearsIt)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    zwp_input_method_v2_set_preedit_string(ime.method, "xin j p", 7, 7);
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("xin j p", app.preedit);

    // fcitx5 commits the conversion without setting an empty preedit.
    zwp_input_method_v2_commit_string(ime.method, "新加坡");
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("新加坡", app.commit);
    EXPECT_TRUE(app.preedit.empty());

    // A later client commit, such as after Backspace, must not bring the old preedit back.
    zwp_text_input_v3_set_surrounding_text(app.text, "新加", 6, 6);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    EXPECT_TRUE(app.preedit.empty());
}

TEST(InputMethodProtocol, LateMethodReconnectDuplicateAndDisable)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient()), duplicate(h, h.addClient());
    h.focus(app.client);
    app.addText();
    app.enable();
    EXPECT_EQ(1, app.enters);
    ime.addMethod();
    EXPECT_EQ(1, ime.activates);
    duplicate.addMethod();
    EXPECT_EQ(1, duplicate.unavailable);
    zwp_input_method_v2_destroy(ime.method);
    ime.method = nullptr;
    h.sync(ime.client);
    ime.addMethod();
    EXPECT_EQ(2, ime.activates);
    zwp_text_input_v3_disable(app.text);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(0, ime.deactivates);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(1, ime.deactivates);
}

TEST(InputMethodProtocol, GrabForwardingAndFocusChangeReleasePairing)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient()), other(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    ime.addGrab();
    EXPECT_TRUE(ime.keymapReceived);
    const auto t = input::Timestamp(std::chrono::milliseconds(100));
    h.seat.onKey(t, 30, true);
    h.seat.onKey(t, 30, false);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ(2u, ime.keys.size());
    EXPECT_TRUE(app.client.keyEvents.empty());
    ime.forward = true;
    h.seat.onKey(t, 48, true);
    h.sync(ime.client);
    h.sync(ime.client);
    h.sync(app.client);
    h.seat.onKey(t, 48, false);
    h.sync(ime.client);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ((std::vector<std::pair<uint32_t, bool>> {{48, true}, {48, false}}),
              app.client.keyEvents);
    ime.forward = false;
    h.seat.onKey(t, 46, true);
    h.sync(ime.client);
    h.focus(other.client);
    EXPECT_FALSE(h.seat.clientPressedKeys().contains(46));
    h.seat.onKey(t, 46, false);
    h.sync(other.client);
    EXPECT_TRUE(other.client.keyEvents.empty());
    zwp_virtual_keyboard_v1_key(ime.keyboard, 100, 46, 1);
    h.sync(ime.client);
    h.sync(other.client);
    EXPECT_TRUE(other.client.keyEvents.empty());
}

TEST(InputMethodProtocol, TheGrabFollowsLayoutAndRepeatChanges)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    ime.addGrab();
    EXPECT_EQ(1, ime.keymaps);
    EXPECT_EQ(25, ime.repeatRate);
    EXPECT_EQ(500, ime.repeatDelay);

    input::KeyRepeatSettings settings;
    settings.delay = std::chrono::milliseconds(300);
    settings.interval = std::chrono::milliseconds(20);
    h.seat.setKeyRepeatSettings(settings);
    ASSERT_TRUE(h.seat.setKeyboardLayout("de"));
    h.sync(ime.client);
    h.sync(ime.client);
    EXPECT_EQ(2, ime.keymaps);
    EXPECT_EQ(50, ime.repeatRate);
    EXPECT_EQ(300, ime.repeatDelay);

    // The input method re-uploaded the new keymap, so forwarding still works.
    ime.forward = true;
    const auto t = input::Timestamp(std::chrono::milliseconds(100));
    h.seat.onKey(t, 48, true);
    h.sync(ime.client);
    h.sync(ime.client);
    h.sync(app.client);
    h.seat.onKey(t, 48, false);
    h.sync(ime.client);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ((std::vector<std::pair<uint32_t, bool>> {{48, true}, {48, false}}),
              app.client.keyEvents);
}

TEST(InputMethodProtocol, PopupPositionHideAndSurfaceDestruction)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    auto *window = h.seat.focusedSurface()->node();
    window->setPosition(base::Point(100, 50));
    auto *surface = wl_compositor_create_surface(ime.client.compositor);
    auto *popup = zwp_input_method_v2_get_input_popup_surface(ime.method, surface);
    int rectangles = 0;
    static const zwp_input_popup_surface_v2_listener listener = {
            [](void *data, zwp_input_popup_surface_v2 *, int32_t x, int32_t y, int32_t w,
               int32_t h) {
                ++*static_cast<int *>(data);
                EXPECT_EQ(0, x);
                EXPECT_EQ(-15, y);
                EXPECT_EQ(2, w);
                EXPECT_EQ(15, h);
            }};
    zwp_input_popup_surface_v2_add_listener(popup, &listener, &rectangles);
    h.sync(ime.client);
    auto *resource = wl_client_get_object(ime.client.serverClient,
                                          wl_proxy_get_id(reinterpret_cast<wl_proxy *>(surface)));
    auto *s = wayland::surfaceFromResource(resource);
    ASSERT_NE(nullptr, s);
    // In the popup layer rather than inside the window, so application popups cannot cover it.
    EXPECT_EQ(h.popupLayer, s->node()->parent());
    EXPECT_EQ(base::Point(110, 85), s->node()->absolutePosition());
    EXPECT_EQ(1, rectangles);

    window->setPosition(base::Point(300, 200));
    h.manager.sceneChanged();
    ASSERT_TRUE(h.until([&] {
        return s->node()->absolutePosition() == base::Point(310, 235);
    }));
    h.sync(ime.client);
    EXPECT_EQ(1, rectangles);
    wl_surface_destroy(app.client.surface);
    app.client.surface = nullptr;
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_TRUE(s->isHidden());
    EXPECT_EQ(nullptr, s->node()->parent());
    wl_surface_destroy(surface);
    h.sync(ime.client);
    zwp_input_popup_surface_v2_destroy(popup);
    h.sync(ime.client);
}

TEST(InputMethodProtocol, PopupPositionKeepsInsideTheAreaAndFlipsAboveTheCaret)
{
    const base::Rect area(0, 0, 1000, 800);
    const base::Size size(300, 120);
    // Below the caret while it fits.
    EXPECT_EQ(base::Point(100, 115), wayland::inputPopupPosition({100, 100, 2, 15}, size, area));
    // Above it at the bottom edge.
    EXPECT_EQ(base::Point(100, 580), wayland::inputPopupPosition({100, 700, 2, 15}, size, area));
    // Pushed back in from the right edge, still below.
    EXPECT_EQ(base::Point(700, 115), wayland::inputPopupPosition({900, 100, 2, 15}, size, area));
    // Too tall for either side: the roomier one, pushed back into the area.
    EXPECT_EQ(base::Point(100, 300),
              wayland::inputPopupPosition({100, 300, 2, 15}, {300, 500}, area));
    EXPECT_EQ(base::Point(100, 0),
              wayland::inputPopupPosition({100, 400, 2, 15}, {300, 500}, area));
    // No area or no content yet: just below the caret.
    EXPECT_EQ(base::Point(900, 815),
              wayland::inputPopupPosition({900, 800, 2, 15}, size, base::Rect()));
    EXPECT_EQ(base::Point(900, 815),
              wayland::inputPopupPosition({900, 800, 2, 15}, base::Size(), area));
}

TEST(InputMethodProtocol, PopupFlipsAboveTheCaretAndTellsTheInputMethod)
{
    ImeHarness h;
    h.manager.setPlacementArea([](const base::Rect &) {
        return base::Rect(0, 0, 400, 120);
    });
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    h.seat.focusedSurface()->node()->setPosition(base::Point(100, 50));
    auto *surface = wl_compositor_create_surface(ime.client.compositor);
    auto *popup = zwp_input_method_v2_get_input_popup_surface(ime.method, surface);
    base::Rect rectangle;
    static const zwp_input_popup_surface_v2_listener listener = {
            [](void *data, zwp_input_popup_surface_v2 *, int32_t x, int32_t y, int32_t w,
               int32_t h) {
                *static_cast<base::Rect *>(data) = base::Rect(x, y, w, h);
            }};
    zwp_input_popup_surface_v2_add_listener(popup, &listener, &rectangle);
    h.sync(ime.client);
    auto *resource = wl_client_get_object(ime.client.serverClient,
                                          wl_proxy_get_id(reinterpret_cast<wl_proxy *>(surface)));
    auto *s = wayland::surfaceFromResource(resource);
    ASSERT_NE(nullptr, s);
    // Nothing drawn yet: under the caret at (110, 70).
    EXPECT_EQ(base::Point(110, 85), s->node()->absolutePosition());

    // 60 high does not fit the 35 below the caret, so it goes above.
    auto pool = testsupport::makePool(1 << 16);
    s->attach(pool->createBuffer({100, 60}, 0, 400, render::PixelFormat::Bgrx8888, nullptr), {});
    s->commit();
    EXPECT_EQ(base::Point(110, 10), s->node()->absolutePosition());
    h.sync(ime.client);
    EXPECT_EQ(base::Rect(0, 60, 2, 15), rectangle);
    // Candidates are picked with the pointer: the popup keeps its input region.
    EXPECT_TRUE(s->hitTest(base::Point(50, 30)));

    wl_surface_destroy(surface);
    h.sync(ime.client);
    zwp_input_popup_surface_v2_destroy(popup);
    h.sync(ime.client);
}

TEST(InputMethodProtocol, DisableAndDisconnectReleaseForwardedKeys)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    ime.addGrab();
    ime.forward = true;
    h.seat.onKey(input::Timestamp {}, 30, true);
    h.sync(ime.client);
    h.sync(ime.client);
    h.sync(app.client);
    zwp_text_input_v3_disable(app.text);
    zwp_text_input_v3_commit(app.text);
    h.sync(app.client);
    EXPECT_EQ((std::vector<std::pair<uint32_t, bool>> {{30, true}, {30, false}}),
              app.client.keyEvents);
    h.seat.onKey(input::Timestamp {}, 30, false);
    h.sync(app.client);
    EXPECT_EQ(2u, app.client.keyEvents.size());
    zwp_input_method_keyboard_grab_v2_release(ime.grab);
    ime.grab = nullptr;
    zwp_virtual_keyboard_v1_destroy(ime.keyboard);
    ime.keyboard = nullptr;
    app.enable();
    h.sync(ime.client);
    ime.addGrab();
    h.seat.onKey(input::Timestamp {}, 48, true);
    h.sync(ime.client);
    h.sync(ime.client);
    h.sync(app.client);
    zwp_input_method_v2_destroy(ime.method);
    ime.method = nullptr;
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ((std::pair<uint32_t, bool> {48, false}), app.client.keyEvents.back());
}

TEST(InputMethodProtocol, RestrictedSessionRejectsTextAndForwarding)
{
    struct Gate : input::SessionGate {
        bool restrictsInput() const override { return true; }

        input::EventSink *secureInput() const override { return nullptr; }

        scene::Node *inputRoot() const override { return nullptr; }

        bool allowsFocus(const compositor::Surface *) const override { return false; }

        bool allowsActivation() const override { return false; }
    } gate;

    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.enable();
    h.sync(ime.client);
    ime.addGrab();
    h.seat.setSessionGate(&gate);
    zwp_input_method_v2_commit_string(ime.method, "blocked");
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_TRUE(app.commit.empty());
    h.seat.setFocusedSurface(nullptr);
    h.sync(ime.client);
    EXPECT_EQ(1, ime.deactivates);
    h.seat.setSessionGate(nullptr);
}

TEST(InputMethodProtocol, InvalidUtf8OffsetDisconnectsOnlyOffendingClient)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addText();
    ime.addMethod();
    h.focus(app.client);
    app.client.expectError = true;
    zwp_text_input_v3_set_surrounding_text(app.text, "中", 1, 0);
    ASSERT_TRUE(h.until([&] {
        return wl_display_get_error(app.client.display) != 0;
    }));
    h.sync(ime.client);
    EXPECT_EQ(0, ime.activates);
}

TEST(InputMethodProtocol, TextInputV1ActivatesOnFocusAndRelaysEdits)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addTextV1();
    ime.addMethod();
    app.activateV1();
    EXPECT_EQ(0, app.enters);
    EXPECT_EQ(1, app.modifierMaps);
    h.sync(ime.client);
    EXPECT_EQ(0, ime.activates);
    h.focus(app.client);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(1, app.enters);
    EXPECT_EQ(1, ime.activates);
    EXPECT_EQ("a中b", ime.surrounding);
    EXPECT_EQ(uint32_t(ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DATE), ime.purpose);

    zwp_text_input_v1_commit_state(app.textV1, 42);
    h.sync(app.client);
    h.sync(ime.client);
    zwp_input_method_v2_set_preedit_string(ime.method, "你好", 3, 6);
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("你好", app.preedit);
    EXPECT_EQ(3, app.preeditCursor);
    EXPECT_EQ((std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> {
                      {0, 6, ZWP_TEXT_INPUT_V1_PREEDIT_STYLE_UNDERLINE},
                      {3, 3, ZWP_TEXT_INPUT_V1_PREEDIT_STYLE_HIGHLIGHT}}),
              app.styles);

    zwp_input_method_v2_delete_surrounding_text(ime.method, 3, 1);
    zwp_input_method_v2_commit_string(ime.method, "世界");
    zwp_input_method_v2_set_preedit_string(ime.method, "", -1, -1);
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_EQ("世界", app.commit);
    EXPECT_TRUE(app.preedit.empty());
    // Exo's byte offset from the start of the surrounding text, not cursor-relative.
    EXPECT_EQ(1, app.deleteIndex);
    EXPECT_EQ(4u, app.deleteLength);
    EXPECT_EQ(42u, app.eventSerial);

    zwp_text_input_v1_set_content_type(app.textV1, 0, ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_TERMINAL);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(uint32_t(ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_TERMINAL), ime.purpose);
    EXPECT_EQ(1, ime.activates);

    zwp_text_input_v1_reset(app.textV1);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(2, ime.activates);
}

TEST(InputMethodProtocol, TextInputV1FocusLossDeactivateAndSurfaceDestruction)
{
    ImeHarness h;
    Endpoint app(h, h.addClient()), ime(h, h.addClient());
    app.addTextV1();
    ime.addMethod();
    h.focus(app.client);
    app.activateV1();
    h.sync(ime.client);
    EXPECT_EQ(1, app.enters);
    EXPECT_EQ(1, ime.activates);

    h.focus(ime.client);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(1, app.leaves);
    EXPECT_EQ(1, ime.deactivates);
    zwp_input_method_v2_commit_string(ime.method, "wrong focus");
    zwp_input_method_v2_commit(ime.method, ime.serial);
    h.sync(ime.client);
    h.sync(app.client);
    EXPECT_TRUE(app.commit.empty());

    // Focus loss drops the activation, as in exo; the client must activate again.
    h.focus(app.client);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(1, app.enters);
    EXPECT_EQ(1, ime.activates);
    app.activateV1();
    h.sync(ime.client);
    EXPECT_EQ(2, app.enters);
    EXPECT_EQ(2, ime.activates);

    zwp_text_input_v1_deactivate(app.textV1, app.client.seat);
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(2, app.leaves);
    EXPECT_EQ(2, ime.deactivates);

    app.activateV1();
    h.sync(ime.client);
    EXPECT_EQ(3, ime.activates);
    wl_surface_destroy(app.client.surface);
    app.client.surface = nullptr;
    h.sync(app.client);
    h.sync(ime.client);
    EXPECT_EQ(3, ime.deactivates);
    EXPECT_EQ(3, app.leaves);
}

TEST(InputMethodProtocol, AuthorizerRestrictsInputMethodBinds)
{
    ImeHarness h;
    auto &trusted = h.addClient();
    h.manager.setAuthorizer([&](const wl_client *client) {
        return client == trusted.serverClient;
    });
    Endpoint ime(h, trusted);
    ime.addMethod();
    EXPECT_NE(nullptr, ime.method);

    auto &other = h.addClient();
    other.expectError = true;
    Endpoint app(h, other, false);
    ASSERT_NE(nullptr, app.textManager);
    ASSERT_NE(nullptr, app.textManagerV1);
    auto *registry = wl_display_get_registry(other.display);
    zwp_input_method_manager_v2 *bound = nullptr;
    static const wl_registry_listener listener = {
            [](void *data, wl_registry *r, uint32_t name, const char *interface, uint32_t) {
                if (!strcmp(interface, "zwp_input_method_manager_v2")) {
                    *static_cast<zwp_input_method_manager_v2 **>(data) =
                            static_cast<zwp_input_method_manager_v2 *>(wl_registry_bind(
                                    r, name, &zwp_input_method_manager_v2_interface, 1));
                }
            },
            [](void *, wl_registry *, uint32_t) {}};
    wl_registry_add_listener(registry, &listener, &bound);
    ASSERT_TRUE(h.until([&] {
        return wl_display_get_error(other.display) != 0;
    }));
    // The bind was refused with a protocol error; the proxies are still ours.
    if (bound)
        zwp_input_method_manager_v2_destroy(bound);
    wl_registry_destroy(registry);
}
