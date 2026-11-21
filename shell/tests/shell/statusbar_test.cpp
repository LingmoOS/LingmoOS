// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>

#include <gtest/gtest.h>
#include <unistd.h>

#include "base/locale.h"
#include "base/settings.h"
#include "dbus_test_client.h"
#include "loop_test_util.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/statusbar/app_menu_registrar.h"
#include "shell/statusbar/application_menu.h"
#include "shell/statusbar/battery.h"
#include "shell/statusbar/clock_format.h"
#include "shell/statusbar/dbus_menu.h"
#include "shell/statusbar/mpris.h"
#include "shell/statusbar/status_bar.h"
#include "shell/statusbar/status_notifier_watcher.h"
#include "shell/statusbar/system_tray_model.h"
#include "shell/statusbar/vpn_notifier.h"
#include "shell_test_util.h"
#include "statusbar_fakes.h"
#include "temporary_directory.h"
#include "lingmo/upower/battery_status.h"
#include "ui/bitmap.h"

using namespace Lingmo;
namespace dbus = base::dbus;
using test::FakeMenuExporter;
using test::FakeTrayItem;
using test::Pixmap;
using test::layout;
using test::menuWithFileAndEdit;

namespace {

dbus::Connection *sessionBus()
{
    return dbus::Connection::session(core::mainLoop());
}

} // namespace

TEST(DBusMenuTest, FetchesOneLevelAtATimeAndFollowsUpdates)
{
    test::DBusTestClient application;
    FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = menuWithFileAndEdit();
    exporter.layouts[1] = layout(
            1, {},
            {layout(10, {{"label", dbus::Variant(std::string("_Open"))},
                         {"shortcut", dbus::Variant(std::vector<std::vector<std::string>>{{"Control", "o"}})}}),
             layout(11, {{"type", dbus::Variant(std::string("separator"))}}),
             layout(12, {{"label", dbus::Variant(std::string("Wrap"))},
                         {"toggle-type", dbus::Variant(std::string("checkmark"))},
                         {"toggle-state", dbus::Variant(int32_t(1))},
                         {"enabled", dbus::Variant(false)}})});

    shell::DBusMenu menu(sessionBus(), application.connection->uniqueName(), "/MenuBar");
    ASSERT_TRUE(test::runUntil([&] { return menu.root().childrenLoaded; }));
    EXPECT_TRUE(exporter.called("GetLayout 0 1"));
    ASSERT_EQ(menu.root().children, (std::vector<int32_t>{1, 2}));
    EXPECT_EQ(menu.item(1)->label, "_File");
    EXPECT_TRUE(menu.item(1)->hasSubmenu);
    EXPECT_FALSE(menu.item(1)->childrenLoaded);

    // Opening a submenu without entries fetches them; "opened" goes along.
    menu.aboutToShow(1);
    ASSERT_TRUE(test::runUntil([&] { return menu.item(1)->childrenLoaded; }));
    EXPECT_TRUE(exporter.called("AboutToShow 1"));
    EXPECT_TRUE(exporter.called("Event 1 opened"));
    EXPECT_EQ(menu.item(1)->children, (std::vector<int32_t>{10, 11, 12}));
    EXPECT_EQ(menu.item(10)->shortcut, (std::vector<std::vector<std::string>>{{"Control", "o"}}));
    EXPECT_TRUE(menu.item(11)->separator);
    EXPECT_EQ(menu.item(12)->toggle, shell::DBusMenuItem::Toggle::Checkmark);
    EXPECT_EQ(menu.item(12)->toggleState, 1);
    EXPECT_FALSE(menu.item(12)->enabled);

    // Properties change in place.
    exporter.object.emitSignal(
            "ItemsPropertiesUpdated",
            std::vector<std::tuple<int32_t, dbus::VariantMap>>{
                    {12, {{"enabled", dbus::Variant(true)}, {"label", dbus::Variant(std::string("Wrap Lines"))}}}},
            std::vector<std::tuple<int32_t, std::vector<std::string>>>{{10, {"shortcut"}}});
    ASSERT_TRUE(test::runUntil([&] { return menu.item(12)->enabled; }));
    EXPECT_EQ(menu.item(12)->label, "Wrap Lines");
    EXPECT_TRUE(menu.item(10)->shortcut.empty());

    // A new layout drops what it no longer lists, with everything under it.
    exporter.layouts[0] = layout(0, {}, {layout(2, {{"label", dbus::Variant(std::string("_Edit"))}})});
    exporter.object.emitSignal("LayoutUpdated", uint32_t(2), int32_t(0));
    ASSERT_TRUE(test::runUntil([&] { return menu.item(1) == nullptr; }));
    EXPECT_EQ(menu.item(10), nullptr);
    EXPECT_EQ(menu.root().children, (std::vector<int32_t>{2}));
    // The kind an entry came with stays.
    EXPECT_TRUE(menu.item(2)->hasSubmenu);

    menu.activate(2);
    menu.closed(2);
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("Event 2 closed"); }));
    EXPECT_TRUE(exporter.called("Event 2 clicked"));

    int32_t requested = -1;
    auto activation = menu.activationRequested.connect([&](int32_t id) { requested = id; });
    exporter.object.emitSignal("ItemActivationRequested", int32_t(2), uint32_t(0));
    EXPECT_TRUE(test::runUntil([&] { return requested == 2; }));
}

