// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <map>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <lingmo-status-item-v1-client-protocol.h>
#include <xdg-shell-client-protocol.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wayland/protocols/status_item.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/server.h"
#include "wm/shell_surface.h"
#include "wm/status_items.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 800, 600);
constexpr int kBarHeight = 32;

struct Item {
    wl_surface *surface = nullptr;
    lingmo_status_item_v1 *item = nullptr;
    wl_buffer *buffer = nullptr;
    struct Configure {
        uint32_t serial;
        int height;
        int maxWidth;
    };
    std::vector<Configure> configures;
    std::vector<std::pair<uint32_t, uint32_t>> appearances;
    std::vector<uint32_t> highlights;
    std::vector<base::Rect> places;
};

const lingmo_status_item_v1_listener itemListener = {
        [](void *data, lingmo_status_item_v1 *, uint32_t serial, int32_t height, int32_t maxWidth) {
            static_cast<Item *>(data)->configures.push_back({serial, height, maxWidth});
        },
        [](void *data, lingmo_status_item_v1 *, uint32_t appearance, uint32_t foreground) {
            static_cast<Item *>(data)->appearances.emplace_back(appearance, foreground);
        },
        [](void *data, lingmo_status_item_v1 *, uint32_t highlighted) {
            static_cast<Item *>(data)->highlights.push_back(highlighted);
        },
        [](void *data, lingmo_status_item_v1 *, int32_t x, int32_t y, int32_t width, int32_t height) {
            static_cast<Item *>(data)->places.emplace_back(x, y, width, height);
        },
};

struct Popup {
    wl_surface *surface = nullptr;
    xdg_surface *xdgSurface = nullptr;
    xdg_popup *popup = nullptr;
    wl_buffer *buffer = nullptr;
    std::vector<uint32_t> surfaceConfigures;
    std::vector<base::Rect> configures;
    int dones = 0;
};

const xdg_surface_listener popupSurfaceListener = {
        [](void *data, xdg_surface *, uint32_t serial) {
            static_cast<Popup *>(data)->surfaceConfigures.push_back(serial);
        },
};

const xdg_popup_listener popupListener = {
        [](void *data, xdg_popup *, int32_t x, int32_t y, int32_t width, int32_t height) {
            static_cast<Popup *>(data)->configures.emplace_back(x, y, width, height);
        },
        [](void *data, xdg_popup *) {
            ++static_cast<Popup *>(data)->dones;
        },
        [](void *, xdg_popup *, uint32_t) {},
};

struct Client {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_compositor *compositor = nullptr;
    wl_shm *shm = nullptr;
    wl_seat *seat = nullptr;
    wl_pointer *pointer = nullptr;
    wl_keyboard *keyboard = nullptr;
    xdg_wm_base *wmBase = nullptr;
    lingmo_status_item_manager_v1 *items = nullptr;
    std::map<std::string, std::pair<uint32_t, uint32_t>> globals;
    std::vector<std::unique_ptr<Item>> made;
    std::vector<std::unique_ptr<Popup>> popups;

    uint32_t lastButtonSerial = 0;
    int buttonPresses = 0;
    wl_surface *pointerFocus = nullptr;
    wl_surface *keyboardFocus = nullptr;

