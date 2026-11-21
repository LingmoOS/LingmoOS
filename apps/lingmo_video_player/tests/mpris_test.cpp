// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "mprisserver.h"
#include "sleepinhibitor.h"
#include "waitfor.h"

#include "base/dbus.h"
#include "base/event_loop.h"

#include <gtest/gtest.h>

#include <memory>
#include <unistd.h>
#include <vector>

namespace base = Lingmo::base;
namespace dbus = Lingmo::base::dbus;

class MprisTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_bus = dbus::Connection::session(&m_loop);
        m_client = dbus::Connection::open(dbus::Connection::Bus::Session, &m_loop);
        if (!m_bus || !m_client)
            GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";
        m_name = "test" + std::to_string(getpid());
    }

    // A call from another connection, answered through the loop.
    dbus::Reply call(const std::string& interface, const std::string& method, auto... args)
    {
        dbus::Proxy proxy(m_client.get(), "org.mpris.MediaPlayer2." + m_name, "/org/mpris/MediaPlayer2", interface);
        std::optional<dbus::Reply> reply;
        dbus::PendingCall pending = proxy.callAsync(method, [&](const dbus::Reply& answer) { reply = answer; }, args...);
        waitFor(m_loop, [&] { return reply.has_value(); });
        return reply.value_or(dbus::Reply());
    }

    std::optional<dbus::Variant> property(const std::string& interface, const std::string& name)
    {
        const dbus::Reply reply = call("org.freedesktop.DBus.Properties", "Get", interface, name);
        EXPECT_FALSE(reply.isError()) << name << ": " << reply.error().name << " " << reply.error().message;
        return reply.value<dbus::Variant>();
    }

    base::EventLoop m_loop;
    dbus::Connection* m_bus = nullptr;
    std::unique_ptr<dbus::Connection> m_client;
    std::string m_name;
};

static MprisServer::Description description()
{
    MprisServer::Description description;
    description.identity = "Video Player";
    description.desktopEntry = "lingmo-video-player";
    description.supportedUriSchemes = {"file", "http", "https"};
    description.canQuit = true;
    description.canRaise = true;
    description.canSetFullscreen = true;
    return description;
}

TEST_F(MprisTest, PropertiesAndRequests)
{
    MprisServer server(m_bus, m_name, description());
    ASSERT_TRUE(server.isRegistered());
    server.setCanControl(true);
    server.setCanPlay(true);
    server.setCanPause(true);
    server.setCanSeek(true);
    server.setMinimumRate(0.5);
    server.setMaximumRate(2);
    server.setPlaybackStatus(MprisServer::PlaybackStatus::Paused);
    server.setLoopStatus(MprisServer::LoopStatus::Playlist);
    server.setVolume(1.3);
    server.setPosition(5'000'000);
    server.setMetadata({"/org/lingmo/VideoPlayer/Track/0", "Film", 60'000'000, "file:///v/Film.mkv", {}});

    const char* root = "org.mpris.MediaPlayer2";
    const char* player = "org.mpris.MediaPlayer2.Player";
    EXPECT_EQ(property(root, "Identity")->get<std::string>(), "Video Player");
    EXPECT_EQ(property(root, "SupportedUriSchemes")->get<std::vector<std::string>>(),
              (std::vector<std::string>{"file", "http", "https"}));
    EXPECT_EQ(property(player, "PlaybackStatus")->get<std::string>(), "Paused");
    EXPECT_EQ(property(player, "LoopStatus")->get<std::string>(), "Playlist");
    EXPECT_EQ(property(player, "Position")->get<int64_t>(), 5'000'000);
    EXPECT_EQ(property(player, "CanGoNext")->get<bool>(), false);
    const auto metadata = property(player, "Metadata")->get<dbus::VariantMap>();
    ASSERT_TRUE(metadata);
    EXPECT_EQ(metadata->at("mpris:trackid").get<dbus::ObjectPath>()->path, "/org/lingmo/VideoPlayer/Track/0");
    EXPECT_EQ(metadata->at("mpris:length").get<int64_t>(), 60'000'000);
    EXPECT_FALSE(metadata->contains("mpris:artUrl"));

    int play = 0;
    int pause = 0;
    int next = 0;
    std::vector<int64_t> seeks;
    std::vector<double> volumes;
    std::vector<std::string> positions;
    auto s1 = server.playRequested.connect([&] { ++play; });
    auto s2 = server.pauseRequested.connect([&] { ++pause; });
    auto s3 = server.nextRequested.connect([&] { ++next; });
    auto s4 = server.seekRequested.connect([&](int64_t offset) { seeks.push_back(offset); });
    auto s5 = server.volumeRequested.connect([&](double volume) { volumes.push_back(volume); });
    auto s6 = server.setPositionRequested.connect([&](const std::string& track, int64_t) { positions.push_back(track); });

    EXPECT_FALSE(call(player, "PlayPause").isError());
    EXPECT_EQ(play, 1);
    // Already paused: nothing to do.
    EXPECT_FALSE(call(player, "Pause").isError());
    EXPECT_EQ(pause, 0);
    EXPECT_FALSE(call(player, "Seek", int64_t(-2'000'000)).isError());
    // Past the end moves on to the next file.
    EXPECT_FALSE(call(player, "Seek", int64_t(70'000'000)).isError());
    EXPECT_EQ(seeks, std::vector<int64_t>{-2'000'000});
    EXPECT_EQ(next, 1);

    const dbus::Reply wrongTrack = call(player, "SetPosition", dbus::ObjectPath{"/other"}, int64_t(1));
    EXPECT_TRUE(wrongTrack.isError());
    EXPECT_FALSE(call(player, "SetPosition", dbus::ObjectPath{"/org/lingmo/VideoPlayer/Track/0"}, int64_t(1)).isError());
    EXPECT_EQ(positions.size(), 1u);
    EXPECT_TRUE(call(player, "SetPosition", dbus::ObjectPath{"/org/lingmo/VideoPlayer/Track/0"}, int64_t(-1)).isError());
    EXPECT_EQ(positions.size(), 1u);

    EXPECT_FALSE(call("org.freedesktop.DBus.Properties", "Set", std::string(player), std::string("Volume"),
                      dbus::Variant(0.5)).isError());
    EXPECT_EQ(volumes, std::vector<double>{0.5});
    // No MIME type is listed, so nothing opens through MPRIS.
    EXPECT_TRUE(call(player, "OpenUri", std::string("file:///v/a.mkv")).isError());
}

