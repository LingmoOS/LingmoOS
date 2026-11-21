// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <filesystem>

#include <gtest/gtest.h>
#include <unistd.h>

#include "base/settings.h"
#include "dbus_test_client.h"
#include "loop_test_util.h"
#include "shell/common/app_launcher.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/dock/dock.h"
#include "shell_test_util.h"
#include "temporary_directory.h"

using namespace Lingmo;
namespace dbus = base::dbus;
using shell::DockModel;

namespace {

// The session's AppRuntime, as far as starting applications goes.
class FakeAppRuntime
{
public:
    explicit FakeAppRuntime(dbus::Connection *bus)
        : object(bus, "/AppRuntime", "org.lingmo.AppRuntime")
    {
        object.addMethod("launchApplication",
                         [this](const std::string &id, const std::vector<std::string> &, const std::string &) {
                             launched.push_back(id);
                             return uint32_t(1);
                         });
        object.addMethod("launchCommand", [this](const std::string &id, const std::vector<std::string> &,
                                                 const std::string &, const std::string &) {
            launched.push_back(id);
            return uint32_t(1);
        });
        object.addMethod("quitApplication", [this](const std::string &id) {
            quit.push_back(id);
            return true;
        });
        EXPECT_TRUE(object.publish());
        EXPECT_TRUE(bus->requestName("org.lingmo.AppRuntime"));
    }

    dbus::Object object;
    std::vector<std::string> launched;
    std::vector<std::string> quit;
};

class DockTest : public testing::Test
{
protected:
    DockTest()
    {
        applications.addSimple("files", "Files", "files-icon");
        applications.addSimple("terminal", "Terminal", "terminal-icon");
        applications.addSimple("editor", "Editor", "editor-icon");
        applications.addSimple("lingmo-settings", "Settings", "settings-icon");
        applications.addSimple("lingmo-filemanager", "Files", "files-icon");
        applications.scan();
    }

    void start(const std::string &settings = {})
    {
        if (!settings.empty())
            directory.write("shell.conf", settings);
        trashHome = directory.filePath("Trash");
        dock = std::make_unique<shell::Dock>(
                dbus::Connection::session(core::mainLoop()), tasks, launcher, directory.filePath("shell.conf"),
                std::make_unique<shell::TrashMonitor>(directory.filePath("mountinfo"), trashHome));
        ASSERT_TRUE(test::runUntil([this] { return model().isLoaded() && dock->settings().isReady(); }));
    }

    DockModel &model() { return dock->applications(); }

    std::vector<std::string> ids()
    {
        std::vector<std::string> list;
        for (const auto &item : model().items())
            list.push_back(item->id);
        return list;
    }

    std::vector<std::string> savedPins()
    {
        shell::waitForFileThread();
        return base::Settings(directory.filePath("shell.conf")).stringListValue("Dock/Pinned");
    }

    uint64_t open(const std::string &appId, const std::string &title = "Window")
    {
        shell::TaskWindow window;
        window.appId = appId;
        window.title = title;
        window.pid = 4242;
        return windows.add(window);
    }

    test::TemporaryDirectory directory;
    std::string trashHome;
    test::FakeApplications applications;
    test::FakeTaskWindows windows;
    shell::TaskManager tasks{windows, applications, [](int64_t) { return shell::ProcessCommand(); }};
    test::DBusTestClient runtimeClient;
    FakeAppRuntime runtime{runtimeClient.connection.get()};
    shell::ApplicationLauncher launcher{dbus::Connection::session(core::mainLoop())};
    std::unique_ptr<shell::Dock> dock;
};

} // namespace

