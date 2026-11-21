// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <tuple>

#include "base/event_loop.h"
#include "lingmo/upower/battery_status.h"
#include "lingmo/upower/device.h"

using namespace Lingmo;
using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace Lingmo::upower;
using namespace std::chrono_literals;

namespace {

constexpr const char* kDevicePath = "/org/freedesktop/UPower/devices/battery_BAT0";
constexpr const char* kDeviceInterface = "org.freedesktop.UPower.Device";

// lingmo-services' battery and UPower on the session bus, on a thread of
// their own as in other processes.
class FakeServices
{
public:
    explicit FakeServices(bool battery)
        : m_battery(battery)
    {
        std::promise<bool> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        m_ready = ready.get_future().get();
    }
    ~FakeServices()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    bool isReady() const { return m_ready; }
    // Runs on the services' thread.
    template<typename F>
    auto inService(F f)
    {
        std::promise<decltype(f())> result;
        m_handle.post([&] {
            if constexpr (std::is_void_v<decltype(f())>) {
                f();
                result.set_value();
            } else {
                result.set_value(f());
            }
        });
        return result.get_future().get();
    }

    // lingmo-services' battery.
    int32_t chargePercent = 64;
    int64_t lastChargedSecs = 1700000000;
    int refreshes = 0;
    std::unique_ptr<Object> batteryObject;

    // UPower.
    bool onBattery = false;
    bool thresholdEnabled = false;
    bool enableAskedInteractively = false;
    std::tuple<std::string, uint32_t, uint32_t> historyRequest;
    std::unique_ptr<Object> upowerObject;
    std::unique_ptr<Object> deviceObject;
    double displayPercentage = 41.6;
    std::unique_ptr<Object> displayObject;

private:
    void run(std::promise<bool>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<Connection> connection = Connection::open(Connection::Bus::Session, &loop);
        if (!connection || !connection->requestName("org.lingmo.Services")
            || !connection->requestName("org.freedesktop.UPower")) {
            ready.set_value(false);
            return;
        }

        bool published = true;
        if (m_battery) {
            batteryObject = std::make_unique<Object>(connection.get(), "/org/lingmo/Services/Battery",
                                                     "org.lingmo.Services.Battery");
            Object& battery = *batteryObject;
            battery.addProperty("chargeState", [] { return int32_t(2); });
            battery.addProperty("chargePercent", [this] { return chargePercent; });
            battery.addProperty("capacity", [] { return int32_t(93); });
            battery.addProperty("statusString", [] { return std::string("Discharging"); });
            battery.addProperty("lastChargedPercent", [] { return int32_t(100); });
            battery.addProperty("lastChargedSecs", [this] { return lastChargedSecs; });
            battery.addProperty("udi", [] { return std::string(kDevicePath); });
            battery.addMethod("refresh", [this] { ++refreshes; });
            battery.addSignal("chargePercentChanged", "i");
            published = battery.publish();
        }

        upowerObject = std::make_unique<Object>(connection.get(), "/org/freedesktop/UPower", "org.freedesktop.UPower");
        upowerObject->addProperty("OnBattery", [this] { return onBattery; });
        published = upowerObject->publish() && published;

        deviceObject = std::make_unique<Object>(connection.get(), kDevicePath, kDeviceInterface);
        Object& device = *deviceObject;
        device.addProperty("NativePath", [] { return std::string("BAT0"); });
        device.addProperty("ChargeThresholdSupported", [] { return true; });
        device.addProperty("ChargeThresholdEnabled", [this] { return thresholdEnabled; });
        device.addProperty("ChargeStartThreshold", [] { return uint32_t(70); });
        device.addProperty("ChargeEndThreshold", [] { return uint32_t(90); });
        device.addMethod("EnableChargeThreshold", "b", "", [this](MethodCall& call) {
            bool enabled = false;
            call.read(enabled);
            enableAskedInteractively = call.message().allowsInteractiveAuthorization();
            if (enabled == thresholdEnabled) {
                call.replyError("org.freedesktop.UPower.GeneralError", "Unchanged");
                return;
            }
            thresholdEnabled = enabled;
            deviceObject->emitPropertiesChanged({"ChargeThresholdEnabled"});
            call.reply();
        });
        device.addMethod("GetHistory", [this](const std::string& type, uint32_t timespan, uint32_t resolution) {
            historyRequest = {type, timespan, resolution};
            // Newest first, as UPower returns them.
            return std::vector<std::tuple<uint32_t, double, uint32_t>>{{300, 80.5, 1}, {200, 70, 2}, {100, 75, 2}};
        });
        published = device.publish() && published;

        displayObject = std::make_unique<Object>(connection.get(), "/org/freedesktop/UPower/devices/DisplayDevice",
                                                 kDeviceInterface);
        displayObject->addProperty("IsPresent", [this] { return m_battery; });
        displayObject->addProperty("Type", [this] { return uint32_t(m_battery ? 2 : 0); });
        displayObject->addProperty("Percentage", [this] { return displayPercentage; });
        published = displayObject->publish() && published;

        ready.set_value(published);
        loop.run();
        batteryObject.reset();
        upowerObject.reset();
        deviceObject.reset();
        displayObject.reset();
    }