TEST(DBusMenuTest, DecodesIconsAndRefusesHugeOnes)
{
    test::DBusTestClient application;
    FakeMenuExporter exporter(application.connection.get(), "/Menu");
    ui::Bitmap red(4, 4, ui::Bitmap::Format::Argb32Premultiplied);
    red.fill(0xffff0000);
    const std::string png = red.encodePng().value();
    std::string huge = png;
    // Width 100000 in the header.
    huge[16] = 0;
    huge[17] = 1;
    huge[18] = char(0x86);
    huge[19] = char(0xa0);
    exporter.layouts[0] =
            layout(0, {},
                   {layout(1, {{"icon-data", dbus::Variant(std::vector<uint8_t>(png.begin(), png.end()))}}),
                    layout(2, {{"icon-data", dbus::Variant(std::vector<uint8_t>(huge.begin(), huge.end()))}})});

    shell::DBusMenu menu(sessionBus(), application.connection->uniqueName(), "/Menu");
    ASSERT_TRUE(test::runUntil([&] { return menu.item(1) && menu.item(1)->icon; }));
    EXPECT_EQ(menu.item(1)->icon->width(), 4);
    EXPECT_EQ(menu.item(1)->icon->pixel(1, 1), 0xffff0000u);
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_EQ(menu.item(2)->icon, nullptr);
}

