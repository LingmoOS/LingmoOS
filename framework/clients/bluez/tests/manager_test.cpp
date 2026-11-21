// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>

#include "base/timer.h"
#include "lingmo/bluez/devices_model.h"
#include "lingmo/bluez/manager.h"
#include "fake_bluez.h"

using namespace Lingmo::Base;
using namespace Lingmo::bluez;
using namespace fake;
using namespace std::chrono_literals;

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

static void wait(EventLoop& loop, std::chrono::milliseconds time)
{
    waitFor(loop, [] { return false; }, time);
}

static int count(const std::vector<std::string>& calls, const std::string& member)
{
    return int(std::count(calls.begin(), calls.end(), member));
}

static int indexOf(const std::vector<std::string>& calls, const std::string& member)
{
    const auto found = std::find(calls.begin(), calls.end(), member);
    return found == calls.end() ? -1 : int(found - calls.begin());
}

class ManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        server = std::make_unique<FakeBluez>(loop);
        ASSERT_TRUE(server->registerService());
        manager = std::make_shared<Manager>(dbus::Connection::session(&loop));
        ASSERT_TRUE(waitFor(loop, [this] { return manager->operational(); }));
        ASSERT_TRUE(waitFor(loop, [this] { return manager->devices().size() == 1; }));
    }

    void TearDown() override
    {
        manager.reset();
        server.reset();
    }

    void enableAgent()
    {
        manager->setAgentEnabled(true);
        ASSERT_TRUE(waitFor(loop, [this] { return manager->agentReady(); }));
    }

    EventLoop loop;
    std::unique_ptr<FakeBluez> server;
    std::shared_ptr<Manager> manager;
};

TEST_F(ManagerTest, AvailabilityDoesNotDependOnPower)
{
    EXPECT_TRUE(manager->available());
    int availability = 0;
    const auto subscription = manager->availableChanged.connect([&availability] { ++availability; });
    manager->setEnabled(false);
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->enabled(); }));
    EXPECT_TRUE(manager->available());
    EXPECT_EQ(availability, 0);
    const Interfaces properties = server->objects[{adapter}];
    server->remove(adapter, {adapterInterface});
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->available(); }));
    EXPECT_TRUE(manager->operational());
    EXPECT_EQ(availability, 1);
    server->add(adapter, properties);
    ASSERT_TRUE(waitFor(loop, [this] { return manager->available(); }));
    EXPECT_EQ(availability, 2);
    server->unregisterService();
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->available(); }));
    EXPECT_FALSE(manager->operational());
}

TEST_F(ManagerTest, PropertiesAndHotplug)
{
    DevicesModel model(manager);
    EXPECT_EQ(model.count(), 1);
    EXPECT_TRUE(manager->enabled());
    EXPECT_EQ(manager->devices().front()->name(), "Headphones");
    const auto entry = manager->object(device);
    EXPECT_EQ(entry->batteryPercentage(), -1);
    server->add(device, {{"org.bluez.Battery1", {{"Percentage", uint8_t(72)}}}});
    ASSERT_TRUE(waitFor(loop, [&] { return entry->batteryPercentage() == 72; }));
    server->remove(device, {"org.bluez.Battery1"});
    ASSERT_TRUE(waitFor(loop, [&] { return entry->batteryPercentage() == -1; }));
    server->set(device, deviceInterface, "Alias", std::string("Renamed"));
    ASSERT_TRUE(waitFor(loop, [&] { return entry->name() == "Renamed"; }));
    manager->setEnabled(false);
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->enabled(); }));
    manager->setEnabled(true);
    ASSERT_TRUE(waitFor(loop, [this] { return manager->enabled(); }));
    server->remove(adapter, {adapterInterface});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->adapters().empty(); }));
    ASSERT_TRUE(waitFor(loop, [&] { return model.count() == 0; }));
}

