// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <filesystem>
#include <fstream>
#include <future>
#include <tuple>

#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/file.h"
#include "base/paths.h"
#include "base/settings.h"
#include "dbus_test_client.h"
#include "loop_test_util.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/notification/notifications.h"
#include "shell_test_util.h"
#include "temporary_directory.h"
#include "ui/bitmap.h"

using namespace Lingmo;
namespace dbus = base::dbus;
using shell::Notification;
using shell::NotificationServer;
using shell::NotificationSettings;

namespace {

constexpr const char *kService = "org.freedesktop.Notifications";
constexpr const char *kPath = "/org/freedesktop/Notifications";

using ImageData = std::tuple<int32_t, int32_t, int32_t, bool, int32_t, int32_t, std::vector<uint8_t>>;

class NotificationServiceTest : public testing::Test
{
protected:
    NotificationServiceTest()
    {
        applications.addSimple("org.example.Chat", "Chat", "chat-icon");
        applications.addSimple("org.example.Mail", "Mail", "mail-icon");
        applications.scan();
        // The client below is this process.
        commands[getpid()] = {"/usr/bin/org.example.Chat", "org.example.Chat"};
    }

    void start(const std::string &settings = {})
    {
        if (!settings.empty())
            directory.write("shell.conf", settings);
        tasks = std::make_unique<shell::TaskManager>(windows, applications,
                                                     [this](int64_t pid) { return commands[pid]; });
        notifications = std::make_unique<shell::Notifications>(
                dbus::Connection::session(core::mainLoop()), applications, tasks.get(),
                shell::Notifications::Files{directory.filePath("shell.conf"), directory.filePath("history"),
                                            directory.filePath("images")});
        ASSERT_TRUE(notifications->registerServices());
        ASSERT_TRUE(test::runUntil([this] {
            return notifications->settings().isReady() && notifications->history().isLoaded();
        }));
    }

    uint32_t notify(const std::string &summary, uint32_t replaces = 0, dbus::VariantMap hints = {},
                    std::vector<std::string> actions = {}, const std::string &appName = "chat")
    {
        const dbus::Reply reply = client.call(kService, kPath, kService, "Notify", appName, replaces,
                                              std::string(), summary, std::string("Body"), actions, hints,
                                              int32_t(-1));
        EXPECT_FALSE(reply.isError()) << reply.error().message;
        return reply.value<uint32_t>().value_or(0);
    }

    // Lets the lookups behind the notifications finish.
    void settle()
    {
        test::runFor(std::chrono::milliseconds(50));
        shell::waitForFileThread();
        test::runFor(std::chrono::milliseconds(20));
    }

    test::TemporaryDirectory directory;
    test::FakeApplications applications;
    test::FakeTaskWindows windows;
    std::map<int64_t, shell::ProcessCommand> commands;
    std::unique_ptr<shell::TaskManager> tasks;
    std::unique_ptr<shell::Notifications> notifications;
    test::DBusTestClient client;
};

} // namespace

TEST_F(NotificationServiceTest, AnswersTheSpecsQuestions)
{
    start();
    const dbus::Reply capabilities = client.call(kService, kPath, kService, "GetCapabilities");
    EXPECT_EQ(capabilities.value<std::vector<std::string>>(),
              (std::vector<std::string>{"body", "body-hyperlinks", "body-markup", "icon-static", "actions",
                                        "persistence"}));

    std::string name, vendor, version, spec;
    ASSERT_TRUE(client.call(kService, kPath, kService, "GetServerInformation").read(name, vendor, version, spec));
    EXPECT_EQ(name, "Lingmo");
    EXPECT_EQ(vendor, "LingmoOS");
    EXPECT_EQ(version, "1.0");
    EXPECT_EQ(spec, "1.2");

    const std::string xml = client.introspect(kService, kPath);
    EXPECT_NE(xml.find("name=\"app_name\""), std::string::npos);
    EXPECT_NE(xml.find("name=\"expire_timeout\""), std::string::npos);
}

