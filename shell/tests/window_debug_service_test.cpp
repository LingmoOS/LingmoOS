// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/dbus.h"
#include "base/json.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input_test_util.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/scene.h"
#include "session/window_debug_service.h"
#include "shm_test_util.h"
#include "wm/window.h"
#include "wm/window_manager.h"

#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>
#include <optional>

using namespace Lingmo;
namespace dbus = base::dbus;
using base::json::Value;

namespace {

class WindowDebugServiceTest : public testing::Test
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

        Desktop() { manager.setOutputBounds(base::Rect(0, 0, 1280, 800)); }
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

    // The service answers on this thread, so the wait has to keep the loop running.
    template<typename... Args>
    dbus::Reply call(const std::string &method, const Args &...args)
    {
        std::optional<dbus::Reply> reply;
        dbus::Proxy proxy(client.get(), "org.lingmo.WindowDebug1", "/org/lingmo/WindowDebug1",
                          "org.lingmo.WindowDebug1");
        auto pending = proxy.callAsync(
                method, [&reply](const dbus::Reply &answer) { reply = answer; }, args...);
        EXPECT_TRUE(test::runUntil([&] { return reply.has_value(); }));
        return reply.value_or(dbus::Reply());
    }

    bool callBool(const std::string &method, const auto &...args)
    {
        return call(method, args...).template value<bool>().value_or(false);
    }

    Value listWindows()
    {
        const std::string text = call("ListWindows").value<std::string>().value_or("");
        return Value::parse(text).value_or(Value());
    }
};

} // namespace

TEST_F(WindowDebugServiceTest, ListsTheWindowsBottomToTop)
{
    ASSERT_TRUE(client);
    Desktop desktop;
    wm::Window *editor = desktop.open("kate");
    wm::Window *viewer = desktop.open("loupe");
    editor->setTitle("notes.txt");
    session::WindowDebugService service(&desktop.manager);
    ASSERT_TRUE(service.registerService());

    const Value windows = listWindows();
    ASSERT_EQ(windows.size(), 2u);
    const Value &bottom = windows[size_t(0)];
    EXPECT_EQ(bottom["id"].toString(), std::to_string(editor->id()));
    EXPECT_EQ(bottom["appId"].toString(), "kate");
    EXPECT_EQ(bottom["title"].toString(), "notes.txt");
    EXPECT_EQ(bottom["state"].toString(), "normal");
    const Value &bounds = bottom["bounds"];
    EXPECT_EQ(base::Rect(int(bounds[size_t(0)].toInt()), int(bounds[size_t(1)].toInt()),
                    int(bounds[size_t(2)].toInt()), int(bounds[size_t(3)].toInt())),
              editor->bounds());
    EXPECT_EQ(windows[size_t(1)]["appId"].toString(), "loupe");
    EXPECT_EQ(windows[size_t(1)]["id"].toString(), std::to_string(viewer->id()));
    // Members sorted by name.
    const auto &members = bottom.toObject();
    EXPECT_TRUE(std::is_sorted(members.begin(), members.end(),
                               [](const auto &a, const auto &b) { return a.first < b.first; }));
}

TEST_F(WindowDebugServiceTest, MovesResizesAndActivatesAWindow)
{
    Desktop desktop;
    wm::Window *editor = desktop.open("kate");
    wm::Window *viewer = desktop.open("loupe");
    viewer->activate();
    session::WindowDebugService service(&desktop.manager);
    ASSERT_TRUE(service.registerService());
    const uint64_t id = editor->id();

    EXPECT_TRUE(callBool("MoveWindow", id, int32_t(40), int32_t(60)));
    EXPECT_EQ(editor->bounds().topLeft(), base::Point(40, 60));
    EXPECT_EQ(editor->bounds().size(), base::Size(200, 100));

    EXPECT_TRUE(callBool("SetWindowBounds", id, int32_t(100), int32_t(120), int32_t(300),
                         int32_t(200)));
    EXPECT_EQ(editor->bounds(), base::Rect(100, 120, 300, 200));

    EXPECT_TRUE(callBool("ActivateWindow", id));
    EXPECT_TRUE(editor->isActive());
    EXPECT_EQ(desktop.manager.windows().back(), editor);

    EXPECT_TRUE(callBool("SnapWindow", id, false));
    EXPECT_EQ(editor->stateType(), wm::WindowStateType::SecondarySnapped);
    EXPECT_TRUE(callBool("RestoreWindow", id));
    EXPECT_EQ(editor->stateType(), wm::WindowStateType::Normal);

    EXPECT_TRUE(callBool("MinimizeWindow", id));
    EXPECT_EQ(editor->stateType(), wm::WindowStateType::Minimized);
}