TEST_F(ManagerTest, RemovedChildObjectIsDropped)
{
    const std::string transport = device + "/sep1/fd0";
    const std::string transportInterface = "org.bluez.MediaTransport1";
    server->add(transport, {{transportInterface, {{"State", std::string("idle")}, {"Device", dbus::ObjectPath{device}}}},
                            {"org.freedesktop.DBus.Properties", {}}});
    ASSERT_TRUE(waitFor(loop, [&] { return manager->object(transport) != nullptr; }));
    std::weak_ptr<BluetoothObject> entry = manager->object(transport);
    server->remove(transport, {transportInterface});
    ASSERT_TRUE(waitFor(loop, [&] { return entry.expired(); }));
    EXPECT_FALSE(manager->object(transport));
    EXPECT_EQ(manager->devices().size(), 1u);
}

TEST_F(ManagerTest, ModelKeepsOrder)
{
    DevicesModel model(manager);
    int rowChanges = 0;
    const auto subscription = model.rowsChanged.connect([&rowChanges] { ++rowChanges; });
    const auto nameAt = [&model](int row) { return model.devices()[row]->name(); };
    const std::string second = adapter + "/dev_11_11_11_11_11_11";
    server->add(second, {{deviceInterface,
                          {{"Address", std::string("11:11:11:11:11:11")},
                           {"Alias", std::string("Zebra")},
                           {"Adapter", dbus::ObjectPath{adapter}},
                           {"Paired", false},
                           {"Connected", false}}}});
    ASSERT_TRUE(waitFor(loop, [&] { return model.count() == 2; }));
    EXPECT_EQ(nameAt(0), "Headphones");
    EXPECT_EQ(nameAt(1), "Zebra");
    const int added = rowChanges;
    server->set(second, deviceInterface, "Connected", true);
    ASSERT_TRUE(waitFor(loop, [&] { return nameAt(0) == "Zebra"; }));
    EXPECT_EQ(rowChanges, added + 1);
    server->remove(second, {deviceInterface});
    ASSERT_TRUE(waitFor(loop, [&] { return model.count() == 1; }));
    EXPECT_EQ(nameAt(0), "Headphones");
}

TEST_F(ManagerTest, ModelFilters)
{
    const std::string unnamed = adapter + "/dev_22_22_22_22_22_22";
    server->add(unnamed, {{deviceInterface,
                           {{"Address", std::string("22:22:22:22:22:22")},
                            {"Alias", std::string("22-22-22-22-22-22")},
                            {"Adapter", dbus::ObjectPath{adapter}},
                            {"Paired", false},
                            {"Connected", false}}}});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->devices().size() == 2; }));
    DevicesModel paired;
    paired.setFilter(DevicesModel::PairedDevices);
    paired.setManager(manager);
    DevicesModel other;
    other.setFilter(DevicesModel::UnpairedDevices);
    other.setNamedOnly(true);
    other.setManager(manager);
    EXPECT_EQ(paired.count(), 0);
    EXPECT_EQ(other.count(), 1);
    server->set(device, deviceInterface, "Paired", true);
    ASSERT_TRUE(waitFor(loop, [&] { return paired.count() == 1; }));
    EXPECT_EQ(other.count(), 0);
    other.setNamedOnly(false);
    EXPECT_EQ(other.count(), 1);
}

TEST_F(ManagerTest, DiscoveryOwnership)
{
    server->set(adapter, adapterInterface, "Discovering", true);
    ASSERT_TRUE(waitFor(loop, [this] { return manager->scanning(); }));
    EXPECT_EQ(count(server->calls, "StartDiscovery"), 0);
    manager->setDiscoveryRequested(true);
    ASSERT_TRUE(waitFor(loop, [this] { return count(server->calls, "StartDiscovery") == 1; }));
    manager->setDiscoveryRequested(true);
    manager->setDiscoveryRequested(false);
    ASSERT_TRUE(waitFor(loop, [this] { return count(server->calls, "StopDiscovery") == 1; }));
    manager->setDiscoveryRequested(false);
    wait(loop, 30ms);
    EXPECT_EQ(count(server->calls, "StopDiscovery"), 1);
}

