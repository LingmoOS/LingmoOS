// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/detached_call.h"

#include "base/dbus.h"
#include "base/event_loop.h"

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <poll.h>
#include <vector>

#include <gtest/gtest.h>

using namespace Lingmo::Base;

namespace {

class DetachedCallTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        service = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        client = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        ASSERT_TRUE(service && client);
        object = std::make_unique<dbus::Object>(service.get(), "/test", "org.lingmo.Test");
        object->addMethod("Echo", [](const std::string &text) { return text; });
        object->addMethod("Hold", "", "", [this](dbus::MethodCall &call) {
            held.push_back(call);
        });
        ASSERT_TRUE(object->publish());
    }

    dbus::Message call(const std::string &method) const
    {
        return client->newMethodCall(service->uniqueName(), "/test", "org.lingmo.Test", method);
    }

    bool runUntil(const std::function<bool()> &condition)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!condition()) {
            if (std::chrono::steady_clock::now() >= deadline)
                return condition();
            pollfd fd {loop.fd(), POLLIN, 0};
            ::poll(&fd, 1, 10);
            loop.dispatch();
        }
        return true;
    }

    EventLoop loop;
    std::unique_ptr<dbus::Connection> service;
    std::unique_ptr<dbus::Connection> client;
    std::unique_ptr<dbus::Object> object;
    std::vector<dbus::MethodCall> held;
};

} // namespace

TEST_F(DetachedCallTest, theAnswerArrivesAndTheCallbackGoes)
{
    auto alive = std::make_shared<int>(0);
    std::optional<std::string> answer;
    dbus::Message echo = call("Echo");
    echo.append(std::string("pong"));
    Lingmo::Core::callDetached(client.get(), echo, [alive, &answer](const dbus::Reply &reply) {
        answer = reply.value<std::string>();
    });

    ASSERT_TRUE(runUntil([&] { return answer.has_value(); }));
    EXPECT_EQ(*answer, "pong");
    EXPECT_EQ(alive.use_count(), 1);
}

TEST_F(DetachedCallTest, aConnectionThatClosesFirstTakesTheCallWithIt)
{
    auto alive = std::make_shared<int>(0);
    bool answered = false;
    Lingmo::Core::callDetached(client.get(), call("Hold"), [alive, &answered](const dbus::Reply &) {
        answered = true;
    });
    ASSERT_TRUE(runUntil([&] { return !held.empty(); }));

    client.reset();
    EXPECT_EQ(alive.use_count(), 1);
    EXPECT_FALSE(answered);
}

TEST_F(DetachedCallTest, aCallWithoutAConnectionFailsAtOnce)
{
    std::optional<dbus::Reply> answer;
    Lingmo::Core::callDetached(nullptr, dbus::Message(), [&answer](const dbus::Reply &reply) {
        answer = reply;
    });

    ASSERT_TRUE(answer.has_value());
    EXPECT_TRUE(answer->isError());
}
