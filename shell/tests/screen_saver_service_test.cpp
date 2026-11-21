// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/dbus.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "session/screen_saver_service.h"
#include "session/session_controller.h"

#include <gtest/gtest.h>
#include <optional>
#include <unistd.h>

using namespace Lingmo;
namespace dbus = base::dbus;

namespace {

class ScreenSaverServiceTest : public testing::Test
{
protected:
    std::unique_ptr<dbus::Connection> client = dbus::Connection::open(dbus::Connection::Bus::Session, core::mainLoop());

    // A call in flight; its reply lands in `reply`.
    struct Pending {
        std::optional<dbus::Reply> reply;
        dbus::PendingCall call;
        bool finished() const { return reply.has_value(); }
    };

    template<typename... Args>
    std::unique_ptr<Pending> callAt(const std::string &method, const std::string &path,
                                    dbus::Connection *bus, const Args &...args)
    {
        auto pending = std::make_unique<Pending>();
        Pending *raw = pending.get();
        dbus::Proxy proxy(bus ? bus : client.get(), "org.freedesktop.ScreenSaver", path,
                          "org.freedesktop.ScreenSaver");
        pending->call = proxy.callAsync(
                method, [raw](const dbus::Reply &reply) { raw->reply = reply; }, args...);
        return pending;
    }

    template<typename... Args>
    std::unique_ptr<Pending> call(const std::string &method, const Args &...args)
    {
        return callAt(method, "/ScreenSaver", nullptr, args...);
    }

    dbus::Reply finished(std::unique_ptr<Pending> pending)
    {
        EXPECT_TRUE(test::runUntil([&] { return pending->finished(); }));
        return pending->reply.value_or(dbus::Reply());
    }

    bool waitFor(const std::function<bool()> &done) { return test::runUntil(done); }
};

} // namespace

TEST_F(ScreenSaverServiceTest, LockReplyWaitsForEveryOutputAndRepeatedRequestsAreIdempotent)
{
    ASSERT_TRUE(client);
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    controller.setOutputs({{"one", base::Rect(0, 0, 800, 600)},
                           {"two", base::Rect(800, 0, 800, 600)}});
    session::ScreenSaverService service(&controller);
    ASSERT_TRUE(service.registerService());
    std::vector<bool> changed;
    auto subscription = service.activeChanged.connect([&](bool active) { changed.push_back(active); });
    auto first = call("Lock");
    EXPECT_TRUE(waitFor([&] { return controller.state() == session::SessionState::Locking; }));
    EXPECT_TRUE(controller.isBlocked());
    EXPECT_TRUE(root.isVisible());
    EXPECT_FALSE(first->finished());
    auto second = call("Lock");
    const dbus::Reply active = finished(call("GetActive"));
    EXPECT_TRUE(active.value<bool>().value_or(false));
    controller.notifyOutputPresented("one");
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(first->finished());
    EXPECT_FALSE(second->finished());
    controller.notifyOutputPresented("two");
    EXPECT_TRUE(waitFor([&] { return first->finished() && second->finished(); }));
    EXPECT_FALSE(first->reply->isError());
    EXPECT_FALSE(second->reply->isError());
    EXPECT_EQ(controller.state(), session::SessionState::Locked);
    EXPECT_EQ(changed.size(), 1u);
    EXPECT_FALSE(finished(call("Lock")).isError());
    EXPECT_EQ(changed.size(), 1u);
    EXPECT_TRUE(controller.requestUnlock());
    EXPECT_TRUE(service.GetActive());
    controller.notifyFramePresented();
    EXPECT_FALSE(service.GetActive());
    ASSERT_EQ(changed.size(), 2u);
    EXPECT_FALSE(changed.back());
}