    explicit Client(int fd)
        : display(wl_display_connect_to_fd(fd))
    {
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *, uint32_t name, const char *interface, uint32_t version) {
                    static_cast<Client *>(data)->globals[interface] = {name, version};
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        wl_display_roundtrip(display);
        compositor = static_cast<wl_compositor *>(
                wl_registry_bind(registry, globals["wl_compositor"].first, &wl_compositor_interface, 4));
        shm = static_cast<wl_shm *>(wl_registry_bind(registry, globals["wl_shm"].first, &wl_shm_interface, 1));
        wmBase = static_cast<xdg_wm_base *>(
                wl_registry_bind(registry, globals["xdg_wm_base"].first, &xdg_wm_base_interface, 3));
        items = static_cast<lingmo_status_item_manager_v1 *>(
                wl_registry_bind(registry, globals["lingmo_status_item_manager_v1"].first,
                                 &lingmo_status_item_manager_v1_interface, 1));
        seat = static_cast<wl_seat *>(wl_registry_bind(registry, globals["wl_seat"].first, &wl_seat_interface, 5));
        pointer = wl_seat_get_pointer(seat);
        static const wl_pointer_listener pointerListener = {
                [](void *data, wl_pointer *, uint32_t, wl_surface *surface, wl_fixed_t, wl_fixed_t) {
                    static_cast<Client *>(data)->pointerFocus = surface;
                },
                [](void *data, wl_pointer *, uint32_t, wl_surface *) {
                    static_cast<Client *>(data)->pointerFocus = nullptr;
                },
                [](void *, wl_pointer *, uint32_t, wl_fixed_t, wl_fixed_t) {},
                [](void *data, wl_pointer *, uint32_t serial, uint32_t, uint32_t, uint32_t state) {
                    auto *client = static_cast<Client *>(data);
                    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
                        client->lastButtonSerial = serial;
                        ++client->buttonPresses;
                    }
                },
                [](void *, wl_pointer *, uint32_t, uint32_t, wl_fixed_t) {},
                [](void *, wl_pointer *) {},
                [](void *, wl_pointer *, uint32_t) {},
                [](void *, wl_pointer *, uint32_t, uint32_t) {},
                [](void *, wl_pointer *, uint32_t, int32_t) {},
                nullptr,
                nullptr,
        };
        wl_pointer_add_listener(pointer, &pointerListener, this);
        keyboard = wl_seat_get_keyboard(seat);
        static const wl_keyboard_listener keyboardListener = {
                [](void *, wl_keyboard *, uint32_t, int32_t fd, uint32_t) { close(fd); },
                [](void *data, wl_keyboard *, uint32_t, wl_surface *surface, wl_array *) {
                    static_cast<Client *>(data)->keyboardFocus = surface;
                },
                [](void *data, wl_keyboard *, uint32_t, wl_surface *) {
                    static_cast<Client *>(data)->keyboardFocus = nullptr;
                },
                [](void *, wl_keyboard *, uint32_t, uint32_t, uint32_t, uint32_t) {},
                [](void *, wl_keyboard *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {},
                [](void *, wl_keyboard *, int32_t, int32_t) {},
        };
        wl_keyboard_add_listener(keyboard, &keyboardListener, this);
        wl_display_roundtrip(display);
    }

    ~Client()
    {
        for (auto &popup : popups) {
            if (popup->popup)
                xdg_popup_destroy(popup->popup);
            xdg_surface_destroy(popup->xdgSurface);
            wl_surface_destroy(popup->surface);
            if (popup->buffer)
                wl_buffer_destroy(popup->buffer);
        }
        for (auto &item : made) {
            if (item->item)
                lingmo_status_item_v1_destroy(item->item);
            wl_surface_destroy(item->surface);
            if (item->buffer)
                wl_buffer_destroy(item->buffer);
        }
        wl_keyboard_destroy(keyboard);
        wl_pointer_destroy(pointer);
        wl_seat_destroy(seat);
        lingmo_status_item_manager_v1_destroy(items);
        xdg_wm_base_destroy(wmBase);
        wl_shm_destroy(shm);
        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    wl_buffer *makeBuffer(const base::Size &size)
    {
        const int stride = size.width() * 4;
        const size_t bytes = size_t(stride) * size_t(size.height());
        const int fd = testsupport::makeMemfd(bytes);
        wl_shm_pool *pool = wl_shm_create_pool(shm, fd, int32_t(bytes));
        wl_buffer *buffer =
                wl_shm_pool_create_buffer(pool, 0, size.width(), size.height(), stride, WL_SHM_FORMAT_ARGB8888);
        wl_shm_pool_destroy(pool);
        close(fd);
        return buffer;
    }

    Item *makeItem(const char *identifier = nullptr)
    {
        made.push_back(std::make_unique<Item>());
        Item *item = made.back().get();
        item->surface = wl_compositor_create_surface(compositor);
        item->item = lingmo_status_item_manager_v1_get_status_item(items, item->surface);
        lingmo_status_item_v1_add_listener(item->item, &itemListener, item);
        if (identifier)
            lingmo_status_item_v1_set_identifier(item->item, identifier);
        sync();
        return item;
    }

    // Answers the last configure with a buffer `width` wide.
    void draw(Item *item, int width)
    {
        const Item::Configure configure = item->configures.back();
        lingmo_status_item_v1_ack_configure(item->item, configure.serial);
        if (item->buffer)
            wl_buffer_destroy(item->buffer);
        item->buffer = makeBuffer({width, configure.height});
        wl_surface_attach(item->surface, item->buffer, 0, 0);
        wl_surface_damage(item->surface, 0, 0, width, configure.height);
        wl_surface_commit(item->surface);
        sync();
    }

    // A popup of `size` hung from the item, centred under it.
    Popup *openPopup(Item *item, const base::Size &itemSize, const base::Size &size, bool grab)
    {
        popups.push_back(std::make_unique<Popup>());
        Popup *popup = popups.back().get();
        popup->surface = wl_compositor_create_surface(compositor);
        popup->xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, popup->surface);
        xdg_surface_add_listener(popup->xdgSurface, &popupSurfaceListener, popup);
        xdg_positioner *positioner = xdg_wm_base_create_positioner(wmBase);
        xdg_positioner_set_size(positioner, size.width(), size.height());
        xdg_positioner_set_anchor_rect(positioner, 0, 0, itemSize.width(), itemSize.height());
        xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_BOTTOM);
        xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM);
        popup->popup = xdg_surface_get_popup(popup->xdgSurface, nullptr, positioner);
        xdg_positioner_destroy(positioner);
        xdg_popup_add_listener(popup->popup, &popupListener, popup);
        if (grab)
            xdg_popup_grab(popup->popup, seat, lastButtonSerial);
        lingmo_status_item_v1_get_popup(item->item, popup->popup);
        sync();
        if (!popup->surfaceConfigures.empty()) {
            xdg_surface_ack_configure(popup->xdgSurface, popup->surfaceConfigures.back());
            popup->buffer = makeBuffer(size);
            wl_surface_attach(popup->surface, popup->buffer, 0, 0);
            wl_surface_commit(popup->surface);
            sync();
        }
        return popup;
    }

    void sync() { wl_display_roundtrip(display); }

    uint32_t syncExpectingError(const wl_interface **interface = nullptr)
    {
        if (wl_display_roundtrip(display) >= 0)
            return UINT32_MAX;
        if (wl_display_get_error(display) != EPROTO)
            return UINT32_MAX;
        const wl_interface *raisedOn = nullptr;
        uint32_t id = 0;
        const uint32_t code = wl_display_get_protocol_error(display, &raisedOn, &id);
        if (interface)
            *interface = raisedOn;
        return code;
    }
};

class StatusItemProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *cursorContainer = scene.root()->addChild(std::make_unique<scene::Node>("cursor"));
    compositor::Compositor compositor {surfaceContainer};
    input::Seat seat {&compositor};
    input::InputManager manager {&seat};
    wm::WindowManager windowManager {&compositor, &seat};
    wayland::Server server;
    testsupport::FakeRenderer renderer;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    // The bar: a panel across the top, as the shell's.
    std::unique_ptr<compositor::Surface> barSurface;
    std::unique_ptr<wm::ShellSurface> barShell;

    std::mutex mutex;
    std::function<void()> pendingCheck;

    wm::StatusItems *items() { return windowManager.statusItems(); }

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        seat.setCursorContainer(cursorContainer);
        manager.setBounds(kOutput.size());
        manager.setEventSink(&windowManager);
        windowManager.setOutputBounds(kOutput);
    }

    void TearDown() override
    {
        if (barSurface)
            items()->setBar(nullptr, nullptr);
        barShell.reset();
        barSurface.reset();
    }

    // Maps the bar and hands it to the status items.
    void showBar()
    {
        barSurface = compositor.createSurface();
        barShell = std::make_unique<wm::ShellSurface>(&windowManager, barSurface.get());
        wm::ShellWindowMetadata metadata;
        metadata.role = wm::WindowRole::Panel;
        metadata.focusPolicy = wm::FocusPolicy::NoFocus;
        windowManager.setShellMetadata(barSurface.get(), metadata);
        barShell->setToplevel();
        commitBar();
        ASSERT_TRUE(barShell->window());
        barShell->window()->setBounds(base::Rect(0, 0, kOutput.width(), kBarHeight));
        items()->setBarOrigin({0, 0});
        items()->setLimits(kBarHeight, 300);
        items()->setAppearance(true, 0xffffffff);
        items()->setBar(barSurface.get(), barShell.get());
    }

    // What the bar's frames do: its commit applies where its items are.
    void commitBar()
    {
        barSurface->attach(pool->createBuffer({kOutput.width(), kBarHeight}, 0, kOutput.width() * 4,
                                              render::PixelFormat::Bgra8888, nullptr),
                           {});
        barSurface->damage(base::Rect(0, 0, kOutput.width(), kBarHeight));
        barSurface->commit();
        compositor.updateTextures(renderer);
    }

    // The bar's layout: the shown items right to left from `right`, 6 apart.
    void layOut(int right = 700)
    {
        int x = right;
        const std::vector<wm::StatusItem *> shown = [&] {
            std::vector<wm::StatusItem *> list;
            for (wm::StatusItem *item : items()->items()) {
                if (item->isShown())
                    list.push_back(item);
            }
            return list;
        }();
        for (auto it = shown.rbegin(); it != shown.rend(); ++it) {
            x -= (*it)->width() + 6;
            (*it)->place({x + 3, 0});
        }
        commitBar();
    }

    void check(std::function<void()> body)
    {
        std::promise<void> done;
        auto future = done.get_future();
        {
            std::lock_guard lock(mutex);
            pendingCheck = [&] {
                body();
                done.set_value();
            };
        }
        future.wait();
    }

    void run(std::function<void(Client &)> body)
    {
        auto waylandSeat = std::make_unique<wayland::WaylandSeat>(&seat, server.serialTracker());
        auto xdgShell = std::make_unique<wayland::WaylandXdgShell>(&compositor, &windowManager, &seat,
                                                                   server.serialTracker());
        ASSERT_TRUE(wayland::createCompositorGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createShmGlobal(server.display(), &compositor));
        ASSERT_TRUE(wayland::createSeatGlobal(server.display(), waylandSeat.get()));
        ASSERT_TRUE(wayland::createXdgShellGlobal(server.display(), xdgShell.get()));
        ASSERT_TRUE(wayland::createStatusItemGlobal(server.display(), items()));

        int fds[2];
        ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds));
        ASSERT_TRUE(wl_client_create(server.display(), fds[0]));

        std::atomic<bool> done = false;
        std::thread client([&] {
            {
                Client c(fds[1]);
                body(c);
            }
            done = true;
        });

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!done) {
            server.dispatch(std::chrono::milliseconds(1));
            server.flush();
            std::function<void()> work;
            {
                std::lock_guard lock(mutex);
                work.swap(pendingCheck);
            }
            if (work)
                work();
            if (std::chrono::steady_clock::now() > deadline) {
                ADD_FAILURE() << "client timed out";
                shutdown(fds[1], SHUT_RDWR);
                break;
            }
        }
        client.join();
        wl_display_destroy_clients(server.display());
        EXPECT_TRUE(items()->items().empty());
        EXPECT_TRUE(windowManager.popupGrabs().empty());
    }
};