TEST_F(ManagerTest, PairTrustConnectForget)
{
    enableAgent();
    const auto entry = manager->object(device);
    entry->pair();
    ASSERT_TRUE(waitFor(loop, [&] { return entry->paired() && entry->trusted() && entry->connected() && !entry->busy(); }));
    EXPECT_LT(indexOf(server->calls, "Pair"), indexOf(server->calls, "Set"));
    EXPECT_LT(indexOf(server->calls, "Set"), indexOf(server->calls, "Connect"));
    entry->disconnectFromDevice();
    ASSERT_TRUE(waitFor(loop, [&] { return !entry->connected() && !entry->busy(); }));
    entry->forget();
    ASSERT_TRUE(waitFor(loop, [this] { return manager->devices().empty(); }));
    EXPECT_EQ(server->lastPath, adapter);
}

TEST_F(ManagerTest, DeviceInformation)
{
    const std::string service = device + "/service000e";
    const std::string characteristic = "org.bluez.GattCharacteristic1";
    const auto add = [&](const std::string& name, const std::string& uuid, const std::string& value) {
        server->add(service + name, {{characteristic, {{"UUID", uuid},
                                                       {"Value", std::vector<uint8_t>(value.begin(), value.end())}}}});
    };
    add("/char000f", "00002a29-0000-1000-8000-00805f9b34fb", "Logitech");
    add("/char0011", "00002a24-0000-1000-8000-00805f9b34fb", "Pebble K390");
    add("/char0015", "00002A26-0000-1000-8000-00805F9B34FB", std::string("RBK80.00_0011\0\0", 15));
    ASSERT_TRUE(waitFor(loop, [&] { return manager->object(service + "/char0015") != nullptr; }));
    const auto entry = manager->object(device);
    entry->readDeviceInformation();
    wait(loop, 30ms);
    EXPECT_EQ(count(server->calls, "ReadValue"), 0);
    server->set(device, deviceInterface, "Connected", true);
    ASSERT_TRUE(waitFor(loop, [&] { return entry->connected(); }));
    entry->readDeviceInformation();
    ASSERT_TRUE(waitFor(loop, [&] { return entry->modelNumber() == "Pebble K390"; }));
    ASSERT_TRUE(waitFor(loop, [&] { return entry->firmwareRevision() == "RBK80.00_0011"; }));
    EXPECT_EQ(count(server->calls, "ReadValue"), 2);
}

TEST_F(ManagerTest, PairThroughDefaultAgent)
{
    EXPECT_FALSE(manager->agentEnabled());
    const auto entry = manager->object(device);
    entry->pair();
    ASSERT_TRUE(waitFor(loop, [&] { return entry->paired() && entry->trusted() && !entry->busy(); }));
    EXPECT_TRUE(manager->errorText().empty());
}

TEST_F(ManagerTest, DisplayedCodeClosesWhenPaired)
{
    enableAgent();
    bool finished = false;
    const dbus::PendingCall display = server->ask(
        "DisplayPasskey", [&finished](const dbus::Reply&) { finished = true; }, dbus::ObjectPath{device},
        uint32_t(7), uint16_t(0));
    ASSERT_TRUE(waitFor(loop, [&] { return finished; }));
    EXPECT_EQ(manager->pairingKind(), "DisplayPasskey");
    server->set(device, deviceInterface, "Paired", true);
    ASSERT_TRUE(waitFor(loop, [this] { return manager->pairingKind().empty(); }));
}

