// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"
#include "lingmo/display/display_client.h"

using namespace Lingmo::Base;
using namespace Lingmo::Display;
using namespace std::chrono_literals;

static constexpr const char* kService = "org.lingmo.DisplayConfig.ClientTest";
static constexpr const char* kPath = "/org/lingmo/DisplayConfig1";
static constexpr const char* kInterface = "org.lingmo.DisplayConfig1";

// Runs the loop until the condition holds or the time is up.
static bool waitFor(EventLoop& loop, const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5s)
{
    if (condition())
        return true;
    Timer poll(&loop);
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    Timer limit(&loop);
    limit.setSingleShot(true);
    limit.onTimeout = [&loop] { loop.quit(); };
    poll.start(5ms);
    limit.start(timeout);
    loop.run();
    return condition();
}

namespace {

// The compositor side, on a connection of its own as it would be in another process.
class FakeDisplayConfig
{
public:
    enum class Answer {
        BeforeReply,
        AfterReply,
        Never,
    };

    explicit FakeDisplayConfig(EventLoop& loop)
        : m_loop(loop)
        , m_bus(dbus::Connection::open(dbus::Connection::Bus::Session, &loop))
    {
    }

    ~FakeDisplayConfig() { stop(); }

    bool start()
    {
        m_object = std::make_unique<dbus::Object>(m_bus.get(), kPath, kInterface);
        m_object->addMethod("SetDockedLidClosed", [this](bool closed) {
            dockedRequests.push_back(closed);
            return finish(succeeded, error);
        });
        m_object->addMethod("SetDisplayPower", [this](const std::vector<std::string>&, uint32_t state) {
            powerRequests.push_back(state);
            return finish(true, ErrorCode::None);
        });
        m_object->addMethod("SetNightLight", [this](double strength, uint32_t durationMs) {
            nightLightRequests.emplace_back(strength, durationMs);
        });
        m_object->addSignal("OperationFinished", "sbbusastt");
        return m_object->publish() && m_bus->requestName(kService);
    }

    void stop()
    {
        if (!m_object)
            return;
        m_bus->releaseName(kService);
        m_object.reset();
    }

    Answer answer = Answer::AfterReply;
    // For SetDockedLidClosed; SetDisplayPower always succeeds.
    bool succeeded = true;
    ErrorCode error = ErrorCode::None;
    std::vector<bool> dockedRequests;
    std::vector<uint32_t> powerRequests;
    std::vector<std::pair<double, uint32_t>> nightLightRequests;

private:
    std::string finish(bool success, ErrorCode code)
    {
        const std::string token = "token-" + std::to_string(++m_serial);
        const auto report = [this, token, success, code] {
            if (m_object)
                m_object->emitSignal("OperationFinished", token, success, false, uint32_t(code), std::string(),
                                     std::vector<std::string>(), uint64_t(1), uint64_t(1));
        };
        if (answer == Answer::BeforeReply)
            report();
        else if (answer == Answer::AfterReply)
            m_loop.post(report);
        return token;
    }

    EventLoop& m_loop;
    std::unique_ptr<dbus::Connection> m_bus;
    std::unique_ptr<dbus::Object> m_object;
    int m_serial = 0;
};

struct Result
{
    bool succeeded = false;
    ErrorCode error = ErrorCode::None;
};

} // namespace

TEST(DisplayClientTest, RequestsNeedTheService)
{
    EventLoop loop;
    DisplayClient client(kService);
    EXPECT_FALSE(client.setDockedLidClosed(true, [](bool, ErrorCode) {
        ADD_FAILURE() << "a request that was never sent must not complete";
    }));
    EXPECT_FALSE(client.setDisplayPower(false));
    EXPECT_FALSE(client.setNightLight(0.5, 1000));
    waitFor(loop, [] { return false; }, 100ms);
}