TEST_F(StatusItemProtocolTest, AnItemIsConfiguredOnceThereIsABarAndPlacedOnceItDraws)
{
    run([&](Client &c) {
        Item *item = c.makeItem();
        EXPECT_TRUE(item->configures.empty());
        check([&] { showBar(); });
        c.sync();
        ASSERT_EQ(1u, item->configures.size());
        EXPECT_EQ(kBarHeight, item->configures.back().height);
        EXPECT_EQ(300, item->configures.back().maxWidth);
        ASSERT_EQ(1u, item->appearances.size());
        EXPECT_EQ(uint32_t(LINGMO_STATUS_ITEM_V1_APPEARANCE_DARK), item->appearances.back().first);
        EXPECT_EQ(0xffffffffu, item->appearances.back().second);
        ASSERT_EQ(1u, item->highlights.size());
        EXPECT_EQ(0u, item->highlights.back());

        check([&] {
            ASSERT_EQ(1u, items()->items().size());
            EXPECT_FALSE(items()->items().front()->isShown());
        });
        c.draw(item, 40);
        check([&] {
            wm::StatusItem *model = items()->items().front();
            EXPECT_TRUE(model->isShown());
            EXPECT_EQ(40, model->width());
            layOut();
            EXPECT_EQ(barSurface.get(), model->surface()->parentSurface());
            EXPECT_EQ(base::Rect(657, 0, 40, kBarHeight), model->screenRect());
        });
        c.sync();
        ASSERT_FALSE(item->places.empty());
        EXPECT_EQ(base::Rect(657, 0, 40, kBarHeight), item->places.back());

        // The look and the height follow the bar's.
        check([&] {
            items()->setAppearance(false, 0xd9000000);
            items()->setLimits(28, 300);
        });
        c.sync();
        EXPECT_EQ(uint32_t(LINGMO_STATUS_ITEM_V1_APPEARANCE_LIGHT), item->appearances.back().first);
        EXPECT_EQ(0xd9000000u, item->appearances.back().second);
        EXPECT_EQ(28, item->configures.back().height);
    });
}

