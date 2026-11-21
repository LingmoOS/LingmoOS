// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

#include "base/file.h"
#include "base/strings.h"
#include "clipboard_test_util.h"
#include "temporary_directory.h"
#include "wayland/input_method_launcher.h"
#include "wayland/privileged_client.h"
#include "wayland/protocols/input_method.h"

using namespace clipboard_test;

namespace {
using Lines = std::vector<std::string>;

Lines lines(const std::string &path)
{
    const std::optional<std::string> contents = base::file::read(path);
    if (!contents)
        return {};
    return base::strings::split(*contents, '\n', base::strings::SplitBehavior::SkipEmptyParts);
}

struct LauncherHarness : Harness {
    wayland::PrivilegedClients privileged {server.display()};
    scene::Node popupLayer;
    wayland::InputMethodManager manager {server.display(), &waylandSeat, &popupLayer};
    wayland::InputMethodLauncher launcher {server.display()};
    test::TemporaryDirectory directory;
    std::string log = directory.filePath("log");

    LauncherHarness()
    {
        const auto authorize = [this](const wl_client *client) {
            return launcher.isInputMethod(client);
        };
        manager.setAuthorizer(authorize);
        privileged.addRestrictedGlobal(manager.inputMethodGlobal(), authorize);
        launcher.setRestartDelay(10);
    }

    ~LauncherHarness()
    {
        launcher.setCommand({});
        wl_display_destroy_clients(server.display());
        privileged.removeRestrictedGlobal(manager.inputMethodGlobal());
        manager.shutdown();
    }

    std::vector<std::string> command(const char *mode) const
    {
        return {LINGMO_LAUNCHED_CLIENT, mode, log};
    }
};
} // namespace

TEST(InputMethodLauncher, OnlyTheLaunchedConnectionIsTheInputMethod)
{
    LauncherHarness h;
    int running = 0;
    const auto runningChanged = h.launcher.runningChanged.connect([&] {
        ++running;
    });
    h.launcher.setCommand(h.command("stay"));
    EXPECT_TRUE(h.launcher.isRunning());
    ASSERT_TRUE(h.until([&] {
        return lines(h.log).size() == 1;
    }));
    EXPECT_EQ(Lines({"ime"}), lines(h.log));

    auto &app = h.addClient();
    EXPECT_FALSE(h.launcher.isInputMethod(app.serverClient));
    bool visible = false;
    auto *registry = wl_display_get_registry(app.display);
    static const wl_registry_listener listener = {
            [](void *data, wl_registry *, uint32_t, const char *interface, uint32_t) {
                if (!strcmp(interface, "zwp_input_method_manager_v2"))
                    *static_cast<bool *>(data) = true;
            },
            [](void *, wl_registry *, uint32_t) {}};
    wl_registry_add_listener(registry, &listener, &visible);
    h.sync(app);
    EXPECT_FALSE(visible);
    wl_registry_destroy(registry);

    h.launcher.setCommand({});
    EXPECT_FALSE(h.launcher.isRunning());
    EXPECT_EQ(2, running);
}

TEST(InputMethodLauncher, RestartsALostConnectionWithALimit)
{
    LauncherHarness h;
    h.launcher.setCommand(h.command("exit"));
    ASSERT_TRUE(h.until(
            [&] {
                return lines(h.log).size() == 6 && !h.launcher.isRunning();
            },
            10000));
    h.until(
            [] {
                return false;
            },
            500);
    EXPECT_EQ(6u, lines(h.log).size());
    EXPECT_FALSE(h.launcher.isRunning());

    // Selecting the same method again restarts one that gave up.
    h.launcher.setCommand(h.command("stay"));
    ASSERT_TRUE(h.until([&] {
        return lines(h.log).size() == 7;
    }));
    EXPECT_TRUE(h.launcher.isRunning());
}

TEST(InputMethodLauncher, RestartTakesTheCurrentEnvironment)
{
    // How the input method learns an X11 display Xwayland only opened later.
    LauncherHarness h;
    std::vector<std::string> environment;
    for (char **entry = environ; *entry; ++entry) {
        if (!std::string_view(*entry).starts_with("DISPLAY="))
            environment.emplace_back(*entry);
    }
    h.launcher.setEnvironment(environment);
    h.launcher.setCommand(h.command("display"));
    ASSERT_TRUE(h.until([&] {
        return lines(h.log) == Lines({"-"}) && h.launcher.isRunning();
    }));
    environment.emplace_back("DISPLAY=:7");
    h.launcher.setEnvironment(environment);
    h.launcher.restart();
    ASSERT_TRUE(h.until([&] {
        return lines(h.log) == Lines({"-", ":7"}) && h.launcher.isRunning();
    }));
}

TEST(InputMethodLauncher, MissingProgramDoesNotLeaveAConnection)
{
    LauncherHarness h;
    h.launcher.setCommand({"/no/such/input-method"});
    ASSERT_TRUE(h.until(
            [&] {
                return !h.launcher.isRunning();
            },
            5000));
}