TEST_F(ScreenSaverServiceTest, LockDuringAnUnlockIsAnsweredOnceLockedAgain)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    controller.setOutputs({{"one", base::Rect(0, 0, 800, 600)}});
    session::ScreenSaverService service(&controller);
    ASSERT_TRUE(service.registerService());
    ASSERT_TRUE(controller.requestLock());
    controller.notifyOutputPresented("one");
    ASSERT_TRUE(controller.requestUnlock());

    auto lock = call("Lock");
    EXPECT_TRUE(waitFor([&] { return controller.state() == session::SessionState::Locking; }));
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(lock->finished());
    controller.notifyOutputPresented("one");
    EXPECT_FALSE(finished(std::move(lock)).isError());
    EXPECT_EQ(controller.state(), session::SessionState::Locked);
}

TEST_F(ScreenSaverServiceTest, LockIsNotAnsweredWhileTheDisplayLayoutIsRebuilt)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    controller.setOutputs({{"one", base::Rect(0, 0, 800, 600)}});
    session::ScreenSaverService service(&controller);
    ASSERT_TRUE(service.registerService());

    auto lock = call("Lock");
    EXPECT_TRUE(waitFor([&] { return controller.state() == session::SessionState::Locking; }));
    controller.setOutputs({});
    controller.notifyFramePresented();
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(lock->finished());
    EXPECT_EQ(controller.state(), session::SessionState::Locking);

    controller.setOutputs({{"one", base::Rect(0, 0, 800, 600)}});
    controller.notifyOutputPresented("one");
    EXPECT_FALSE(finished(std::move(lock)).isError());
}

TEST_F(ScreenSaverServiceTest, TheBusHearsActiveChangedAtBothPaths)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    session::ScreenSaverService service(&controller);
    ASSERT_TRUE(service.registerService());
    std::vector<std::string> paths;
    dbus::MatchRule rule;
    rule.interface = "org.freedesktop.ScreenSaver";
    rule.member = "ActiveChanged";
    auto match = client->subscribe(rule, [&](const dbus::Message &message) {
        if (message.argument<bool>().value_or(false))
            paths.push_back(message.path());
    });
    // The match is in place once a call behind it is answered.
    finished(call("GetActive"));
    service.Lock();
    EXPECT_TRUE(waitFor([&] { return paths.size() == 2; }));
}

TEST_F(ScreenSaverServiceTest, NoDbusMethodCanUnlock)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    session::ScreenSaverService service(&controller);
    controller.setOutputs({{"one", base::Rect(0, 0, 800, 600)}});
    ASSERT_TRUE(service.registerService());
    auto lock = call("Lock");
    EXPECT_TRUE(waitFor([&] { return controller.state() == session::SessionState::Locking; }));
    controller.notifyOutputPresented("one");
    EXPECT_FALSE(finished(std::move(lock)).isError());
    EXPECT_EQ(controller.state(), session::SessionState::Locked);
    EXPECT_TRUE(finished(call("Unlock")).isError());
    EXPECT_EQ(controller.state(), session::SessionState::Locked);
}

TEST_F(ScreenSaverServiceTest, LockAndInhibitAnswerAtBothPaths)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    session::ScreenSaverService service(&controller);
    int count = 0;
    service.setInhibition([&count] { ++count; }, [&count] { --count; });
    ASSERT_TRUE(service.registerService());
    for (const std::string path : {"/ScreenSaver", "/org/freedesktop/ScreenSaver"}) {
        EXPECT_FALSE(finished(callAt("GetActive", path, nullptr)).isError());
        const dbus::Reply cookie = finished(
                callAt("Inhibit", path, nullptr, std::string("test"), std::string("video")));
        EXPECT_FALSE(cookie.isError());
        EXPECT_NE(cookie.value<uint32_t>().value_or(0), 0u);
    }
    EXPECT_EQ(count, 2);
}