TEST_F(StatusItemProtocolTest, PointerInputOverTheItemReachesItsClient)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        c.draw(item, 40);
        check([&] {
            layOut();
            manager.onPointerMotionAbsolute(at(1), base::PointF(670, 10));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(item->surface, c.pointerFocus);
        EXPECT_EQ(1, c.buttonPresses);
    });
}

TEST_F(StatusItemProtocolTest, ABufferBeforeTheFirstConfigureIsAProtocolError)
{
    run([&](Client &c) {
        Item *item = c.makeItem();
        item->buffer = c.makeBuffer({20, 20});
        wl_surface_attach(item->surface, item->buffer, 0, 0);
        wl_surface_commit(item->surface);
        const wl_interface *interface = nullptr;
        EXPECT_EQ(uint32_t(LINGMO_STATUS_ITEM_V1_ERROR_UNCONFIGURED_BUFFER), c.syncExpectingError(&interface));
        EXPECT_EQ(&lingmo_status_item_v1_interface, interface);
    });
}

TEST_F(StatusItemProtocolTest, ASurfaceWithARoleCannotBecomeAnItem)
{
    run([&](Client &c) {
        wl_surface *surface = wl_compositor_create_surface(c.compositor);
        xdg_surface *xdgSurface = xdg_wm_base_get_xdg_surface(c.wmBase, surface);
        xdg_toplevel *toplevel = xdg_surface_get_toplevel(xdgSurface);
        lingmo_status_item_v1 *item = lingmo_status_item_manager_v1_get_status_item(c.items, surface);
        const wl_interface *interface = nullptr;
        EXPECT_EQ(uint32_t(LINGMO_STATUS_ITEM_MANAGER_V1_ERROR_ROLE), c.syncExpectingError(&interface));
        EXPECT_EQ(&lingmo_status_item_manager_v1_interface, interface);
        lingmo_status_item_v1_destroy(item);
        xdg_toplevel_destroy(toplevel);
        xdg_surface_destroy(xdgSurface);
        wl_surface_destroy(surface);
    });
}

TEST_F(StatusItemProtocolTest, APopupHangsCentredUnderTheItemAndHighlightsIt)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        c.draw(item, 40);
        check([&] {
            layOut();
            manager.onPointerMotionAbsolute(at(1), base::PointF(670, 10));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();

        Popup *popup = c.openPopup(item, {40, kBarHeight}, {100, 60}, true);
        // Relative to the bar: the item's centre, 677, less half the popup.
        ASSERT_EQ(1u, popup->configures.size());
        EXPECT_EQ(base::Rect(627, kBarHeight, 100, 60), popup->configures.back());
        check([&] {
            ASSERT_EQ(1u, windowManager.popupGrabs().size());
            wm::ShellSurface *grab = windowManager.popupGrabs().front();
            EXPECT_EQ(barShell.get(), grab->popupParent());
            ASSERT_TRUE(grab->window());
            EXPECT_EQ(base::Rect(627, kBarHeight, 100, 60), grab->window()->bounds());
            EXPECT_TRUE(items()->items().front()->isHighlighted());
            // The popup's keys go to the item's client.
            EXPECT_EQ(items()->items().front()->surface(), seat.focusedSurface());
        });
        c.sync();
        EXPECT_EQ(1u, item->highlights.back());
        EXPECT_EQ(item->surface, c.keyboardFocus);

        // A press on the item reaches it and keeps the popup.
        check([&] {
            manager.onPointerMotionAbsolute(at(4), base::PointF(680, 12));
            manager.onPointerButton(at(5), BTN_LEFT, true);
            manager.onPointerButton(at(6), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(2, c.buttonPresses);
        EXPECT_EQ(0, popup->dones);

        // A press on the bar beside it dismisses the popup.
        check([&] {
            manager.onPointerMotionAbsolute(at(7), base::PointF(100, 12));
            manager.onPointerButton(at(8), BTN_LEFT, true);
            manager.onPointerButton(at(9), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(1, popup->dones);
        EXPECT_EQ(0u, item->highlights.back());
        check([&] {
            EXPECT_TRUE(windowManager.popupGrabs().empty());
            EXPECT_FALSE(items()->items().front()->isHighlighted());
        });
    });
}

TEST_F(StatusItemProtocolTest, APressAwayFromTheBarDismissesThePopupAndStaysThere)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        c.draw(item, 40);
        check([&] {
            layOut();
            manager.onPointerMotionAbsolute(at(1), base::PointF(670, 10));
            manager.onPointerButton(at(2), BTN_LEFT, true);
            manager.onPointerButton(at(3), BTN_LEFT, false);
        });
        c.sync();
        Popup *popup = c.openPopup(item, {40, kBarHeight}, {100, 60}, true);
        check([&] {
            manager.onPointerMotionAbsolute(at(4), base::PointF(300, 400));
            manager.onPointerButton(at(5), BTN_LEFT, true);
            manager.onPointerButton(at(6), BTN_LEFT, false);
        });
        c.sync();
        EXPECT_EQ(1, popup->dones);
        EXPECT_EQ(1, c.buttonPresses);
    });
}

TEST_F(StatusItemProtocolTest, APopupOfAnItemWithoutAPlaceIsDismissedAtOnce)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        Popup *popup = c.openPopup(item, {40, kBarHeight}, {100, 60}, false);
        EXPECT_EQ(1, popup->dones);
        EXPECT_TRUE(popup->configures.empty());
    });
}

