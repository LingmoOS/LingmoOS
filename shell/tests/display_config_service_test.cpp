// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>

#include "base/dbus.h"
#include "core/main_loop.h"
#include "display/display_configuration_controller.h"
#include "display/display_manager.h"
#include "fake_native_display_delegate.h"
#include "ipc/display_config_service.h"
#include "loop_test_util.h"

using namespace Lingmo;
namespace dbus = Lingmo::base::dbus;

namespace {

constexpr const char *kService = "org.lingmo.DisplayConfig.ServiceTest";

struct NightLightCall {
    double strength;
    std::chrono::milliseconds duration;
};

// The session is never on screen here: the night light goes through anyway.
struct Service {
    display::DisplayManager manager;
    test::FakeNativeDisplayDelegate delegate;
    display::DisplayConfigurationController controller {&manager, &delegate};
    display::ipc::DisplayConfigService service {&manager, &controller,
                                                dbus::Connection::session(core::mainLoop()),
                                                [] { return false; }};
    std::optional<NightLightCall> nightLight;

    Service()
    {
        service.setNightLightHandler([this](double strength, std::chrono::milliseconds duration) {
            nightLight = NightLightCall {strength, duration};
        });
    }
};

class DisplayConfigServiceTest : public ::testing::Test
{
protected:
    void SetUp() override { ASSERT_NE(client, nullptr); }

    // Waits with the loops running: the service answers from this thread.
    template<typename... Args>
    dbus::Reply call(const char *method, const Args &...arguments)
    {
        dbus::Proxy proxy(client.get(), kService, "/org/lingmo/DisplayConfig1",
                          "org.lingmo.DisplayConfig1");
        std::optional<dbus::Reply> reply;
        auto pending = proxy.callAsync(
                method, [&reply](const dbus::Reply &result) { reply = result; }, arguments...);
        if (!test::runUntil([&reply] { return reply.has_value(); }))
            return {};
        return *reply;
    }

    std::unique_ptr<dbus::Connection> client =
            dbus::Connection::open(dbus::Connection::Bus::Session, core::mainLoop());
};

} // namespace

TEST_F(DisplayConfigServiceTest, NightLightReachesTheCompositorWhileTheSessionIsInactive)
{
    Service service;
    ASSERT_TRUE(service.service.registerService(kService));

    const auto reply = call("SetNightLight", 0.5, uint32_t(1000));
    EXPECT_FALSE(reply.isError()) << reply.error().name;
    ASSERT_TRUE(service.nightLight);
    EXPECT_EQ(service.nightLight->strength, 0.5);
    EXPECT_EQ(service.nightLight->duration.count(), 1000);
}

TEST_F(DisplayConfigServiceTest, TheStrengthIsClampedToTheRange)
{
    Service service;
    ASSERT_TRUE(service.service.registerService(kService));

    call("SetNightLight", 2.5, uint32_t(0));
    ASSERT_TRUE(service.nightLight);
    EXPECT_EQ(service.nightLight->strength, 1.0);
    call("SetNightLight", -1.0, uint32_t(0));
    EXPECT_EQ(service.nightLight->strength, 0.0);
}

TEST_F(DisplayConfigServiceTest, AStrengthThatIsNotANumberIsRefused)
{
    Service service;
    ASSERT_TRUE(service.service.registerService(kService));

    const auto reply =
            call("SetNightLight", std::numeric_limits<double>::quiet_NaN(), uint32_t(0));
    EXPECT_TRUE(reply.isError());
    EXPECT_EQ(reply.error().name, "org.freedesktop.DBus.Error.InvalidArgs");
    EXPECT_FALSE(service.nightLight);
}

// The night light's exception does not open the configuration writes.
TEST_F(DisplayConfigServiceTest, OtherWritesStillNeedTheSessionOnScreen)
{
    Service service;
    ASSERT_TRUE(service.service.registerService(kService));

    const auto reply = call("RevertConfiguration", std::string());
    EXPECT_TRUE(reply.isError());
    EXPECT_EQ(reply.error().name, "org.lingmo.DisplayConfig1.Error.Unauthorized");
}

// The signature clients expect: the XML once listed one string too few after
// `as`.
TEST_F(DisplayConfigServiceTest, StateKeepsItsSignature)
{
    Service service;
    ASSERT_TRUE(service.service.registerService(kService));

    const auto reply = call("GetState");
    ASSERT_FALSE(reply.isError()) << reply.error().name;
    EXPECT_EQ(reply.message().signature(),
              "sstta(sssssssasssiiuuua(iiiiiiubbs))a(ssasiiiiiiiiuub)a(ssui)st");
}

// Introspection names every argument, as the interface XML does.
TEST_F(DisplayConfigServiceTest, IntrospectionNamesTheArguments)
{
    Service service;
    ASSERT_TRUE(service.service.registerService(kService));

    dbus::Proxy introspectable(client.get(), kService, "/org/lingmo/DisplayConfig1",
                               "org.freedesktop.DBus.Introspectable");
    std::optional<dbus::Reply> reply;
    auto pending = introspectable.callAsync(
            "Introspect", [&reply](const dbus::Reply &result) { reply = result; });
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    std::string xml;
    ASSERT_TRUE(reply->read(xml)) << reply->error().name;
    for (const char *argument :
         {R"x(type="a(sb(iiiiiiubs)uu)" name="displays" direction="in")x",
          R"x(type="s" name="operationToken" direction="out")x",
          R"x(type="a(sssssssasssiiuuua(iiiiiiubbs))" name="physicalDisplays" direction="out")x",
          R"x(type="t" name="previewDeadlineMs" direction="out")x",
          R"x(type="d" name="strength" direction="in")x",
          R"x(type="as" name="affectedDisplayIds")x", R"x(type="t" name="deadlineMs")x"})
        EXPECT_NE(xml.find(argument), std::string::npos) << argument;
}