TEST_F(DockTest, LoadsThePinsAndForgetsUninstalledOnes)
{
    start("[Dock]\nPinned=terminal, gone, files\n");
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "terminal", "files"}));
    EXPECT_TRUE(model().at(0).fixed);
    EXPECT_EQ(model().at(1).iconName, "terminal-icon");
    EXPECT_EQ(model().pinnedCount(), 3);
    EXPECT_EQ(savedPins(), (std::vector<std::string>{"terminal", "files"}));

    // Missing keys were written with their defaults.
    base::Settings settings(directory.filePath("shell.conf"));
    EXPECT_EQ(settings.intValue("Dock/IconSize"), 54);
    EXPECT_EQ(settings.intValue("Dock/Direction"), 1);
    EXPECT_TRUE(settings.boolValue("Dock/MinimizedArea"));
}

TEST_F(DockTest, StartsFromTheDefaultPinsWithoutAList)
{
    applications.addSimple("lingmo-terminal", "Terminal");
    applications.scan();
    start();
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "lingmo-terminal"}));
    // Nothing is written until the user changes the pins.
    shell::waitForFileThread();
    EXPECT_FALSE(base::Settings(directory.filePath("shell.conf")).contains("Dock/Pinned"));
}

TEST_F(DockTest, FollowsRunningApplications)
{
    start("[Dock]\nPinned=files\n");
    const uint64_t first = open("files", "Home");
    const uint64_t second = open("files", "Documents");
    const uint64_t loose = open("unknown", "Tool");
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "files", "unknown"}));
    EXPECT_EQ(model().at(1).windows, (std::vector<uint64_t>{first, second}));
    EXPECT_EQ(model().at(2).visibleName, "Tool");
    EXPECT_EQ(model().at(2).iconName, "unknown");
    EXPECT_FALSE(model().at(2).pinnable());

    windows.setActive(second);
    EXPECT_TRUE(model().at(1).isActive);

    // Clicking the active application's icon minimizes that window.
    windows.requests.clear();
    model().clicked("files");
    EXPECT_EQ(windows.requests, (std::vector<std::string>{"minimize " + std::to_string(second)}));

    // With no active window, the next click restores the same window.
    windows.setActive(0);
    windows.requests.clear();
    model().clicked("files");
    EXPECT_EQ(windows.requests, (std::vector<std::string>{"activate " + std::to_string(second)}));

    // A hidden application comes back whole.
    windows.change(first, [](shell::TaskWindow &w) { w.hidden = true; }, shell::kStateChange);
    windows.requests.clear();
    model().clicked("files");
    EXPECT_EQ(windows.requests.front(), "unhide " + std::to_string(first));

    // Unpinned applications go with their last window; pinned ones stay.
    windows.remove(loose);
    windows.remove(first);
    windows.remove(second);
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "files"}));
    EXPECT_TRUE(model().at(1).windows.empty());
}

TEST_F(DockTest, LaunchesAndQuitsThroughTheRuntime)
{
    start("[Dock]\nPinned=editor\n");
    bool hidden = false;
    model().hideLauncher = [&] { hidden = true; };
    model().clicked("editor");
    EXPECT_TRUE(hidden);
    ASSERT_TRUE(test::runUntil([&] { return !runtime.launched.empty(); }));
    EXPECT_EQ(runtime.launched.front(), "editor");

    bool toggled = false;
    model().toggleLauncher = [&] { toggled = true; };
    model().clicked(DockModel::kLauncherId);
    EXPECT_TRUE(toggled);

    const uint64_t window = open("editor");
    model().quit("editor");
    EXPECT_EQ(windows.requests.back(), "quit " + std::to_string(window));
}

TEST_F(DockTest, ClickingTheActiveApplicationMinimizesAndNewInstancePolicyIsEnforced)
{
    start("[Dock]\nPinned=terminal, lingmo-settings, lingmo-filemanager\n");
    const uint64_t first = open("terminal", "Terminal");
    const uint64_t second = open("terminal", "Terminal 2");
    windows.setActive(first);

    model().clicked("terminal");
    EXPECT_EQ(windows.requests.back(), "minimize " + std::to_string(first));

    windows.setActive(0);
    windows.requests.clear();
    model().clicked("terminal");
    EXPECT_EQ(windows.requests.back(), "activate " + std::to_string(first));

    EXPECT_TRUE(model().canOpenNewInstance("terminal"));
    EXPECT_FALSE(model().canOpenNewInstance("lingmo-settings"));
    EXPECT_FALSE(model().canOpenNewInstance("lingmo-filemanager"));
    EXPECT_TRUE(model().openNewInstance("lingmo-settings"));
    EXPECT_TRUE(model().openNewInstance("lingmo-filemanager"));

    open("lingmo-settings", "Settings");
    open("lingmo-filemanager", "Files");
    EXPECT_FALSE(model().openNewInstance("lingmo-settings"));
    EXPECT_FALSE(model().openNewInstance("lingmo-filemanager"));
}