TEST_F(MprisTest, ChangesAreAnnounced)
{
    MprisServer server(m_bus, m_name, description());
    ASSERT_TRUE(server.isRegistered());
    std::vector<std::string> changed;
    dbus::MatchRule rule;
    rule.sender = "org.mpris.MediaPlayer2." + m_name;
    rule.path = "/org/mpris/MediaPlayer2";
    rule.interface = "org.freedesktop.DBus.Properties";
    rule.member = "PropertiesChanged";
    dbus::Match match = m_client->subscribe(
            rule,
            [&](const dbus::Message& message) {
                std::string interface;
                dbus::VariantMap properties;
                std::vector<std::string> invalidated;
                if (message.read(interface, properties, invalidated)) {
                    for (const auto& [name, value] : properties)
                        changed.push_back(name);
                }
            });
    // The bus installs the match rule asynchronously.
    runFor(m_loop, std::chrono::milliseconds(200));
    server.setPlaybackStatus(MprisServer::PlaybackStatus::Playing);
    server.setPosition(1);
    server.setVolume(0.7);
    waitFor(m_loop, [&] { return changed.size() >= 2; });
    runFor(m_loop, std::chrono::milliseconds(50));
    EXPECT_EQ(changed, (std::vector<std::string>{"PlaybackStatus", "Volume"}));
}

TEST_F(MprisTest, ApplicationTakesFilesOverDBus)
{
    Application application("/nonexistent/video-player.conf", "/nonexistent/covers");
    // The service name is the session's; the object is enough for a call.
    ASSERT_TRUE(application.publish(m_bus));
    std::vector<std::string> media;
    std::vector<std::string> activations;
    auto opened = application.openRequested.connect(
            [&](const std::vector<std::string>& files, const std::vector<std::string>&) { media = files; });
    auto activated =
            application.activationRequested.connect([&](const std::string& token) { activations.push_back(token); });

    dbus::Proxy proxy(m_client.get(), m_bus->uniqueName(), Application::kPath, Application::kService);
    std::optional<dbus::Reply> reply;
    dbus::PendingCall pending = proxy.callAsync(
            "open", [&](const dbus::Reply& answer) { reply = answer; },
            std::vector<std::string>{"https://example.com/a.mp4"}, std::string("token"));
    ASSERT_TRUE(waitFor(m_loop, [&] { return reply.has_value(); }));
    EXPECT_FALSE(reply->isError());
    EXPECT_EQ(media, std::vector<std::string>{"https://example.com/a.mp4"});
    EXPECT_EQ(activations, std::vector<std::string>{"token"});
}

// A ScreenSaver on the test bus that hands out cookies and records the ones returned.
class ScreenSaverTest : public MprisTest
{
protected:
    void SetUp() override
    {
        MprisTest::SetUp();
        if (IsSkipped())
            return;
        ASSERT_TRUE(m_client->requestName("org.freedesktop.ScreenSaver"));
        m_service = std::make_unique<dbus::Object>(m_client.get(), "/ScreenSaver", "org.freedesktop.ScreenSaver");
        m_service->addMethod("Inhibit", [this](std::string, std::string) { return ++m_lastCookie; });
        m_service->addMethod("UnInhibit", [this](uint32_t cookie) { m_released.push_back(cookie); });
        ASSERT_TRUE(m_service->publish());
    }

    std::unique_ptr<dbus::Object> m_service;
    uint32_t m_lastCookie = 0;
    std::vector<uint32_t> m_released;
};

TEST_F(ScreenSaverTest, CookieReturnedWhenReleased)
{
    {
        SleepInhibitor inhibitor(m_bus, m_bus);
        inhibitor.setMode(SleepInhibitor::Mode::Screen);
        ASSERT_TRUE(waitFor(m_loop, [&] { return inhibitor.isHeld(); }));
    }
    EXPECT_TRUE(waitFor(m_loop, [&] { return !m_released.empty(); }));
    EXPECT_EQ(m_released, std::vector<uint32_t>{1});
}

// The reply comes after the inhibitor went: its cookie still goes back.
TEST_F(ScreenSaverTest, LateCookieReturned)
{
    {
        SleepInhibitor inhibitor(m_bus, m_bus);
        inhibitor.setMode(SleepInhibitor::Mode::Screen);
    }
    EXPECT_TRUE(waitFor(m_loop, [&] { return !m_released.empty(); }));
    EXPECT_EQ(m_released, std::vector<uint32_t>{1});
}
