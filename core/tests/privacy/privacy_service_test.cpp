// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "privacyservice.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/timer.h"

#include <csignal>
#include <filesystem>
#include <map>
#include <tuple>

#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>

namespace dbus = Lingmo::base::dbus;
using namespace std::chrono_literals;

// polkit, answering every check with "authorized" after @p delay, as when an
// administrator types a password. Runs in a child process of its own.
[[noreturn]] static void runSlowPolkit(int readyFd, std::chrono::milliseconds delay)
{
    Lingmo::base::EventLoop loop;
    dbus::Connection *bus = dbus::Connection::session(&loop);
    if (!bus)
        _exit(1);

    dbus::Object authority(bus, "/org/freedesktop/PolicyKit1/Authority", "org.freedesktop.PolicyKit1.Authority");
    std::vector<std::unique_ptr<Lingmo::base::Timer>> answers;
    authority.addMethod("CheckAuthorization", "(sa{sv})sa{ss}us", "(bba{ss})", [&](dbus::MethodCall &call) {
        auto timer = std::make_unique<Lingmo::base::Timer>();
        timer->setSingleShot(true);
        timer->onTimeout = [call]() mutable {
            call.reply(std::tuple<bool, bool, std::map<std::string, std::string>>(true, false, {}));
        };
        timer->start(delay);
        answers.push_back(std::move(timer));
    });
    if (!authority.publish() || !bus->requestName("org.freedesktop.PolicyKit1"))
        _exit(1);

    const char ready = 'R';
    if (::write(readyFd, &ready, 1) != 1)
        _exit(1);
    ::close(readyFd);
    _exit(loop.run());
}

TEST(PrivacyServiceTest, StaysUpUntilAPendingAuthorizationIsAnswered)
{
    int ready[2];
    ASSERT_EQ(::pipe(ready), 0);
    const pid_t polkit = ::fork();
    ASSERT_GE(polkit, 0);
    if (polkit == 0) {
        ::close(ready[0]);
        runSlowPolkit(ready[1], 1000ms);
    }
    ::close(ready[1]);
    char marker = 0;
    ASSERT_EQ(::read(ready[0], &marker, 1), 1);
    ::close(ready[0]);

    Lingmo::base::file::TemporaryDirectory state(std::filesystem::temp_directory_path().string(), "privacy-test-");
    ASSERT_TRUE(state.isValid());

    Lingmo::base::EventLoop loop;
    dbus::Connection *bus = dbus::Connection::session(&loop);
    ASSERT_NE(bus, nullptr);

    // Turning the camera on while it is on changes no device.
    PrivacyService service(state.path(), 200ms);
    ASSERT_TRUE(service.exportTo(bus));
    ASSERT_TRUE(bus->requestName("org.lingmo.Privacy"));

    bool replied = false;
    bool idleBeforeReply = false;
    std::string error;
    service.onIdle = [&] {
        idleBeforeReply = !replied;
        loop.quit(0);
    };

    Lingmo::base::Timer deadline;
    deadline.setSingleShot(true);
    deadline.onTimeout = [&] {
        loop.quit(1);
    };
    deadline.start(10s);

    const dbus::Proxy privacy(bus, "org.lingmo.Privacy", "/org/lingmo/Privacy", "org.lingmo.Privacy");
    const dbus::PendingCall call = privacy.callAsync(
            "setCameraEnabled",
            [&](const dbus::Reply &reply) {
                replied = true;
                if (reply.isError())
                    error = reply.error().name;
            },
            true);

    const int result = loop.run();
    ::kill(polkit, SIGTERM);
    ::waitpid(polkit, nullptr, 0);

    ASSERT_EQ(result, 0) << "the service never went idle";
    EXPECT_FALSE(idleBeforeReply);
    EXPECT_TRUE(replied);
    EXPECT_EQ(error, "");
    EXPECT_TRUE(service.cameraEnabled());
}
