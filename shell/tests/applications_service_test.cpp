// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/dbus.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "fake_renderer.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/scene.h"
#include "session/applications_service.h"
#include "shm_test_util.h"
#include "wm/quit_all.h"
#include "wm/window.h"
#include "wm/window_manager.h"

#include <gtest/gtest.h>
#include <optional>

using namespace Lingmo;
namespace dbus = base::dbus;

namespace {

class ApplicationsServiceTest : public testing::Test
{
protected:
    std::unique_ptr<dbus::Connection> client = dbus::Connection::open(dbus::Connection::Bus::Session, core::mainLoop());

    struct Desktop {
        scene::Scene scene;
        scene::Node *surfaces =
                scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
        compositor::Compositor compositor {surfaces};
        testsupport::FakeRenderer renderer;
        input::Seat seat {&compositor};
        wm::WindowManager manager {&compositor, &seat};
        std::vector<std::unique_ptr<compositor::Surface>> owned;
        std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

        ~Desktop() { seat.shutdown(); }

        wm::Window *open(const std::string &applicationId)
        {
            owned.push_back(compositor.createSurface());
            auto *surface = owned.back().get();
            auto buffer = pool->createBuffer({200, 100}, 0, 800, render::PixelFormat::Bgrx8888,
                                             nullptr);
            surface->attach(buffer, {});
            surface->commit();
            compositor.updateTextures(renderer);
            return manager.createWindow(surface, {.requestedSize = {200, 100},
                                                  .applicationId = applicationId,
                                                  .closeCallback = [] {}});
        }
    };

    struct Pending {
        std::optional<dbus::Reply> reply;
        dbus::PendingCall call;
    };

    std::unique_ptr<Pending> quitAll()
    {
        auto pending = std::make_unique<Pending>();
        Pending *raw = pending.get();
        dbus::Proxy proxy(client.get(), "org.lingmo.Applications1", "/org/lingmo/Applications1",
                          "org.lingmo.Applications1");
        pending->call = proxy.callAsync("QuitAll", [raw](const dbus::Reply &reply) { raw->reply = reply; });
        return pending;
    }

    bool waitFor(const std::function<bool()> &done) { return test::runUntil(done); }
};

} // namespace

TEST_F(ApplicationsServiceTest, RepliesOnceEveryApplicationHasQuit)
{
    ASSERT_TRUE(client);
    Desktop desktop;
    wm::Window *editor = desktop.open("kate");
    wm::QuitAll quit(&desktop.manager);
    std::vector<bool> ending;
    session::ApplicationsService service(&quit, [&ending](bool value) { ending.push_back(value); });
    ASSERT_TRUE(service.registerService());

    auto first = quitAll();
    auto second = quitAll();
    EXPECT_TRUE(waitFor([&] { return quit.isRunning(); }));
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_FALSE(first->reply);
    EXPECT_EQ(ending, std::vector<bool> {true});

    desktop.manager.destroyWindow(editor);
    EXPECT_TRUE(waitFor([&] { return first->reply && second->reply; }));
    for (auto *pending : {first.get(), second.get()}) {
        ASSERT_TRUE(pending->reply);
        std::string outcome, applicationId;
        EXPECT_TRUE(pending->reply->read(outcome, applicationId));
        EXPECT_EQ(outcome, "quit");
    }
    // The session ends from here, with the docks still away.
    EXPECT_EQ(ending, std::vector<bool> {true});
}

TEST_F(ApplicationsServiceTest, ACancelledQuitBringsTheDocksBack)
{
    Desktop desktop;
    desktop.open("frozen");
    int64_t now = 0;
    wm::QuitAll quit(&desktop.manager, [&now] { return now; });
    std::vector<bool> ending;
    session::ApplicationsService service(&quit, [&ending](bool value) { ending.push_back(value); });
    ASSERT_TRUE(service.registerService());

    auto call = quitAll();
    EXPECT_TRUE(waitFor([&] { return quit.isRunning(); }));
    now = wm::QuitAll::kUnresponsiveAfterMs + 1;
    ASSERT_TRUE(waitFor([&] { return call->reply.has_value(); }));
    std::string outcome, applicationId;
    EXPECT_TRUE(call->reply->read(outcome, applicationId));
    EXPECT_EQ(outcome, "unresponsive");
    EXPECT_EQ(applicationId, "frozen");
    EXPECT_EQ(ending, (std::vector<bool> {true, false}));
}