TEST(SystemTrayTest, RegistersItemsAndReadsThem)
{
    shell::StatusNotifierWatcher watcher(sessionBus());
    ASSERT_TRUE(watcher.registerService());
    // The icon theme is the test's: only "known-icon-symbolic" exists.
    shell::SystemTrayModel tray(watcher, sessionBus(), nullptr, [](const std::string &name, const std::string &) {
        return name == "known-icon-symbolic";
    });

    auto application = std::make_unique<test::DBusTestClient>();
    FakeTrayItem item(application->connection.get());
    item.iconName = "known-icon";
    // One 2x2 picture, ARGB in network order: opaque red, half-transparent
    // white, then opaque red again.
    item.pixmaps = {Pixmap(2, 2, {0xff, 0xff, 0, 0, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0xff, 0xff, 0, 0})};

    test::DBusTestClient observer;
    observer.listen("org.kde.StatusNotifierWatcher");
    EXPECT_FALSE(application
                         ->call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
                                "org.kde.StatusNotifierWatcher", "RegisterStatusNotifierItem",
                                std::string("/StatusNotifierItem"))
                         .isError());
    ASSERT_TRUE(test::runUntil([&] { return tray.count() == 1 && !tray.at(0)->pixmaps().empty(); }));
    shell::StatusNotifierItem *shown = tray.at(0);
    EXPECT_EQ(shown->id(), application->connection->uniqueName() + "/StatusNotifierItem");
    EXPECT_EQ(shown->title(), "Example");
    EXPECT_EQ(shown->toolTip(), "Tip");
    // Only the -symbolic variant exists.
    EXPECT_EQ(shown->iconName(), "known-icon-symbolic");
    EXPECT_EQ(shown->iconThemePath(), "/nonexistent/icons");
    EXPECT_EQ(shown->menu(), nullptr);
    ASSERT_EQ(shown->pixmaps().size(), 1u);
    EXPECT_EQ(shown->pixmaps().front().pixel(0, 0), 0xffff0000u);
    EXPECT_EQ(shown->pixmaps().front().pixel(1, 0), 0x80808080u);

    const auto items = observer.getProperty("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
                                            "org.kde.StatusNotifierWatcher", "RegisteredStatusNotifierItems");
    EXPECT_EQ(items.value<dbus::Variant>().value_or(dbus::Variant()).value<std::vector<std::string>>(),
              std::vector<std::string>{shown->id()});
    const auto host = observer.getProperty("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
                                           "org.kde.StatusNotifierWatcher", "IsStatusNotifierHostRegistered");
    EXPECT_TRUE(host.value<dbus::Variant>().value_or(dbus::Variant()).value<bool>());
    EXPECT_TRUE(test::runUntil([&] { return !observer.signalsSeen.empty(); }));
    EXPECT_EQ(observer.signalsSeen.front().member(), "StatusNotifierItemRegistered");

    // Clicks reach the item; without a menu it shows its own.
    tray.leftButtonClick(shown->id(), 10, 20);
    tray.middleButtonClick(shown->id(), 0, 0);
    EXPECT_EQ(tray.contextMenu(shown->id(), 0, 0), nullptr);
    shown->scroll(120, "vertical");
    ASSERT_TRUE(test::runUntil([&] { return item.calls.size() == 4; }));
    EXPECT_EQ(item.calls,
              (std::vector<std::string>{"Activate 10,20", "SecondaryActivate", "ContextMenu", "Scroll 120 vertical"}));

    // A new title is read after the item says so.
    item.title = "Renamed";
    item.object.emitSignal("NewTitle");
    EXPECT_TRUE(test::runUntil([&] { return shown->title() == "Renamed"; }));

    // Its application leaving takes it away.
    int removed = 0;
    auto subscription = tray.rowsRemoved.connect([&](int, int count) { removed += count; });
    application.reset();
    EXPECT_TRUE(test::runUntil([&] { return removed == 1; }));
    EXPECT_EQ(tray.count(), 0);
    EXPECT_TRUE(watcher.items().empty());
}

TEST(SystemTrayTest, RefusesItemsOfServicesNotOnTheBus)
{
    shell::StatusNotifierWatcher watcher(sessionBus());
    ASSERT_TRUE(watcher.registerService());
    test::DBusTestClient application;
    application.call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
                     "RegisterStatusNotifierItem", std::string("org.example.Gone"));
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_TRUE(watcher.items().empty());
}

TEST(SystemTrayTest, OffersTheItemsMenu)
{
    shell::StatusNotifierWatcher watcher(sessionBus());
    ASSERT_TRUE(watcher.registerService());
    shell::SystemTrayModel tray(watcher, sessionBus(), nullptr,
                                [](const std::string &, const std::string &) { return false; });
    test::DBusTestClient application;
    FakeTrayItem item(application.connection.get());
    item.menuPath = "/MenuBar";
    FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = menuWithFileAndEdit();
    application.call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
                     "RegisterStatusNotifierItem", std::string("/StatusNotifierItem"));
    ASSERT_TRUE(test::runUntil([&] { return tray.count() == 1 && tray.at(0)->menu(); }));
    shell::DBusMenu *menu = tray.contextMenu(tray.at(0)->id(), 0, 0);
    ASSERT_NE(menu, nullptr);
    EXPECT_TRUE(test::runUntil([&] { return exporter.called("AboutToShow 0"); }));
    EXPECT_TRUE(test::runUntil([&] { return menu->root().children.size() == 2; }));
}