TEST_F(ScreenSaverServiceTest, InhibitCookiesBelongToTheirCaller)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    session::ScreenSaverService service(&controller);
    int count = 0;
    service.setInhibition([&count] { ++count; }, [&count] { --count; });
    ASSERT_TRUE(service.registerService());
    auto first = dbus::Connection::open(dbus::Connection::Bus::Session, core::mainLoop());
    auto second = dbus::Connection::open(dbus::Connection::Bus::Session, core::mainLoop());
    const std::string path = "/org/freedesktop/ScreenSaver";
    const auto inhibit = [&](dbus::Connection *bus) {
        return finished(callAt("Inhibit", path, bus, std::string("test"), std::string("video")));
    };
    dbus::Reply reply = inhibit(first.get());
    ASSERT_FALSE(reply.isError());
    const uint32_t cookie = reply.value<uint32_t>().value_or(0);
    EXPECT_NE(cookie, 0u);
    EXPECT_EQ(count, 1);
    // Another caller cannot release it.
    EXPECT_FALSE(finished(callAt("UnInhibit", path, second.get(), cookie)).isError());
    EXPECT_EQ(count, 1);
    reply = inhibit(first.get());
    EXPECT_NE(reply.value<uint32_t>().value_or(0), cookie);
    EXPECT_EQ(count, 2);
    finished(callAt("UnInhibit", path, first.get(), cookie));
    EXPECT_EQ(count, 1);
    finished(callAt("UnInhibit", path, first.get(), cookie));
    EXPECT_EQ(count, 1);
    // Leaving the bus releases everything the caller still holds.
    first.reset();
    EXPECT_TRUE(waitFor([&] { return count == 0; }));
}

TEST_F(ScreenSaverServiceTest, InhibitFromACallerThatAlreadyLeftIsReleased)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure);
    session::ScreenSaverService service(&controller);
    int acquired = 0;
    int count = 0;
    service.setInhibition(
            [&] {
                ++acquired;
                ++count;
            },
            [&count] { --count; });
    ASSERT_TRUE(service.registerService());
    auto gone = dbus::Connection::open(dbus::Connection::Bus::Session, core::mainLoop());
    const std::string name = gone->uniqueName();
    dbus::Message message = gone->newMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                                "org.freedesktop.ScreenSaver", "Inhibit");
    message.append(std::string("test"), std::string("video"));
    // Sent without waiting for the cookie, as dbus-send does; the blocking
    // call behind it makes sure the bus has it.
    ASSERT_TRUE(gone->send(message));
    dbus::Proxy driver(gone.get(), "org.freedesktop.DBus", "/org/freedesktop/DBus",
                       "org.freedesktop.DBus");
    ASSERT_FALSE(driver.call("GetId").isError());
    gone.reset();
    // Blocking calls dispatch nothing here, so the service handles the
    // Inhibit only after the bus has announced that the caller left.
    dbus::Proxy clientDriver(client.get(), "org.freedesktop.DBus", "/org/freedesktop/DBus",
                             "org.freedesktop.DBus");
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (clientDriver.call("NameHasOwner", name).value<bool>().value_or(true)
           && std::chrono::steady_clock::now() < deadline)
        usleep(10000);
    EXPECT_FALSE(clientDriver.call("NameHasOwner", name).value<bool>().value_or(true));
    EXPECT_TRUE(waitFor([&] { return acquired == 1 && count == 0; }));
}

TEST_F(ScreenSaverServiceTest, InhibitIsRefusedWithoutACounter)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure, nullptr, session::SessionState::Login);
    session::ScreenSaverService service(&controller);
    ASSERT_TRUE(service.registerService());
    EXPECT_TRUE(finished(call("Inhibit", std::string("test"), std::string("video"))).isError());
}

TEST_F(ScreenSaverServiceTest, LoginRemainsDistinctAndBlocked)
{
    scene::Node root;
    session::SecureContainer secure(&root);
    session::SessionController controller(&secure, nullptr, session::SessionState::Login);
    session::ScreenSaverService service(&controller);
    ASSERT_TRUE(service.registerService());
    EXPECT_TRUE(finished(call("Lock")).isError());
    EXPECT_TRUE(service.GetActive());
    EXPECT_EQ(controller.state(), session::SessionState::Login);
}