TEST_F(StatusItemProtocolTest, AParentedPopupCannotBeHungFromAnItem)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        c.draw(item, 40);
        check([&] { layOut(); });
        Popup *popup = c.openPopup(item, {40, kBarHeight}, {100, 60}, false);
        ASSERT_EQ(1u, popup->configures.size());
        lingmo_status_item_v1_get_popup(item->item, popup->popup);
        const wl_interface *interface = nullptr;
        EXPECT_EQ(uint32_t(LINGMO_STATUS_ITEM_V1_ERROR_INVALID_POPUP), c.syncExpectingError(&interface));
        EXPECT_EQ(&lingmo_status_item_v1_interface, interface);
    });
}

TEST_F(StatusItemProtocolTest, DestroyingTheItemTakesItOutAndDismissesItsPopups)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        c.draw(item, 40);
        check([&] { layOut(); });
        Popup *popup = c.openPopup(item, {40, kBarHeight}, {100, 60}, false);
        ASSERT_EQ(1u, popup->configures.size());
        lingmo_status_item_v1_destroy(item->item);
        item->item = nullptr;
        c.sync();
        EXPECT_EQ(1, popup->dones);
        check([&] { EXPECT_TRUE(items()->items().empty()); });
    });
}

TEST_F(StatusItemProtocolTest, AskingForTheHighlightShowsIt)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *item = c.makeItem();
        lingmo_status_item_v1_set_highlight(item->item, 1);
        c.sync();
        EXPECT_EQ(1u, item->highlights.back());
        check([&] { EXPECT_TRUE(items()->items().front()->isHighlighted()); });
        lingmo_status_item_v1_set_highlight(item->item, 2);
        EXPECT_EQ(uint32_t(LINGMO_STATUS_ITEM_V1_ERROR_INVALID_VALUE), c.syncExpectingError());
    });
}

TEST_F(StatusItemProtocolTest, NewItemsStandLeftmostAndComeBackToTheirPlaceByIdentifier)
{
    run([&](Client &c) {
        check([&] { showBar(); });
        Item *first = c.makeItem("first");
        Item *second = c.makeItem("second");
        Item *third = c.makeItem("third");
        const auto order = [&] {
            std::vector<int> widths;
            check([&] {
                for (wm::StatusItem *item : items()->items())
                    widths.push_back(item->width());
            });
            return widths;
        };
        c.draw(first, 10);
        c.draw(second, 20);
        c.draw(third, 30);
        EXPECT_EQ((std::vector<int> {30, 20, 10}), order());

        // The middle one goes and comes back between the other two.
        lingmo_status_item_v1_destroy(second->item);
        second->item = nullptr;
        c.sync();
        EXPECT_EQ((std::vector<int> {30, 10}), order());
        Item *again = c.makeItem("second");
        c.draw(again, 21);
        EXPECT_EQ((std::vector<int> {30, 21, 10}), order());
    });
}

} // namespace