TEST(AppMenuRegistrarTest, FindsAProcessMenuAndForgetsItsServiceLeaving)
{
    shell::AppMenuRegistrar registrar(sessionBus());
    ASSERT_TRUE(registrar.registerService());
    auto application = std::make_unique<test::DBusTestClient>();
    std::vector<uint32_t> registered;
    auto subscription = registrar.windowRegistered.connect(
            [&](uint32_t id, const std::string &, const std::string &) { registered.push_back(id); });

    const char *service = "com.canonical.AppMenu.Registrar";
    const char *path = "/com/canonical/AppMenu/Registrar";
    application->call(service, path, service, "RegisterWindow", uint32_t(7), dbus::ObjectPath{"/menu/7"});
    application->call(service, path, service, "RegisterWindow", uint32_t(8), dbus::ObjectPath{"/menu/8"});
    ASSERT_TRUE(test::runUntil([&] { return registered.size() == 2; }));
    const std::string exporter = application->connection->uniqueName();
    EXPECT_EQ(registrar.serviceForPid(uint32_t(getpid())), exporter);
    EXPECT_EQ(registrar.pathForPid(uint32_t(getpid())), "/menu/8");

    std::string owner;
    dbus::ObjectPath menuPath;
    ASSERT_TRUE(application->call(service, path, service, "GetMenuForWindow", uint32_t(7)).read(owner, menuPath));
    EXPECT_EQ(owner, exporter);
    EXPECT_EQ(menuPath.path, "/menu/7");

    application->call(service, path, service, "UnregisterWindow", uint32_t(8));
    ASSERT_TRUE(test::runUntil([&] { return registrar.pathForPid(uint32_t(getpid())) == "/menu/7"; }));

    std::vector<uint32_t> unregistered;
    auto gone = registrar.windowUnregistered.connect([&](uint32_t id) { unregistered.push_back(id); });
    application.reset();
    EXPECT_TRUE(test::runUntil([&] { return unregistered == std::vector<uint32_t>{7}; }));
    EXPECT_EQ(registrar.serviceForPid(uint32_t(getpid())), "");
}

TEST(AppMenuRegistrarTest, TheDesktopsMenuIsNotItsProcesssOtherWindows)
{
    // The file manager registers its desktop's File/Edit as window 0 and
    // owns browser windows as well: those never show the desktop's menus.
    shell::AppMenuRegistrar registrar(sessionBus());
    ASSERT_TRUE(registrar.registerService());
    auto application = std::make_unique<test::DBusTestClient>();
    std::vector<uint32_t> registered;
    auto subscription = registrar.windowRegistered.connect(
            [&](uint32_t id, const std::string &, const std::string &) { registered.push_back(id); });
    const char *service = "com.canonical.AppMenu.Registrar";
    const char *path = "/com/canonical/AppMenu/Registrar";
    const uint32_t pid = uint32_t(getpid());
    application->call(service, path, service, "RegisterWindow", uint32_t(0),
                      dbus::ObjectPath{"/org/lingmo/DesktopMenu"});
    ASSERT_TRUE(test::runUntil([&] { return registered.size() == 1; }));
    const std::string exporter = application->connection->uniqueName();
    EXPECT_EQ(registrar.serviceForPid(pid), "");
    EXPECT_EQ(registrar.pathForPid(pid), "");
    EXPECT_EQ(registrar.desktopServiceForPid(pid), exporter);
    EXPECT_EQ(registrar.desktopPathForPid(pid), "/org/lingmo/DesktopMenu");

    // A window of its own that does export a menu shows that one, and the
    // desktop keeps its menu, newer registration or not.
    application->call(service, path, service, "RegisterWindow", uint32_t(3), dbus::ObjectPath{"/menu/3"});
    ASSERT_TRUE(test::runUntil([&] { return registered.size() == 2; }));
    application->call(service, path, service, "RegisterWindow", uint32_t(0),
                      dbus::ObjectPath{"/org/lingmo/DesktopMenu"});
    ASSERT_TRUE(test::runUntil([&] { return registered.size() == 3; }));
    EXPECT_EQ(registrar.pathForPid(pid), "/menu/3");
    EXPECT_EQ(registrar.desktopPathForPid(pid), "/org/lingmo/DesktopMenu");
}