TEST_F(NotificationServiceTest, ShowsANotificationAsTheSendersApplication)
{
    start();
    const uint32_t id = notify("Hello", 0, {{"urgency", dbus::Variant(uint8_t(2))}, {"value", dbus::Variant(int32_t(40))}},
                               {"default", "Open", "reply", "Reply"});
    EXPECT_EQ(id, 1u);
    settle();

    const auto &banners = notifications->banners().notifications();
    ASSERT_EQ(banners.size(), 1u);
    const Notification &shown = banners.front();
    EXPECT_EQ(shown.id, id);
    EXPECT_EQ(shown.summary, "Hello");
    EXPECT_EQ(shown.desktopEntry, "org.example.Chat");
    EXPECT_EQ(shown.appName, "chat");
    EXPECT_EQ(shown.appIcon, "chat-icon");
    EXPECT_EQ(shown.urgency, 2);
    EXPECT_EQ(shown.value, 40);
    ASSERT_EQ(shown.actionList().size(), 1u);
    EXPECT_EQ(shown.actionList().front().key, "reply");
    EXPECT_EQ(shown.service, client.connection->uniqueName());

    // The sender is noted for Settings.
    const dbus::Reply applications =
            client.call("org.lingmo.Notification", "/Notification", "org.lingmo.Notification", "applications");
    const auto list = applications.value<std::vector<dbus::VariantMap>>();
    ASSERT_TRUE(list.has_value());
    ASSERT_EQ(list->size(), 1u);
    EXPECT_EQ(list->front().at("key").value<std::string>(), "org.example.Chat");
    EXPECT_EQ(list->front().at("name").value<std::string>(), "Chat");
    EXPECT_EQ(list->front().at("mode").value<int32_t>(), 0);
}

TEST_F(NotificationServiceTest, ReplacesInPlaceAndFoldsRepeats)
{
    start();
    const uint32_t first = notify("Progress");
    settle();
    const std::optional<int64_t> created = notifications->banners().notifications().front().created;

    EXPECT_EQ(notify("Progress 50%", first), first);
    settle();
    ASSERT_EQ(notifications->banners().count(), 1);
    EXPECT_EQ(notifications->banners().notifications().front().summary, "Progress 50%");
    EXPECT_EQ(notifications->banners().notifications().front().created, created);
    EXPECT_TRUE(notifications->banners().notifications().front().updated.has_value());

    // The same text again within a second is the same notification.
    EXPECT_EQ(notify("Progress 50%"), first);
    // A closed id is not reused: replacing it gives a new one.
    notifications->banners().close(first);
    settle();
    EXPECT_EQ(notify("Again", first), first + 1);
}

TEST_F(NotificationServiceTest, ReplacesAndClosesWhatIsNotShownYet)
{
    start();
    // The first notification's picture waits on the file thread, so the
    // ones behind it are not shown before their replacements arrive.
    std::promise<void> gate;
    std::shared_future<void> opened = gate.get_future().share();
    shell::fileThread().post([opened] { opened.wait(); });

    std::vector<uint8_t> pixels(size_t(16) * 16 * 4, 0x80);
    notify("Photo", 0, {{"image-data", dbus::Variant(ImageData(16, 16, 16 * 4, true, 8, 4, pixels))}});
    const uint32_t progress = notify("Progress");
    EXPECT_EQ(notify("Progress 50%", progress), progress);
    const uint32_t revoked = notify("Revoked");
    EXPECT_EQ(notify("Revoked again", revoked), revoked);
    EXPECT_FALSE(client.call(kService, kPath, kService, "CloseNotification", revoked).isError());
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_EQ(notifications->banners().count(), 0);

    gate.set_value();
    settle();
    const auto &banners = notifications->banners().notifications();
    ASSERT_EQ(banners.size(), 2u);
    EXPECT_EQ(banners.back().id, progress);
    EXPECT_EQ(banners.back().summary, "Progress 50%");

    // Nothing waits behind them.
    notify("Later");
    settle();
    EXPECT_EQ(notifications->banners().notifications().back().summary, "Later");
}