TEST_F(DockTest, PinsMovesAndDrops)
{
    start("[Dock]\nPinned=files\n");
    open("terminal");
    open("editor");
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "files", "terminal", "editor"}));

    model().pin("editor");
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "files", "editor", "terminal"}));
    EXPECT_EQ(savedPins(), (std::vector<std::string>{"files", "editor"}));

    // Within the pinned section; nothing goes before the launcher.
    model().move(2, 1);
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "editor", "files", "terminal"}));
    model().move(1, 0);
    EXPECT_EQ(ids()[0], DockModel::kLauncherId);
    // A pinned item does not cross the divider.
    model().move(1, 3, true);
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "editor", "files", "terminal"}));

    // An unpinned one dropped among the pins is pinned there.
    EXPECT_TRUE(model().dropApplication("terminal", 1, true));
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "terminal", "editor", "files"}));
    EXPECT_EQ(savedPins(), (std::vector<std::string>{"terminal", "editor", "files"}));

    // Unpinning a running application moves it among the running ones.
    model().unPin("terminal");
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "editor", "files", "terminal"}));
    EXPECT_FALSE(model().at(3).isPinned);

    // A desktop file inserted where a drag left a gap.
    EXPECT_TRUE(model().insert("/usr/share/applications/terminal.desktop", 1));
    EXPECT_EQ(ids(), (std::vector<std::string>{DockModel::kLauncherId, "terminal", "editor", "files"}));
}

TEST_F(DockTest, ServesOrgLingmoDock)
{
    start("[Dock]\nPinned=files\n");
    ASSERT_TRUE(dock->registerService());

    test::DBusTestClient settings;
    settings.call("org.lingmo.Dock", "/Dock", "org.lingmo.Dock", "setDirection", int32_t(0));
    settings.call("org.lingmo.Dock", "/Dock", "org.lingmo.Dock", "setGlassTint", 3.0);
    EXPECT_EQ(dock->settings().direction(), shell::DockSettings::Left);
    EXPECT_EQ(dock->settings().glassTint(), 1.0);

    const std::string xml = settings.introspect("org.lingmo.Dock", "/Dock");
    EXPECT_NE(xml.find("name=\"direction\""), std::string::npos);

    shell::waitForFileThread();
    base::Settings saved(directory.filePath("shell.conf"));
    EXPECT_EQ(saved.intValue("Dock/Direction", -1), 0);
    EXPECT_EQ(saved.doubleValue("Dock/GlassTint"), 1.0);
}