namespace {

class FakePlayer
{
public:
    FakePlayer(dbus::Connection *bus, const std::string &name)
        : root(bus, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2")
        , player(bus, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player")
    {
        root.addProperty("Identity", [name] { return name; });
        root.addProperty("CanRaise", [] { return true; });
        player.addProperty("PlaybackStatus", [this] { return status; });
        player.addProperty("CanControl", [] { return true; });
        player.addProperty("CanGoNext", [] { return true; });
        player.addProperty("Metadata", [this] { return metadata; });
        for (const char *method : {"Play", "Pause", "PlayPause", "Next", "Previous", "Stop"})
            player.addMethod(method, [this, method] { calls.push_back(method); });
        EXPECT_TRUE(root.publish());
        EXPECT_TRUE(player.publish());
        EXPECT_TRUE(bus->requestName("org.mpris.MediaPlayer2." + name));
    }

    void setStatus(const std::string &value)
    {
        status = value;
        player.emitPropertiesChanged({"PlaybackStatus"});
    }

    dbus::Object root;
    dbus::Object player;
    std::string status = "Stopped";
    dbus::VariantMap metadata;
    std::vector<std::string> calls;
};

} // namespace

TEST(MprisTest, ShowsThePlayerThatLastStartedPlaying)
{
    test::DBusTestClient first;
    FakePlayer music(first.connection.get(), "music");
    music.metadata = {{"xesam:title", dbus::Variant(std::string("Song"))},
                      {"xesam:artist", dbus::Variant(std::vector<std::string>{"A", "B"})},
                      {"mpris:artUrl", dbus::Variant(std::string("file:///art.png"))}};

    shell::MprisManager manager(sessionBus());
    ASSERT_TRUE(test::runUntil([&] { return manager.current() && manager.current()->isReady(); }));
    EXPECT_EQ(manager.currentService(), "org.mpris.MediaPlayer2.music");
    EXPECT_EQ(manager.current()->identity(), "music");
    EXPECT_EQ(manager.current()->title(), "Song");
    EXPECT_EQ(manager.current()->artist(), "A, B");
    EXPECT_EQ(manager.current()->artUrl(), "file:///art.png");
    EXPECT_TRUE(manager.current()->canGoNext());

    music.setStatus("Playing");
    ASSERT_TRUE(test::runUntil(
            [&] { return manager.current()->playbackStatus() == shell::MprisPlayer::PlaybackStatus::Playing; }));

    // Another player appears while this one plays: it waits behind it.
    auto second = std::make_unique<test::DBusTestClient>();
    auto video = std::make_unique<FakePlayer>(second->connection.get(), "video");
    ASSERT_TRUE(test::runUntil([&] { return manager.availableServices().size() == 2; }));
    EXPECT_EQ(manager.currentService(), "org.mpris.MediaPlayer2.music");

    // It starts playing, then the shown one stops: it takes over.
    test::runFor(std::chrono::milliseconds(30));
    video->setStatus("Playing");
    test::runFor(std::chrono::milliseconds(30));
    EXPECT_EQ(manager.currentService(), "org.mpris.MediaPlayer2.music");
    music.setStatus("Paused");
    ASSERT_TRUE(test::runUntil([&] { return manager.currentService() == "org.mpris.MediaPlayer2.video"; }));

    manager.current()->playPause();
    manager.current()->next();
    ASSERT_TRUE(test::runUntil([&] { return video->calls.size() == 2; }));
    EXPECT_EQ(video->calls, (std::vector<std::string>{"PlayPause", "Next"}));

    // It leaves: the next one on the list is shown.
    video.reset();
    second.reset();
    EXPECT_TRUE(test::runUntil([&] { return manager.currentService() == "org.mpris.MediaPlayer2.music"; }));
}

TEST(ApplicationMenuTest, SwitchesWithTheActiveApplicationOnceItsMenuIsIn)
{
    test::DBusTestClient application;
    FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = menuWithFileAndEdit();
    exporter.layouts[1] = layout(1, {}, {});
    exporter.layouts[2] = layout(2, {}, {});

    test::FakeApplications applications;
    applications.addSimple("org.example.Editor", "Editor");
    applications.scan();
    test::FakeTaskWindows windows;
    shell::TaskManager tasks(windows, applications, [](int64_t) { return shell::ProcessCommand(); });
    shell::ApplicationMenu menu(sessionBus(), windows, tasks, nullptr);
    int switches = 0;
    auto switched = menu.switched.connect([&] { ++switches; });

    shell::TaskWindow editor;
    editor.appId = "org.example.Editor";
    editor.title = "Draft";
    editor.menuService = application.connection->uniqueName();
    editor.menuPath = "/MenuBar";
    const uint64_t id = windows.add(editor);
    windows.setActive(id);

    ASSERT_TRUE(test::runUntil([&] { return menu.menu() != nullptr; }));
    EXPECT_EQ(menu.title(), "Editor");
    EXPECT_EQ(menu.menu()->root().children.size(), 2u);
    // The submenus the bar pops up were asked to get ready.
    EXPECT_TRUE(test::runUntil([&] { return exporter.called("AboutToShow 1") && exporter.called("AboutToShow 2"); }));

    // A window without a menu: its name alone.
    shell::TaskWindow plain;
    plain.appId = "plain";
    plain.title = "Plain";
    const uint64_t other = windows.add(plain);
    windows.setActive(other);
    ASSERT_TRUE(test::runUntil([&] { return menu.title() == "Plain"; }));
    EXPECT_EQ(menu.menu(), nullptr);

    menu.setDesktop(true);
    ASSERT_TRUE(test::runUntil([&] { return menu.title() == "Desktop"; }));
    menu.hideOtherApplications();
    EXPECT_EQ(windows.requests.back(), "hideOthers 0");

    windows.setActive(0);
    menu.setDesktop(false);
    ASSERT_TRUE(test::runUntil([&] { return menu.title().empty(); }));
    EXPECT_GE(switches, 3);
}

TEST(ClockFormatTest, FollowsTheLocalesOrderWithoutCommas)
{
    const base::DateTime time = base::DateTime::utc({2026, 9, 26}, {13, 40, 0, 0});
    // CLDR puts a narrow no-break space before the day period, kept as is.
    EXPECT_EQ(base::Locale("en_US").toString(time, shell::clock_format::pattern("en_US", false)),
              "Sat Sep 26 1:40\u202fPM");
    EXPECT_EQ(base::Locale("en_US").toString(time, shell::clock_format::pattern("en_US", true)),
              "Sat Sep 26 13:40");
    EXPECT_EQ(base::Locale("zh_CN").toString(time, shell::clock_format::pattern("zh_CN", false)),
              "9\xe6\x9c\x88"
              "26\xe6\x97\xa5\xe5\x91\xa8\xe5\x85\xad \xe4\xb8\x8b\xe5\x8d\x88"
              "1:40");
    EXPECT_EQ(shell::clock_format::toLocaleFormat("d 'de' MMMM"), "d' de 'MMMM");
    EXPECT_EQ(shell::clock_format::toLocaleFormat("G y"), "");

    EXPECT_FALSE(shell::clock_format::showsSeconds(shell::clock_format::pattern("en_US", false)));
    EXPECT_FALSE(shell::clock_format::showsSeconds("H:mm' Uhr 'ddd"));
    EXPECT_FALSE(shell::clock_format::showsSeconds("H:mm' secs'"));
    EXPECT_TRUE(shell::clock_format::showsSeconds("H:mm:ss"));
    EXPECT_TRUE(shell::clock_format::showsSeconds("'a''b' s"));
}

TEST(BatteryTest, PicksTheIconForTheLevel)
{
    EXPECT_EQ(shell::Battery::iconSource(100, true), "battery-level-100-symbolic.svg");
    EXPECT_EQ(shell::Battery::iconSource(95, true), "battery-level-100-symbolic.svg");
    EXPECT_EQ(shell::Battery::iconSource(94, false), "battery-level-90-charging-symbolic.svg");
    EXPECT_EQ(shell::Battery::iconSource(15, true), "battery-level-20-symbolic.svg");
    EXPECT_EQ(shell::Battery::iconSource(14, true), "battery-level-10-symbolic.svg");
    EXPECT_EQ(shell::Battery::iconSource(4, true), "battery-level-0-symbolic.svg");
}

// lingmo-services comes up after the shell: the battery shows once it does.
TEST(BatteryTest, ShowsOnceTheServiceComesUp)
{
    test::TemporaryDirectory directory;
    const auto make = [] { return std::make_shared<upower::BatteryStatus>(sessionBus(), nullptr); };
    shell::Battery battery(make(), directory.filePath("shell.conf"));
    battery.followService(sessionBus(), make);
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_FALSE(battery.isAvailable());

    test::DBusTestClient services;
    dbus::Object object(services.connection.get(), "/org/lingmo/Services/Battery",
                        "org.lingmo.Services.Battery");
    object.addProperty("chargePercent", [] { return int32_t(70); });
    object.addProperty("chargeState", [] { return int32_t(1); });
    ASSERT_TRUE(object.publish());
    int changes = 0;
    auto subscription = battery.changed.connect([&] { ++changes; });
    ASSERT_TRUE(services.connection->requestName("org.lingmo.Services"));
    EXPECT_TRUE(test::runUntil([&] { return battery.isAvailable(); }));
    EXPECT_EQ(70, battery.chargePercent());
    EXPECT_GT(changes, 0);
}

TEST(StatusBarTest, ServesOrgLingmoStatusbar)
{
    test::TemporaryDirectory directory;
    directory.write("shell.conf", "[Statusbar]\nTwentyFourTime=true\n");
    shell::StatusBar statusBar(sessionBus(), dbus::Connection::system(core::mainLoop()), nullptr,
                               directory.filePath("shell.conf"));
    ASSERT_TRUE(statusBar.registerServices());
    ASSERT_TRUE(test::runUntil([&] { return statusBar.twentyFourTime(); }));
    EXPECT_NE(statusBar.timeFormat().find('H'), std::string::npos);
    ASSERT_TRUE(test::runUntil([&] { return statusBar.statusSettings().isReady(); }));
    EXPECT_DOUBLE_EQ(shell::StatusBarSettings::kDefaultBlurSigma, statusBar.statusSettings().blurSigma());
    EXPECT_DOUBLE_EQ(shell::StatusBarSettings::kDefaultOpacity, statusBar.statusSettings().opacity());

    test::DBusTestClient settings;
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setTwentyFourTime", false);
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setBatteryPercentage", true);
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setBlurSigma", 32.0);
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setOpacity", 0.25);
    EXPECT_FALSE(statusBar.twentyFourTime());
    EXPECT_TRUE(statusBar.battery().showPercentage());
    EXPECT_DOUBLE_EQ(32.0, statusBar.statusSettings().blurSigma());
    EXPECT_DOUBLE_EQ(0.25, statusBar.statusSettings().opacity());
    EXPECT_EQ(statusBar.timeFormat().find('H'), std::string::npos);

    bool requested = false;
    auto subscription = statusBar.powerActions().requested.connect([&] { requested = true; });
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "requestReboot");
    EXPECT_TRUE(requested);
    EXPECT_EQ(statusBar.powerActions().pending(), shell::PowerActions::Reboot);
    statusBar.powerActions().cancel();
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_EQ(statusBar.powerActions().pending(), shell::PowerActions::None);