TEST_F(ManagerTest, FailedPairDoesNotTrust)
{
    enableAgent();
    server->failMethod = "Pair";
    int failures = 0;
    const auto subscription = manager->deviceFailed.connect([&failures](std::string, std::string) { ++failures; });
    const auto entry = manager->object(device);
    entry->pair();
    ASSERT_TRUE(waitFor(loop, [&] { return failures == 1 && !entry->busy(); }));
    EXPECT_FALSE(entry->trusted());
    EXPECT_EQ(count(server->calls, "Connect"), 0);
    EXPECT_TRUE(manager->errorText().empty());
}

TEST_F(ManagerTest, ConnectFailureIsExplained)
{
    server->failMethod = "Connect";
    server->failMessage = "br-connection-page-timeout";
    std::vector<std::pair<std::string, std::string>> failures;
    const auto subscription = manager->deviceFailed.connect([&failures](std::string summary, std::string body) {
        failures.emplace_back(summary, body);
    });
    const auto entry = manager->object(device);
    entry->connectToDevice();
    ASSERT_TRUE(waitFor(loop, [&] { return failures.size() == 1; }));
    EXPECT_NE(failures[0].first.find(entry->name()), std::string::npos);
    EXPECT_NE(failures[0].second.find("didn't respond"), std::string::npos);
    EXPECT_EQ(failures[0].second.find("br-connection"), std::string::npos);
    EXPECT_TRUE(manager->errorText().empty());
}

TEST_F(ManagerTest, DismissedPairingIsQuiet)
{
    server->failMethod = "Pair";
    server->failName = "org.bluez.Error.AuthenticationCanceled";
    int failures = 0;
    const auto subscription = manager->deviceFailed.connect([&failures](std::string, std::string) { ++failures; });
    const auto entry = manager->object(device);
    entry->pair();
    ASSERT_TRUE(waitFor(loop, [&] { return !entry->busy(); }));
    wait(loop, 30ms);
    EXPECT_EQ(failures, 0);
}

TEST_F(ManagerTest, ConfirmationAndValidation)
{
    enableAgent();
    std::optional<dbus::Reply> confirmation;
    const dbus::PendingCall confirm = server->ask(
        "RequestConfirmation", [&](const dbus::Reply& reply) { confirmation = reply; }, dbus::ObjectPath{device},
        uint32_t(42));
    ASSERT_TRUE(waitFor(loop, [this] { return manager->pairingCode() == "000042"; }));
    EXPECT_FALSE(confirmation);
    manager->respondPairing(true);
    ASSERT_TRUE(waitFor(loop, [&] { return confirmation.has_value(); }));
    EXPECT_FALSE(confirmation->isError());

    std::optional<dbus::Reply> passkey;
    const dbus::PendingCall askPasskey = server->ask(
        "RequestPasskey", [&](const dbus::Reply& reply) { passkey = reply; }, dbus::ObjectPath{device});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->pairingKind() == "RequestPasskey"; }));
    manager->respondPairing(true, "1000000");
    wait(loop, 30ms);
    EXPECT_FALSE(passkey);
    manager->respondPairing(true, "000123");
    ASSERT_TRUE(waitFor(loop, [&] { return passkey.has_value(); }));
    EXPECT_EQ(passkey->message().signature(), "u");
    EXPECT_EQ(passkey->value<uint32_t>(), 123u);

    std::optional<dbus::Reply> pin;
    const dbus::PendingCall askPin = server->ask(
        "RequestPinCode", [&](const dbus::Reply& reply) { pin = reply; }, dbus::ObjectPath{device});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->pairingKind() == "RequestPinCode"; }));
    manager->respondPairing(true, std::string());
    wait(loop, 30ms);
    EXPECT_FALSE(pin);
    manager->respondPairing(true, "abc123");
    ASSERT_TRUE(waitFor(loop, [&] { return pin.has_value(); }));
    EXPECT_EQ(pin->value<std::string>(), "abc123");
}