    bool m_battery;
    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    bool m_ready = false;
};

class UPowerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!Connection::session(&m_loop))
            GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";
    }

    void startServices(bool battery = true)
    {
        m_services = std::make_unique<FakeServices>(battery);
        ASSERT_TRUE(m_services->isReady());
    }

    // Runs the loop until the condition holds or two seconds pass.
    bool waitFor(const std::function<bool()>& condition)
    {
        const auto deadline = std::chrono::steady_clock::now() + 2s;
        while (!condition()) {
            if (std::chrono::steady_clock::now() > deadline)
                return false;
            runFor(5ms);
        }
        return true;
    }

    void runFor(std::chrono::milliseconds time)
    {
        const int timer = m_loop.startTimer(time, false, [this] { m_loop.quit(); });
        m_loop.run();
        m_loop.stopTimer(timer);
    }

    Connection* bus() { return Connection::session(&m_loop); }

    EventLoop m_loop;
    std::unique_ptr<FakeServices> m_services;
};

} // namespace

TEST_F(UPowerTest, ReadyWithoutABattery)
{
    startServices(false);
    BatteryStatus status(bus(), bus());
    ASSERT_TRUE(waitFor([&] { return status.isReady(); }));
    EXPECT_FALSE(status.isAvailable());
    EXPECT_EQ(status.chargePercent(), 0);
}

TEST_F(UPowerTest, BatteryStatusReadsAndFollowsTheService)
{
    startServices();
    BatteryStatus status(bus(), bus());
    int available = 0;
    Subscription availableSubscription = status.availableChanged.connect([&] { ++available; });
    ASSERT_TRUE(waitFor([&] { return status.isReady(); }));
    EXPECT_TRUE(status.isAvailable());
    EXPECT_EQ(available, 1);
    EXPECT_EQ(status.chargeState(), 2);
    EXPECT_EQ(status.chargePercent(), 64);
    EXPECT_EQ(status.capacity(), 93);
    EXPECT_EQ(status.statusString(), "Discharging");
    EXPECT_EQ(status.lastChargedPercent(), 100);
    EXPECT_EQ(status.lastChargedSecs(), 1700000000);
    EXPECT_EQ(status.udi(), kDevicePath);

    // A change signal is only a cue: the values come from a fresh read.
    int percent = 0;
    Subscription percentSubscription = status.chargePercentChanged.connect([&] { ++percent; });
    m_services->inService([&] {
        m_services->chargePercent = 65;
        m_services->batteryObject->emitSignal("chargePercentChanged", int32_t(0));
    });
    ASSERT_TRUE(waitFor([&] { return status.chargePercent() == 65; }));
    EXPECT_EQ(percent, 1);

    status.refresh();
    ASSERT_TRUE(waitFor([&] { return m_services->inService([&] { return m_services->refreshes; }) == 1; }));
}

TEST_F(UPowerTest, ThePowerSourceFollowsUPower)
{
    startServices();
    BatteryStatus status(bus(), bus());
    ASSERT_TRUE(waitFor([&] { return status.isReady(); }));
    EXPECT_FALSE(status.onBattery());
    int changes = 0;
    Subscription subscription = status.onBatteryChanged.connect([&] { ++changes; });
    m_services->inService([&] {
        m_services->onBattery = true;
        m_services->upowerObject->emitPropertiesChanged({"OnBattery"});
    });
    ASSERT_TRUE(waitFor([&] { return status.onBattery(); }));
    EXPECT_EQ(changes, 1);
    // The service's figures depend on the power source, so it reads again.
    ASSERT_TRUE(waitFor([&] { return m_services->inService([&] { return m_services->refreshes; }) == 1; }));
}

TEST_F(UPowerTest, WithoutASessionBusTheDisplayDeviceIsRead)
{
    startServices();
    BatteryStatus status(nullptr, bus());
    ASSERT_TRUE(waitFor([&] { return status.isReady(); }));
    EXPECT_TRUE(status.isAvailable());
    EXPECT_EQ(status.chargePercent(), 42);
    EXPECT_EQ(status.statusString(), "");

    int percent = 0;
    Subscription subscription = status.chargePercentChanged.connect([&] { ++percent; });
    m_services->inService([&] {
        m_services->displayPercentage = 80.2;
        m_services->displayObject->emitPropertiesChanged({"Percentage"});
    });
    ASSERT_TRUE(waitFor([&] { return status.chargePercent() == 80; }));
    EXPECT_EQ(percent, 1);

    // Nothing asks a service that is not there.
    status.refresh();
    runFor(50ms);
    EXPECT_EQ(m_services->inService([&] { return m_services->refreshes; }), 0);
}