TEST_F(NotificationServiceTest, ClosesWithTheSpecsSignalOrError)
{
    start();
    client.listen(kService);
    const uint32_t id = notify("Close me");
    settle();

    EXPECT_FALSE(client.call(kService, kPath, kService, "CloseNotification", id).isError());
    ASSERT_TRUE(test::runUntil([&] { return !client.signalsSeen.empty(); }));
    uint32_t closedId = 0, reason = 0;
    ASSERT_TRUE(client.signalsSeen.front().read(closedId, reason));
    EXPECT_EQ(closedId, id);
    EXPECT_EQ(reason, 3u);
    test::runFor(std::chrono::milliseconds(10));
    EXPECT_EQ(notifications->banners().count(), 0);

    const dbus::Reply again = client.call(kService, kPath, kService, "CloseNotification", id);
    EXPECT_TRUE(again.isError());
    EXPECT_EQ(again.error().name, "org.freedesktop.DBus.Error.InvalidArgs");

    // A banner that times out goes to the history, without its actions.
    const uint32_t expiring = notify("Later", 0, {}, {"default", "Open"});
    settle();
    client.signalsSeen.clear();
    notifications->banners().invokeDefaultAction(expiring);
    ASSERT_TRUE(test::runUntil([&] { return !client.signalsSeen.empty(); }));
    EXPECT_EQ(client.signalsSeen.front().member(), "ActionInvoked");
    notifications->banners().expired(expiring);
    test::runFor(std::chrono::milliseconds(10));
    ASSERT_EQ(notifications->history().count(), 1);
    EXPECT_EQ(notifications->history().notifications().front().summary, "Later");
    EXPECT_TRUE(notifications->history().notifications().front().actions.empty());
}