TEST_F(ManagerTest, CancellationAndConcurrentRequests)
{
    enableAgent();
    std::optional<dbus::Reply> first;
    std::optional<dbus::Reply> second;
    const dbus::PendingCall askFirst = server->ask(
        "RequestAuthorization", [&](const dbus::Reply& reply) { first = reply; }, dbus::ObjectPath{device});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->pairingKind() == "RequestAuthorization"; }));
    const dbus::PendingCall askSecond = server->ask(
        "RequestPinCode", [&](const dbus::Reply& reply) { second = reply; }, dbus::ObjectPath{device});
    ASSERT_TRUE(waitFor(loop, [&] { return second.has_value(); }));
    EXPECT_EQ(second->error().name, "org.bluez.Error.Rejected");
    manager->respondPairing(false);
    ASSERT_TRUE(waitFor(loop, [&] { return first.has_value(); }));
    EXPECT_EQ(first->error().name, "org.bluez.Error.Rejected");

    bool displayed = false;
    const dbus::PendingCall display = server->ask(
        "DisplayPasskey", [&](const dbus::Reply&) { displayed = true; }, dbus::ObjectPath{device}, uint32_t(7),
        uint16_t(2));
    ASSERT_TRUE(waitFor(loop, [&] { return displayed; }));
    EXPECT_EQ(manager->pairingCode(), "000007");
    EXPECT_EQ(manager->pairingEntered(), 2);
    bool cancelled = false;
    const dbus::PendingCall cancel = server->ask("Cancel", [&](const dbus::Reply&) { cancelled = true; });
    ASSERT_TRUE(waitFor(loop, [&] { return cancelled; }));
    EXPECT_TRUE(manager->pairingKind().empty());
}

TEST_F(ManagerTest, RestartAndPendingOperation)
{
    enableAgent();
    server->holdPair = true;
    std::weak_ptr<BluetoothObject> oldDevice = manager->object(device);
    oldDevice.lock()->pair();
    ASSERT_TRUE(waitFor(loop, [this] { return server->pendingPair.has_value(); }));
    server->unregisterService();
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->operational(); }));
    EXPECT_TRUE(manager->devices().empty());
    EXPECT_TRUE(oldDevice.expired());
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor(loop, [this] { return manager->operational() && manager->agentReady(); }));
    EXPECT_EQ(manager->devices().size(), 1u);
    server->pendingPair->reply();
    server->pendingPair.reset();
    wait(loop, 30ms);
    EXPECT_FALSE(manager->devices().front()->trusted());
    EXPECT_EQ(count(server->calls, "Connect"), 0);
}

TEST_F(ManagerTest, ClosingPageCancelsPendingPair)
{
    enableAgent();
    server->holdPair = true;
    const auto entry = manager->object(device);
    entry->pair();
    ASSERT_TRUE(waitFor(loop, [this] { return server->pendingPair.has_value(); }));
    manager->setAgentEnabled(false);
    ASSERT_TRUE(waitFor(loop, [this] { return count(server->calls, "CancelPairing") > 0; }));
    ASSERT_TRUE(waitFor(loop, [&] { return !entry->busy(); }));
    EXPECT_FALSE(entry->trusted());
}

TEST_F(ManagerTest, RejectForeignAgentCaller)
{
    enableAgent();
    std::unique_ptr<dbus::Connection> foreign = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
    dbus::Message message = foreign->newMethodCall(server->agentOwner, server->agentPath, "org.bluez.Agent1",
                                                   "RequestAuthorization");
    message.append(dbus::ObjectPath{device});
    std::optional<dbus::Reply> reply;
    const dbus::PendingCall call = foreign->callAsync(message, [&reply](const dbus::Reply& r) { reply = r; });
    ASSERT_TRUE(waitFor(loop, [&] { return reply.has_value(); }));
    EXPECT_EQ(reply->error().name, "org.bluez.Error.Rejected");
    EXPECT_TRUE(manager->pairingKind().empty());
}