TEST_F(UPowerTest, WithoutASessionBusOrABatteryNothingIsAvailable)
{
    startServices(false);
    BatteryStatus status(nullptr, bus());
    ASSERT_TRUE(waitFor([&] { return status.isReady(); }));
    EXPECT_FALSE(status.isAvailable());
}

TEST_F(UPowerTest, SharedStatusIsOneWhileHeld)
{
    std::shared_ptr<BatteryStatus> first = BatteryStatus::shared();
    std::shared_ptr<BatteryStatus> second = BatteryStatus::shared();
    EXPECT_EQ(first, second);
    std::weak_ptr<BatteryStatus> held = first;
    first.reset();
    second.reset();
    EXPECT_TRUE(held.expired());
    EXPECT_TRUE(BatteryStatus::shared());
}

TEST_F(UPowerTest, DeviceReadsItsChargeLimit)
{
    startServices();
    Device device(kDevicePath, bus());
    int changes = 0;
    Subscription subscription = device.propertiesChanged.connect([&](const VariantMap&) { ++changes; });
    ASSERT_TRUE(waitFor([&] { return device.isReady(); }));
    EXPECT_EQ(changes, 1);
    EXPECT_EQ(device.nativePath(), "BAT0");
    EXPECT_TRUE(device.chargeThresholdSupported());
    EXPECT_FALSE(device.chargeThresholdEnabled());
    EXPECT_EQ(device.chargeStartThreshold(), 70);
    EXPECT_EQ(device.chargeEndThreshold(), 90);
}

TEST_F(UPowerTest, EnablingTheChargeLimitMayAskAndAnnouncesIt)
{
    startServices();
    Device device(kDevicePath, bus());
    ASSERT_TRUE(waitFor([&] { return device.isReady(); }));
    bool answered = false;
    Error error;
    device.enableChargeThreshold(true, [&](const Error& e) {
        answered = true;
        error = e;
    });
    ASSERT_TRUE(waitFor([&] { return answered; }));
    EXPECT_FALSE(error.isSet());
    EXPECT_TRUE(m_services->inService([&] { return m_services->enableAskedInteractively; }));
    ASSERT_TRUE(waitFor([&] { return device.chargeThresholdEnabled(); }));

    // A refusal comes back as an error.
    answered = false;
    device.enableChargeThreshold(true, [&](const Error& e) {
        answered = true;
        error = e;
    });
    ASSERT_TRUE(waitFor([&] { return answered; }));
    EXPECT_TRUE(error.isSet());
}

TEST_F(UPowerTest, HistoryComesBackAsSamples)
{
    startServices();
    Device device(kDevicePath, bus());
    std::optional<std::vector<HistorySample>> samples;
    bool answered = false;
    PendingCall call = device.history(Device::HistoryType::Charge, 86400, 100,
                                      [&](const std::optional<std::vector<HistorySample>>& result) {
                                          samples = result;
                                          answered = true;
                                      });
    ASSERT_TRUE(waitFor([&] { return answered; }));
    ASSERT_TRUE(samples);
    ASSERT_EQ(samples->size(), 3u);
    EXPECT_EQ((*samples)[0].time, 300u);
    EXPECT_EQ((*samples)[0].value, 80.5);
    EXPECT_EQ((*samples)[0].state, 1u);
    EXPECT_EQ((*samples)[2].time, 100u);
    const auto request = m_services->inService([&] { return m_services->historyRequest; });
    EXPECT_EQ(std::get<0>(request), "charge");
    EXPECT_EQ(std::get<1>(request), 86400u);
    EXPECT_EQ(std::get<2>(request), 100u);

    answered = false;
    call = device.history(Device::HistoryType::Rate, 600, 10,
                          [&](const std::optional<std::vector<HistorySample>>& result) {
                              samples = result;
                              answered = true;
                          });
    ASSERT_TRUE(waitFor([&] { return answered; }));
    EXPECT_EQ(std::get<0>(m_services->inService([&] { return m_services->historyRequest; })), "rate");
}

TEST_F(UPowerTest, HistoryOfAMissingDeviceIsEmpty)
{
    startServices();
    Device device("/org/freedesktop/UPower/devices/battery_BAT9", bus());
    bool answered = false;
    std::optional<std::vector<HistorySample>> samples = std::vector<HistorySample>{};
    PendingCall call = device.history(Device::HistoryType::Charge, 86400, 100,
                                      [&](const std::optional<std::vector<HistorySample>>& result) {
                                          samples = result;
                                          answered = true;
                                      });
    ASSERT_TRUE(waitFor([&] { return answered; }));
    EXPECT_FALSE(samples);
}