TEST_F(DockTest, KeepsMinimizedWindowsBesideTheTrash)
{
    start();
    shell::MinimizedWindows &minimized = dock->minimizedWindows();
    const uint64_t window = open("editor", "Notes");
    windows.change(window, [](shell::TaskWindow &w) {
        w.minimized = true;
        w.geometry = base::Rect(0, 0, 400, 200);
    }, shell::kStateChange);
    ASSERT_EQ(minimized.count(), 1);
    EXPECT_FALSE(minimized.entries()[0].landed);
    EXPECT_TRUE(minimized.entries()[0].flownIn);
    EXPECT_EQ(minimized.title(0), "Notes");
    EXPECT_EQ(minimized.iconName(0), "editor-icon");
    EXPECT_DOUBLE_EQ(minimized.aspect(0), 2.0);
    windows.minimizeLanded(window);
    EXPECT_TRUE(minimized.entries()[0].landed);

    windows.requests.clear();
    minimized.restore(window);
    EXPECT_EQ(windows.requests, (std::vector<std::string>{"activate " + std::to_string(window)}));
    windows.change(window, [](shell::TaskWindow &w) { w.minimized = false; }, shell::kStateChange);
    EXPECT_EQ(minimized.count(), 0);

    // One that never lands still shows its thumbnail after a moment.
    windows.change(window, [](shell::TaskWindow &w) { w.minimized = true; }, shell::kStateChange);
    EXPECT_TRUE(test::runUntil([&] { return minimized.entries()[0].landed; }));

    dock->settings().setMinimizedArea(false);
    EXPECT_EQ(minimized.count(), 0);
    dock->settings().setMinimizedArea(true);
    ASSERT_EQ(minimized.count(), 1);
    EXPECT_TRUE(minimized.entries()[0].landed);
    EXPECT_FALSE(minimized.entries()[0].flownIn);
}

TEST_F(DockTest, SeesSomethingInAnyTrash)
{
    // A volume mounted at <directory>/volume with the user's trash on it.
    const std::string volume = directory.filePath("volume");
    std::filesystem::create_directories(volume);
    directory.write("mountinfo", "36 35 98:0 / " + volume + " rw,noatime - ext4 /dev/sda1 rw\n"
                                 "37 35 0:5 / /proc rw - proc proc rw\n");
    start();
    shell::TrashMonitor &trash = dock->trash();
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_TRUE(trash.isEmpty());

    const std::string files = volume + "/.Trash-" + std::to_string(getuid()) + "/files";
    std::filesystem::create_directories(files);
    directory.write("volume/.Trash-" + std::to_string(getuid()) + "/files/note.txt", "x");
    EXPECT_TRUE(test::runUntil([&] { return !trash.isEmpty(); }));

    std::filesystem::remove(files + "/note.txt");
    EXPECT_TRUE(test::runUntil([&] { return trash.isEmpty(); }));

    std::filesystem::create_directories(trashHome + "/files");
    test::runFor(std::chrono::milliseconds(100));
    directory.write("Trash/files/old.txt", "x");
    EXPECT_TRUE(test::runUntil([&] { return !trash.isEmpty(); }));
}

TEST_F(DockTest, LeavesVolumesTheUserCannotRead)
{
    if (getuid() == 0)
        GTEST_SKIP() << "permissions do not apply to root";

    // A service's credentials, which only root may enter, and a volume the
    // user may enter but not list, with their trash already on it.
    const std::string uid = std::to_string(getuid());
    const std::string locked = directory.filePath("credentials");
    const std::string hidden = directory.filePath("hidden");
    std::filesystem::create_directories(locked);
    std::filesystem::create_directories(hidden + "/.Trash-" + uid + "/files");
    directory.write("hidden/.Trash-" + uid + "/files/note.txt", "x");
    std::filesystem::permissions(locked, std::filesystem::perms::none);
    std::filesystem::permissions(hidden, std::filesystem::perms::owner_exec);
    directory.write("mountinfo", "36 35 0:40 / " + locked + " ro,nosuid - ramfs none rw\n"
                                 "37 35 98:0 / " + hidden + " rw,noatime - ext4 /dev/sda1 rw\n");
    start();
    shell::TrashMonitor &trash = dock->trash();
    EXPECT_TRUE(test::runUntil([&] { return !trash.isEmpty(); }));
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_FALSE(trash.isRetrying());

    std::filesystem::remove(hidden + "/.Trash-" + uid + "/files/note.txt");
    EXPECT_TRUE(test::runUntil([&] { return trash.isEmpty(); }));
    EXPECT_FALSE(trash.isRetrying());

    std::filesystem::permissions(locked, std::filesystem::perms::owner_all);
    std::filesystem::permissions(hidden, std::filesystem::perms::owner_all);
}