TEST_F(ManagerTest, SnapshotPreservesConcurrentEvents)
{
    server->unregisterService();
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->operational(); }));
    server->holdSnapshot = true;
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor(loop, [this] { return server->pendingSnapshot.has_value(); }));
    server->set(device, deviceInterface, "Alias", std::string("New name"));
    server->add(device, {{"org.bluez.Battery1", {{"Percentage", uint8_t(50)}}}});
    server->pendingSnapshot->reply(server->snapshot);
    ASSERT_TRUE(waitFor(loop, [this] { return manager->operational(); }));
    EXPECT_EQ(manager->object(device)->name(), "New name");
    EXPECT_EQ(manager->object(device)->batteryPercentage(), 50);
}

TEST_F(ManagerTest, InvalidatedPropertiesDoNotOverwriteNewSignals)
{
    server->holdGetAll = true;
    server->bus->emitSignal(device, "org.freedesktop.DBus.Properties", "PropertiesChanged", deviceInterface,
                            dbus::VariantMap(), std::vector<std::string>{"Alias"});
    ASSERT_TRUE(waitFor(loop, [this] { return server->pendingGetAll.has_value(); }));
    server->set(device, deviceInterface, "Alias", std::string("New alias"));
    ASSERT_TRUE(waitFor(loop, [this] { return manager->object(device)->name() == "New alias"; }));
    server->pendingGetAll->reply(server->oldProperties);
    wait(loop, 30ms);
    EXPECT_EQ(manager->object(device)->name(), "New alias");
}

TEST_F(ManagerTest, RefreshOfRemovedObjectIsNotAnError)
{
    server->holdGetAll = true;
    server->bus->emitSignal(device, "org.freedesktop.DBus.Properties", "PropertiesChanged", deviceInterface,
                            dbus::VariantMap(), std::vector<std::string>{"RSSI"});
    ASSERT_TRUE(waitFor(loop, [this] { return server->pendingGetAll.has_value(); }));
    server->remove(device, {deviceInterface});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->devices().empty(); }));
    server->pendingGetAll->replyError("org.freedesktop.DBus.Error.UnknownObject", "Method \"GetAll\" doesn't exist");
    wait(loop, 30ms);
    EXPECT_TRUE(manager->errorText().empty());
}

TEST_F(ManagerTest, FailedDiscoveryCanBeRetried)
{
    server->failMethod = "StartDiscovery";
    manager->setDiscoveryRequested(true);
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->discoveryRequested(); }));
    EXPECT_FALSE(manager->errorText().empty());
    EXPECT_EQ(manager->errorText().find("Injected failure"), std::string::npos);
    server->failMethod.clear();
    manager->setDiscoveryRequested(true);
    ASSERT_TRUE(waitFor(loop, [this] { return manager->scanning(); }));
    server->failMethod = "StopDiscovery";
    manager->setDiscoveryRequested(false);
    ASSERT_TRUE(waitFor(loop, [this] { return count(server->calls, "StopDiscovery") > 0; }));
    server->failMethod.clear();
    ASSERT_TRUE(waitFor(loop, [this] { return !manager->scanning(); }));
}

TEST_F(ManagerTest, MultipleAdapters)
{
    const std::string secondAdapter = "/org/bluez/hci1";
    const std::string secondDevice = secondAdapter + "/dev_AA_BB_CC_DD_EE_FF";
    server->add(secondAdapter, {{adapterInterface, {{"Powered", true}}}});
    server->add(secondDevice, {{deviceInterface,
                                {{"Address", std::string("AA:BB:CC:DD:EE:FF")},
                                 {"Adapter", dbus::ObjectPath{secondAdapter}}}}});
    ASSERT_TRUE(waitFor(loop, [this] { return manager->devices().size() == 2; }));
    manager->object(secondDevice)->connectToDevice();
    ASSERT_TRUE(waitFor(loop, [&] { return manager->object(secondDevice)->connected(); }));
    EXPECT_FALSE(manager->object(device)->connected());
}