TEST(DisplayClientTest, SendsTheNightLight)
{
    EventLoop loop;
    FakeDisplayConfig service(loop);
    ASSERT_TRUE(service.start());
    DisplayClient client(kService);
    EXPECT_TRUE(client.setNightLight(0.25, 60000));
    ASSERT_TRUE(waitFor(loop, [&] { return service.nightLightRequests.size() == 1; }));
    EXPECT_EQ(service.nightLightRequests.front().first, 0.25);
    EXPECT_EQ(service.nightLightRequests.front().second, 60000u);
}

TEST(DisplayClientTest, CompletesWhenTheOperationFinishesAfterTheReply)
{
    EventLoop loop;
    FakeDisplayConfig service(loop);
    ASSERT_TRUE(service.start());
    DisplayClient client(kService);
    std::optional<Result> result;
    ASSERT_TRUE(client.setDockedLidClosed(true, [&](bool succeeded, ErrorCode error) {
        result = Result{succeeded, error};
    }));
    ASSERT_TRUE(waitFor(loop, [&] { return result.has_value(); }));
    EXPECT_TRUE(result->succeeded);
    EXPECT_EQ(result->error, ErrorCode::None);
    EXPECT_EQ(service.dockedRequests, std::vector<bool>{true});
}

TEST(DisplayClientTest, CompletesWhenARejectionArrivesBeforeTheReply)
{
    EventLoop loop;
    FakeDisplayConfig service(loop);
    service.answer = FakeDisplayConfig::Answer::BeforeReply;
    service.succeeded = false;
    service.error = ErrorCode::Busy;
    ASSERT_TRUE(service.start());
    DisplayClient client(kService);
    std::optional<Result> result;
    ASSERT_TRUE(client.setDockedLidClosed(true, [&](bool succeeded, ErrorCode error) {
        result = Result{succeeded, error};
    }));
    ASSERT_TRUE(waitFor(loop, [&] { return result.has_value(); }));
    EXPECT_FALSE(result->succeeded);
    EXPECT_EQ(result->error, ErrorCode::Busy);
}

TEST(DisplayClientTest, EachRequestGetsItsOwnResult)
{
    EventLoop loop;
    FakeDisplayConfig service(loop);
    service.answer = FakeDisplayConfig::Answer::BeforeReply;
    ASSERT_TRUE(service.start());
    DisplayClient client(kService);
    std::optional<Result> power;
    std::optional<Result> docked;
    ASSERT_TRUE(client.setDisplayPower(false, [&](bool succeeded, ErrorCode error) {
        power = Result{succeeded, error};
    }));
    // Read when the call is handled, after this one is sent too.
    service.succeeded = false;
    service.error = ErrorCode::NoEnabledOutput;
    ASSERT_TRUE(client.setDockedLidClosed(true, [&](bool succeeded, ErrorCode error) {
        docked = Result{succeeded, error};
    }));
    ASSERT_TRUE(waitFor(loop, [&] { return power.has_value() && docked.has_value(); }));
    EXPECT_TRUE(power->succeeded);
    EXPECT_FALSE(docked->succeeded);
    EXPECT_EQ(docked->error, ErrorCode::NoEnabledOutput);
    EXPECT_EQ(service.powerRequests, std::vector<uint32_t>{1});
}

TEST(DisplayClientTest, LosingTheServiceFailsWhatIsPending)
{
    EventLoop loop;
    FakeDisplayConfig service(loop);
    service.answer = FakeDisplayConfig::Answer::Never;
    ASSERT_TRUE(service.start());
    DisplayClient client(kService);
    std::optional<Result> result;
    ASSERT_TRUE(client.setDockedLidClosed(true, [&](bool succeeded, ErrorCode error) {
        result = Result{succeeded, error};
    }));
    ASSERT_TRUE(waitFor(loop, [&] { return service.dockedRequests.size() == 1; }));
    // Let the reply arrive, so the request waits on its OperationFinished.
    waitFor(loop, [] { return false; }, 100ms);
    EXPECT_FALSE(result.has_value());
    service.stop();
    ASSERT_TRUE(waitFor(loop, [&] { return result.has_value(); }));
    EXPECT_FALSE(result->succeeded);
    EXPECT_EQ(result->error, ErrorCode::Unavailable);
}