TEST_F(NotificationServiceTest, CachesAnImageAsASquareThumbnail)
{
    start();
    std::vector<uint8_t> pixels(size_t(300) * 200 * 4, 0x80);
    const uint32_t id = notify("Photo", 0,
                               {{"image-data", dbus::Variant(ImageData(300, 200, 300 * 4, true, 8, 4, pixels))}});
    settle();
    ASSERT_EQ(notifications->banners().count(), 1);
    const Notification &shown = notifications->banners().notifications().front();
    EXPECT_EQ(shown.id, id);
    ASSERT_FALSE(shown.image.empty());
    EXPECT_EQ(std::filesystem::path(shown.image).parent_path(), directory.filePath("images"));
    const ui::Bitmap thumbnail = ui::Bitmap::decodeFile(shown.image);
    EXPECT_EQ(thumbnail.width(), 132);
    EXPECT_EQ(thumbnail.height(), 132);
    struct stat info = {};
    ASSERT_EQ(stat(directory.filePath("images").c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0777, 0700u);

    // A broken picture still wins over the path, as nothing.
    notify("Broken", 0, {{"image-data", dbus::Variant(ImageData(10000, 1, 4, true, 8, 4, {}))},
                         {"image-path", dbus::Variant(std::string("/tmp/x.png"))}});
    settle();
    EXPECT_TRUE(notifications->banners().notifications().back().image.empty());

    // An icon name in image-path stands in for a missing app icon; a file
    // is the picture.
    notify("Named", 0, {{"image-path", dbus::Variant(std::string("file:///tmp/picture.png"))}});
    settle();
    EXPECT_EQ(notifications->banners().notifications().back().image, "/tmp/picture.png");
}

TEST_F(NotificationServiceTest, FollowsTheApplicationModesAndDoNotDisturb)
{
    start("[Notifications]\nApplications\\org.example.Chat\\Mode=2\n"
          "Applications\\org.example.Chat\\Name=Chat\n");
    // Off: the id is handed out, nothing shows.
    EXPECT_EQ(notify("Muted"), 1u);
    settle();
    EXPECT_EQ(notifications->banners().count(), 0);
    EXPECT_EQ(notifications->history().count(), 0);
    EXPECT_TRUE(client.call(kService, kPath, kService, "CloseNotification", uint32_t(1)).isError());

    // Centre only: straight to the history.
    EXPECT_FALSE(client.call("org.lingmo.Notification", "/Notification", "org.lingmo.Notification",
                             "setApplicationMode", std::string("org.example.Chat"), int32_t(1))
                         .isError());
    notify("Quiet");
    settle();
    EXPECT_EQ(notifications->banners().count(), 0);
    EXPECT_EQ(notifications->history().count(), 1);

    // Saved in shell.conf.
    shell::waitForFileThread();
    base::Settings saved(directory.filePath("shell.conf"));
    EXPECT_EQ(saved.intValue("Notifications/Applications/org.example.Chat/Mode"), 1);

    client.listen("org.lingmo.Notification");
    client.call("org.lingmo.Notification", "/Notification", "org.lingmo.Notification", "setDoNotDisturb", true);
    EXPECT_TRUE(test::runUntil([&] { return !client.signalsSeen.empty(); }));
    EXPECT_EQ(client.signalsSeen.front().member(), "doNotDisturbChanged");
    const dbus::Reply property = client.getProperty("org.lingmo.Notification", "/Notification",
                                                    "org.lingmo.Notification", "doNotDisturb");
    EXPECT_EQ(property.value<dbus::Variant>().value_or(dbus::Variant()).value<bool>(), true);
    notify("Mail", 0, {}, {}, "Mail");
    settle();
    EXPECT_EQ(notifications->banners().count(), 0);
    EXPECT_EQ(notifications->history().count(), 2);

    // A critical notification shows as a banner through both.
    notify("Battery", 0, {{"urgency", dbus::Variant(uint8_t(2))}});
    settle();
    ASSERT_EQ(notifications->banners().count(), 1);
    EXPECT_EQ(notifications->banners().notifications().front().summary, "Battery");
    EXPECT_EQ(notifications->history().count(), 2);
}

TEST_F(NotificationServiceTest, KeepsAtMostFiveBanners)
{
    start();
    for (int i = 0; i < 7; ++i)
        notify("Note " + std::to_string(i));
    settle();
    EXPECT_EQ(notifications->banners().count(), 5);
    EXPECT_EQ(notifications->banners().notifications().front().summary, "Note 2");
    EXPECT_EQ(notifications->history().count(), 2);
    // The history heads with the newest.
    EXPECT_EQ(notifications->history().notifications().front().summary, "Note 1");

    notifications->banners().setBannersSuppressed(true);
    test::runFor(std::chrono::milliseconds(10));
    EXPECT_EQ(notifications->banners().count(), 0);
    EXPECT_EQ(notifications->history().count(), 7);
}

TEST_F(NotificationServiceTest, KeepsTheHistoryAsJson)
{
    directory.write("history",
                    R"([{"service":":1.42","summary":"Hi","body":"Body","appName":"App",)"
                    R"("appIcon":"dialog-information","created":1760000000123,)"
                    R"("desktopEntry":"org.example.App","image":""},)"
                    R"({"service":"S","summary":"","body":"","appName":"","appIcon":"",)"
                    R"("created":1760000001000,"updated":1760000002000,"desktopEntry":"","image":"/tmp/x.png"}])");
    start();
    ASSERT_EQ(notifications->history().count(), 2);
    EXPECT_EQ(notifications->history().notifications()[0].desktopEntry, "org.example.App");
    EXPECT_EQ(notifications->history().notifications()[1].image, "/tmp/x.png");

    // A new one from the first record's application joins it at the top.
    Notification note;
    note.desktopEntry = "org.example.App";
    note.summary = "New";
    note.created = 1760000003000LL;
    notifications->history().add(note);
    EXPECT_EQ(notifications->history().notifications()[0].summary, "New");
    EXPECT_EQ(notifications->history().notifications()[1].desktopEntry, "org.example.App");
    shell::waitForFileThread();

    const auto written = base::json::Value::parse(base::file::read(directory.filePath("history")).value_or(""));
    ASSERT_TRUE(written && written->isArray());
    ASSERT_EQ(written->size(), 3u);
    EXPECT_EQ(Notification::fromJson((*written)[0]).summary, "New");
    EXPECT_EQ(Notification::fromJson((*written)[0]).created, 1760000003000LL);
    EXPECT_FALSE(Notification::fromJson((*written)[0]).updated.has_value());
    EXPECT_EQ(Notification::fromJson((*written)[2]).updated, 1760000002000LL);
    struct stat info = {};
    ASSERT_EQ(stat(directory.filePath("history").c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0777, 0600u);

    notifications->history().remove({0, 2});
    EXPECT_EQ(notifications->history().count(), 1);
    notifications->history().clearAll();
    EXPECT_EQ(notifications->history().count(), 0);
}

TEST_F(NotificationServiceTest, AHistoryInAnotherFormatIsIgnored)
{
    directory.write("history", std::string("\0\0\0\x02\0\0\0\x02", 8));
    start();
    EXPECT_EQ(notifications->history().count(), 0);
}

TEST(NotificationTimeTest, CountsMinutesHoursAndDays)
{
    const int64_t now = 1760000000000LL;
    EXPECT_EQ(shell::friendlyTime(now - 20000, now), "Now");
    EXPECT_EQ(shell::friendlyTime(now - 60000, now), "1 minute ago");
    EXPECT_EQ(shell::friendlyTime(now - 5 * 60000, now), "5 minutes ago");
    EXPECT_EQ(shell::friendlyTime(now - 90 * 60000, now), "2 hours ago");
    EXPECT_EQ(shell::friendlyTime(now - 30 * 3600000LL, now), "1 day ago");
    EXPECT_EQ(shell::friendlyTime(now - 3 * 86400000LL, now), "3 days ago");
    EXPECT_FALSE(shell::friendlyTime(now - 30 * 86400000LL, now).empty());
}

// The cache is its own directory, and only what the shell wrote there goes:
// an older cache's pictures and anything else are left alone.
TEST(NotificationImagesTest, PrunesOnlyItsOwnPictures)
{
    EXPECT_EQ(base::paths::cacheHome() + "/lingmo/notification-images",
              shell::notification_images::defaultDirectory());

    test::TemporaryDirectory directory;
    const std::string images = directory.filePath("images");
    ui::Bitmap picture(base::Size(200, 100), ui::Bitmap::Format::Rgba8888Premultiplied);
    picture.fill(0xff336699u);
    const std::string kept = shell::notification_images::save(images, picture);
    const std::string orphan = shell::notification_images::save(images, picture);
    ASSERT_FALSE(kept.empty());
    ASSERT_FALSE(orphan.empty());
    directory.write("images/note.txt", "not ours");
    directory.write("images/0a1b2c3d.png", "not a uuid");

    shell::notification_images::prune(images, {kept});
    EXPECT_TRUE(std::filesystem::exists(kept));
    EXPECT_FALSE(std::filesystem::exists(orphan));
    EXPECT_TRUE(std::filesystem::exists(images + "/note.txt"));
    EXPECT_TRUE(std::filesystem::exists(images + "/0a1b2c3d.png"));
}

TEST_F(NotificationServiceTest, SoundsFollowNewVisibleNotificationsOnly)
{
    start();
    std::vector<std::string> sounds;
    auto subscription = notifications->banners().soundRequested.connect(
            [&](std::string event) { sounds.push_back(event); });
    const uint32_t first = notify("First");
    settle();
    EXPECT_EQ(sounds, (std::vector<std::string>{"message-new-instant"}));
    notify("Progress", first);
    notify("Silent", 0, {{"suppress-sound", dbus::Variant(true)}});
    settle();
    EXPECT_EQ(sounds.size(), 1u);
    notifications->settings().setDoNotDisturb(true);
    notify("Do not disturb");
    notify("Critical but quiet", 0, {{"urgency", dbus::Variant(uint8_t(2))}});
    settle();
    EXPECT_EQ(sounds.size(), 1u);
    notifications->settings().setDoNotDisturb(false);
    notifications->settings().setApplicationMode("org.example.Chat", NotificationSettings::CenterOnly);
    notify("History only");
    settle();
    EXPECT_EQ(sounds.size(), 1u);
    notifications->settings().setApplicationMode("org.example.Chat", NotificationSettings::Banners);
    notifications->banners().setBannersSuppressed(true);
    notify("Center open");
    settle();
    EXPECT_EQ(sounds.size(), 1u);
    notifications->banners().setBannersSuppressed(false);
    notify("Custom sound", 0, {{"sound-name", dbus::Variant(std::string("dialog-warning"))}});
    settle();
    ASSERT_EQ(sounds.size(), 2u);
    EXPECT_EQ(sounds.back(), "dialog-warning");
}