    shell::waitForFileThread();
    base::Settings saved(directory.filePath("shell.conf"));
    EXPECT_FALSE(saved.boolValue("Statusbar/TwentyFourTime", true));
    EXPECT_TRUE(saved.boolValue("Statusbar/BatteryPercentage", false));
    EXPECT_DOUBLE_EQ(32.0, saved.doubleValue("Statusbar/BlurSigma", -1.0));
    EXPECT_DOUBLE_EQ(0.25, saved.doubleValue("Statusbar/Opacity", -1.0));

    const std::string xml = settings.introspect("org.lingmo.Statusbar", "/Statusbar");
    EXPECT_NE(xml.find("name=\"batteryPercentage\""), std::string::npos);
    EXPECT_NE(xml.find("name=\"setBlurSigma\""), std::string::npos);
    EXPECT_NE(xml.find("name=\"setOpacity\""), std::string::npos);
}

TEST(VpnNotifierTest, TellsAFailedVpnAndOpensItsPage)
{
    test::DBusTestClient server;
    dbus::Object notifications(server.connection.get(), "/org/freedesktop/Notifications",
                               "org.freedesktop.Notifications");
    std::vector<std::string> shown;
    std::string entry;
    notifications.addMethod("Notify", "susssasa{sv}i", "u", [&](dbus::MethodCall &call) {
        std::string appName;
        uint32_t replaces = 0;
        std::string icon;
        std::string summary;
        std::string body;
        std::vector<std::string> actions;
        dbus::VariantMap hints;
        call.read(appName, replaces, icon, summary, body, actions, hints);
        shown = {icon, summary, body, actions.empty() ? std::string() : actions.front()};
        entry = hints["desktop-entry"].value<std::string>();
        call.replyWith({dbus::Variant(uint32_t(7))});
    });
    notifications.addSignal("ActionInvoked", "us");
    ASSERT_TRUE(notifications.publish());
    ASSERT_TRUE(server.connection->requestName("org.freedesktop.Notifications"));

    shell::VpnNotifier notifier(sessionBus());
    int opened = 0;
    notifier.openSettings = [&] { ++opened; };
    notifier.vpnFailed("Office", "The VPN service stopped.");
    ASSERT_TRUE(test::runUntil([&] { return !shown.empty(); }));
    EXPECT_EQ((std::vector<std::string>{"network-vpn", "VPN connection failed",
                                        "\u201COffice\u201D could not connect. The VPN service stopped.", "default"}),
              shown);
    EXPECT_EQ("lingmo-settings", entry);
    test::runFor(std::chrono::milliseconds(20));
    notifications.emitSignal("ActionInvoked", uint32_t(3), std::string("default"));
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_EQ(0, opened);
    notifications.emitSignal("ActionInvoked", uint32_t(7), std::string("default"));
    EXPECT_TRUE(test::runUntil([&] { return opened == 1; }));
}

TEST(AppMenuRegistrarTest, EfficientModeReleasesAndRestoresTheRegistrarName)
{
    shell::AppMenuRegistrar registrar(sessionBus());
    ASSERT_TRUE(registrar.registerService());
    test::DBusTestClient client;
    const auto owned = [&] {
        return client.call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                           "NameHasOwner", std::string("com.canonical.AppMenu.Registrar")).value<bool>().value_or(false);
    };
    EXPECT_TRUE(owned());
    registrar.setEnabled(false);
    EXPECT_FALSE(owned());
    EXPECT_TRUE(registrar.registerService());
    EXPECT_FALSE(owned());
    registrar.setEnabled(true);
    EXPECT_TRUE(owned());
}