TEST_F(WindowDebugServiceTest, FeedsInputOnlyWhenGivenOne)
{
    Desktop desktop;
    session::WindowDebugService service(&desktop.manager);
    ASSERT_TRUE(service.registerService());
    EXPECT_FALSE(callBool("PointerMotion", 40.0, 30.0));

    input::InputManager input(&desktop.seat);
    input.setBounds({1280, 800});
    service.setInput(&input);
    EXPECT_TRUE(callBool("PointerMotion", 40.0, 30.0));
    EXPECT_EQ(desktop.seat.pointerLocation(), base::PointF(40, 30));
    EXPECT_TRUE(callBool("PointerButton", uint32_t(0x110), true));
    EXPECT_NE(desktop.seat.pressedButtons(), input::NoButton);
    EXPECT_TRUE(callBool("PointerButton", uint32_t(0x110), false));
    EXPECT_EQ(desktop.seat.pressedButtons(), input::NoButton);
}

// A real session: input only while root's switch file is there.
TEST_F(WindowDebugServiceTest, OnARealSessionTakesInputOnlyWithRootsFile)
{
    Desktop desktop;
    session::WindowDebugService service(&desktop.manager);
    ASSERT_TRUE(service.registerService());
    input::InputManager input(&desktop.seat);
    input.setBounds({1280, 800});
    service.setInput(&input);
    input_test::TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());

    // No file.
    service.setInputGate(directory.filePath("window-debug-input"));
    EXPECT_FALSE(callBool("PointerMotion", 40.0, 30.0));
    // A file the user made.
    ASSERT_TRUE(input_test::writeFile(directory.filePath("window-debug-input"), ""));
    EXPECT_FALSE(callBool("PointerMotion", 40.0, 30.0));
    EXPECT_FALSE(callBool("Key", uint32_t(30), true));
    // The user's link to a file of root's.
    std::filesystem::create_symlink("/etc/passwd", directory.filePath("link"));
    service.setInputGate(directory.filePath("link"));
    EXPECT_FALSE(callBool("PointerMotion", 40.0, 30.0));
    EXPECT_EQ(desktop.seat.pointerLocation(), base::PointF());

    // A file root owns and only root writes.
    service.setInputGate("/etc/passwd");
    EXPECT_TRUE(callBool("PointerMotion", 40.0, 30.0));
    EXPECT_EQ(desktop.seat.pointerLocation(), base::PointF(40, 30));
    EXPECT_TRUE(callBool("PointerButton", uint32_t(0x110), true));
    EXPECT_NE(desktop.seat.pressedButtons(), input::NoButton);
    EXPECT_TRUE(callBool("PointerButton", uint32_t(0x110), false));
    EXPECT_TRUE(callBool("Key", uint32_t(30), true));
    EXPECT_TRUE(callBool("Key", uint32_t(30), false));
}

TEST_F(WindowDebugServiceTest, AnUnknownIdChangesNothing)
{
    Desktop desktop;
    wm::Window *editor = desktop.open("kate");
    const base::Rect before = editor->bounds();
    session::WindowDebugService service(&desktop.manager);
    ASSERT_TRUE(service.registerService());

    const uint64_t missing = editor->id() + 1000;
    EXPECT_FALSE(callBool("MoveWindow", missing, int32_t(0), int32_t(0)));
    EXPECT_FALSE(callBool("ActivateWindow", missing));
    EXPECT_EQ(editor->bounds(), before);
}

TEST_F(WindowDebugServiceTest, IntrospectionNamesTheArguments)
{
    Desktop desktop;
    session::WindowDebugService service(&desktop.manager);
    ASSERT_TRUE(service.registerService());

    std::optional<dbus::Reply> reply;
    dbus::Proxy proxy(client.get(), "org.lingmo.WindowDebug1", "/org/lingmo/WindowDebug1",
                      "org.freedesktop.DBus.Introspectable");
    auto pending = proxy.callAsync("Introspect", [&reply](const dbus::Reply &answer) { reply = answer; });
    ASSERT_TRUE(test::runUntil([&] { return reply.has_value(); }));
    const std::string xml = reply->value<std::string>().value_or(std::string());
    // Parameters carry their names; sd-bus names every argument or none, so
    // the unnamed result is "result".
    EXPECT_NE(xml.find(R"(<arg type="t" name="id" direction="in"/>)"), std::string::npos) << xml;
    EXPECT_NE(xml.find(R"(<arg type="i" name="height" direction="in"/>)"), std::string::npos);
    EXPECT_NE(xml.find(R"(<arg type="b" name="result" direction="out"/>)"), std::string::npos);
}