TEST_F(DockTest, WatchesOnlyVolumesThatCanHoldATrash)
{
    if (getuid() == 0)
        GTEST_SKIP() << "permissions do not apply to root";

    // Every file made in a memory-backed folder or a system place would wake
    // the compositor, and none of them holds a trash.
    const std::string memory = directory.filePath("tmp");
    const std::string stick = directory.filePath("media/u/stick");
    const std::string data = directory.filePath("mnt/data");
    const std::string runtime = "/run/user/" + std::to_string(getuid());
    for (const std::string &path : {memory, stick, data})
        std::filesystem::create_directories(path);
    const std::string table = "21 1 8:2 / / rw,relatime - ext4 /dev/sda2 rw\n"
                              "22 21 0:21 / /proc rw,nosuid - proc proc rw\n"
                              "23 21 0:30 / " + memory + " rw,nosuid - tmpfs tmpfs rw\n"
                              "24 21 0:31 / " + runtime + " rw,nosuid - fuse.app app rw\n"
                              "25 21 8:17 / " + stick + " rw,nosuid - vfat /dev/sdb1 rw\n"
                              "26 21 8:33 / " + data + " rw,relatime - ext4 /dev/sdc1 rw\n";
    directory.write("mountinfo", table);

    const shell::TrashMonitor::State state =
            shell::TrashMonitor::look(directory.filePath("mountinfo"), directory.filePath("Trash"));
    EXPECT_FALSE(state.entries.contains("/"));
    EXPECT_FALSE(state.entries.contains("/proc"));
    EXPECT_FALSE(state.entries.contains(memory));
    EXPECT_FALSE(state.entries.contains(runtime));
    EXPECT_TRUE(state.entries.contains(stick));
    EXPECT_TRUE(state.entries.contains(data));
}

TEST_F(DockTest, EfficientModePreservesNormalDockSettings)
{
    start("[Dock]\nIconSize=64\nDirection=0\nVisibility=2\nMagnification=true\nBounceOnLaunch=true\nMinimizedArea=true\n");
    auto &settings = dock->settings();
    settings.setEfficientMode(true);
    EXPECT_TRUE(settings.efficientMode());
    EXPECT_EQ(settings.direction(), shell::DockSettings::Bottom);
    EXPECT_EQ(settings.visibility(), shell::DockSettings::AlwaysShow);
    EXPECT_EQ(settings.iconSize(), 32);
    EXPECT_EQ(settings.edgeMargins(), 0);
    EXPECT_FALSE(settings.magnification());
    EXPECT_FALSE(settings.bounceOnLaunch());
    EXPECT_FALSE(settings.minimizedArea());
    shell::waitForFileThread();
    EXPECT_EQ(base::Settings(directory.filePath("shell.conf")).intValue("Desktop/Mode"), 1);
    settings.setEfficientMode(false);
    EXPECT_EQ(settings.direction(), shell::DockSettings::Left);
    EXPECT_EQ(settings.visibility(), shell::DockSettings::IntellHide);
    EXPECT_EQ(settings.iconSize(), 64);
    EXPECT_TRUE(settings.magnification());
    EXPECT_TRUE(settings.bounceOnLaunch());
    EXPECT_TRUE(settings.minimizedArea());
}

TEST_F(DockTest, EfficientModeLoadsAndChangesThroughDBus)
{
    start("[Desktop]\nMode=1\n[Dock]\nIconSize=72\nDirection=2\n");
    EXPECT_TRUE(dock->settings().efficientMode());
    ASSERT_TRUE(dock->registerService());
    test::DBusTestClient client;
    const auto reply = client.call("org.lingmo.Dock", "/Dock", "org.lingmo.Dock", "setEfficientMode", false);
    EXPECT_FALSE(reply.isError());
    EXPECT_FALSE(dock->settings().efficientMode());
    EXPECT_EQ(dock->settings().iconSize(), 72);
    EXPECT_EQ(dock->settings().direction(), shell::DockSettings::Right);
}
