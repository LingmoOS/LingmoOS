// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <endian.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <time.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/timer.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/hotspot.h"
#include "lingmo/networkmanager/network_model.h"
#include "lingmo/networkmanager/vpn_utils.h"
#include "fake_network_manager.h"

using namespace std::chrono_literals;
using namespace Lingmo::Networkmanager;
using namespace Lingmo::Networkmanager::variants;
using Lingmo::Base::EventLoop;
using Lingmo::Base::Timer;
using fakenm::FakeNetworkManager;

static EventLoop* loop = nullptr;

// Runs the loop until the condition holds, or a few seconds have passed.
static bool waitFor(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 3000ms)
{
    if (condition())
        return true;
    bool met = false;
    Timer poll;
    poll.onTimeout = [&] {
        if (condition()) {
            met = true;
            loop->quit();
        }
    };
    poll.start(2ms);
    Timer limit;
    limit.setSingleShot(true);
    limit.onTimeout = [&] { loop->quit(); };
    limit.start(timeout);
    loop->run();
    return met || condition();
}

// Lets what is under way settle.
static void wait(std::chrono::milliseconds time)
{
    waitFor([] { return false; }, time);
}

static const std::string wiredDevice = fakenm::managerPath + "/Devices/1";
static const std::string wifiDevice = fakenm::managerPath + "/Devices/2";
static const std::string wiredProfile = fakenm::managerPath + "/Settings/1";
static const std::string homeProfile = fakenm::managerPath + "/Settings/2";
static const std::string wiredActive = fakenm::managerPath + "/ActiveConnection/1";
static const std::string ip4Config = fakenm::managerPath + "/IP4Config/1";
static const std::string homeStrong = fakenm::managerPath + "/AccessPoint/1";
static const std::string homeWeak = fakenm::managerPath + "/AccessPoint/2";
static const std::string cafe = fakenm::managerPath + "/AccessPoint/3";
static const std::string hidden = fakenm::managerPath + "/AccessPoint/4";
static const std::string library = fakenm::managerPath + "/AccessPoint/5";
static const std::string office = fakenm::managerPath + "/AccessPoint/6";
static const std::string homeActive = fakenm::managerPath + "/ActiveConnection/2";
static const std::string officeVpn = fakenm::managerPath + "/Settings/30";
static const std::string branchVpn = fakenm::managerPath + "/Settings/31";
static const std::string homeVpn = fakenm::managerPath + "/Settings/32";
static const std::string vpnInterface = "org.freedesktop.NetworkManager.VPN.Connection";

// NM_802_11_AP_SEC_*: CCMP pairwise and group ciphers with PSK or 802.1X key management.
static constexpr uint32_t rsnPsk = 0x8 | 0x80 | 0x100;
static constexpr uint32_t rsnEap = 0x8 | 0x80 | 0x200;
// WEP40, WEP104, TKIP, CCMP, WPA, RSN and AP.
static constexpr uint32_t wifiCapabilities = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x40;

using Variant = Lingmo::Base::dbus::Variant;
using VariantMap = Lingmo::Base::dbus::VariantMap;

static Variant addressData(const std::string& address)
{
    return Variant(std::vector<VariantMap>{{{"address", Variant(address)}, {"prefix", Variant(uint32_t(24))}}});
}

static fakenm::Interfaces accessPoint(const std::string& ssid, uint32_t flags, uint32_t rsn, uint8_t strength)
{
    static int address = 0;
    char hw[32];
    snprintf(hw, sizeof(hw), "10:00:00:00:00:%02d", ++address);
    return {{fakenm::accessPoint,
             {{"Ssid", fakenm::bytes(ssid)},
              {"Flags", Variant(flags)},
              {"WpaFlags", Variant(uint32_t(0))},
              {"RsnFlags", Variant(rsn)},
              {"Strength", Variant(strength)},
              {"Mode", Variant(uint32_t(2))},
              {"HwAddress", Variant(std::string(hw))},
              {"Frequency", Variant(uint32_t(2412))}}}};
}

// One Ethernet device with its profile up, and one idle Wi-Fi device in range
// of a saved network on two access points, an open one, an enterprise one, a
// WPA2 one and a hidden one.
static void populate(FakeNetworkManager& nm)
{
    nm.settings[wiredProfile] = {
        {"connection",
         {{"id", Variant(std::string("Wired connection 1"))},
          {"uuid", Variant(std::string("u-wired"))},
          {"type", Variant(std::string("802-3-ethernet"))},
          {"timestamp", Variant(uint64_t(1000))}}},
        {"ipv4", {{"method", Variant(std::string("auto"))}}},
    };
    nm.add(wiredProfile, {{fakenm::connection, {{"Flags", Variant(uint32_t(0))}}}}, false);

    nm.settings[homeProfile] = {
        {"connection",
         {{"id", Variant(std::string("Home"))},
          {"uuid", Variant(std::string("u-home"))},
          {"type", Variant(std::string("802-11-wireless"))},
          {"permissions", Variant(std::vector<std::string>{"user:alice:"})},
          {"timestamp", Variant(uint64_t(2000))}}},
        {"802-11-wireless", {{"ssid", fakenm::bytes("Home")}, {"mode", Variant(std::string("infrastructure"))}}},
        {"802-11-wireless-security", {{"key-mgmt", Variant(std::string("wpa-psk"))}, {"psk-flags", Variant(uint32_t(1))}}},
        {"ipv4", {{"method", Variant(std::string("manual"))}, {"address-data", addressData("10.0.0.2")}}},
    };
    nm.add(homeProfile, {{fakenm::connection, {{"Flags", Variant(uint32_t(0))}}}}, false);

    nm.add(wiredActive,
           {{fakenm::active,
             {{"Connection", fakenm::path(wiredProfile)},
              {"Devices", fakenm::paths({wiredDevice})},
              {"State", Variant(uint32_t(2))},
              {"Type", Variant(std::string("802-3-ethernet"))},
              {"Vpn", Variant(false)},
              {"Id", Variant(std::string("Wired connection 1"))}}}},
           false);
    nm.add(ip4Config,
           {{"org.freedesktop.NetworkManager.IP4Config",
             {{"AddressData", addressData("192.168.1.5")},
              {"Gateway", Variant(std::string("192.168.1.1"))},
              {"NameserverData", addressData("192.168.1.1")}}}},
           false);
    nm.add(wiredDevice,
           {{fakenm::device,
             {{"DeviceType", Variant(uint32_t(1))},
              {"Interface", Variant(std::string("eth0"))},
              {"State", Variant(uint32_t(100))},
              {"Managed", Variant(true)},
              {"Autoconnect", Variant(true)},
              {"HwAddress", Variant(std::string("02:00:00:00:00:01"))},
              {"ActiveConnection", fakenm::path(wiredActive)},
              {"AvailableConnections", fakenm::paths({wiredProfile})},
              {"Ip4Config", fakenm::path(ip4Config)},
              {"Ip6Config", fakenm::path({})}}}},
           false);

    nm.add(homeStrong, accessPoint("Home", 0x1, rsnPsk, 70), false);
    nm.add(homeWeak, accessPoint("Home", 0x1, rsnPsk, 40), false);
    nm.add(cafe, accessPoint("Cafe", 0x0, 0, 50), false);
    nm.add(hidden, accessPoint({}, 0x1, rsnPsk, 60), false);
    nm.add(library, accessPoint("Library", 0x1, rsnEap, 30), false);
    nm.add(office, accessPoint("Office", 0x1, rsnPsk, 20), false);
    nm.add(wifiDevice,
           {{fakenm::device,
             {{"DeviceType", Variant(uint32_t(2))},
              {"Interface", Variant(std::string("wlan0"))},
              {"State", Variant(uint32_t(30))},
              {"Managed", Variant(true)},
              {"Autoconnect", Variant(true)},
              {"HwAddress", Variant(std::string("02:00:00:00:00:02"))},
              {"ActiveConnection", fakenm::path({})},
              {"AvailableConnections", fakenm::paths({homeProfile})},
              {"Ip4Config", fakenm::path({})},
              {"Ip6Config", fakenm::path({})}}},
            {fakenm::wireless,
             {{"WirelessCapabilities", Variant(wifiCapabilities)},
              {"Mode", Variant(uint32_t(2))},
              {"ActiveAccessPoint", fakenm::path({})},
              {"AccessPoints", fakenm::paths({homeStrong, homeWeak, cafe, hidden, library, office})}}}},
           false);
}

static int64_t bootTimeMs()
{
    timespec now;
    clock_gettime(CLOCK_BOOTTIME, &now);
    return int64_t(now.tv_sec) * 1000 + now.tv_nsec / 1000000;
}

// A scan old enough for the client to age access points out, which is past a
// minute; scan times count from boot, so a machine just started has none.
static constexpr int64_t kOldScanMs = 61 * 1000;

static std::optional<NetworkItem> row(const NetworkModel& model, const std::string& name)
{
    for (const NetworkItem& item : model.items()) {
        if (item.name == name)
            return item;
    }
    return std::nullopt;
}

namespace {

struct ClientTest : testing::Test
{
    void SetUp() override
    {
        server = Lingmo::Base::dbus::Connection::open(Lingmo::Base::dbus::Connection::Bus::Session);
        clientBus = Lingmo::Base::dbus::Connection::open(Lingmo::Base::dbus::Connection::Bus::Session);
        ASSERT_TRUE(server && clientBus);
        nm = std::make_unique<FakeNetworkManager>(server.get());
        populate(*nm);
        ASSERT_TRUE(nm->serve());
        Client::setInstanceBus(clientBus.get());
        ASSERT_TRUE(waitFor([] { return Client::instance()->ready(); }));
        EXPECT_EQ(Client::instance()->owner(), server->uniqueName());
    }

    void TearDown() override
    {
        nm->stop();
        waitFor([] { return !Client::instance()->ready(); });
        Client::setInstanceBus(nullptr);
        nm.reset();
        server.reset();
        clientBus.reset();
    }

    // The device's last scan ended `ago` ms back and saw every access point it lists.
    void scannedAgo(int64_t ago, int64_t offset = 0)
    {
        const int64_t lastScan = bootTimeMs() - ago + offset;
        for (const std::string& ap : Client::instance()->accessPoints(wifiDevice))
            nm->set(ap, fakenm::accessPoint, "LastSeen", Variant(int32_t(lastScan / 1000)));
        nm->set(wifiDevice, fakenm::wireless, "LastScan", Variant(int64_t(lastScan)));
        ASSERT_TRUE(waitFor([&] {
            return toInt64(Client::instance()->property(wifiDevice, fakenm::wireless, "LastScan")) == lastScan;
        }));
    }

    void dropAccessPoint(const std::string& ap)
    {
        std::vector<std::string> listed = Client::instance()->accessPoints(wifiDevice);
        std::erase(listed, ap);
        nm->set(wifiDevice, fakenm::wireless, "AccessPoints", fakenm::paths(listed));
        nm->remove(ap);
        ASSERT_TRUE(waitFor([&] { return !Client::instance()->has(ap, fakenm::accessPoint); }));
    }

    static std::string otherActive()
    {
        std::string found;
        for (const std::string& path : Client::instance()->activeConnections()) {
            if (path != wiredActive)
                found = path;
        }
        return found;
    }

    void addVpn(const std::string& path, const std::string& name, const std::string& service, uint64_t timestamp)
    {
        std::string uuid = name;
        for (char& c : uuid)
            c = char(std::tolower(static_cast<unsigned char>(c)));
        nm->settings[path] = {
            {"connection",
             {{"id", Variant(name)},
              {"uuid", Variant(uuid)},
              {"type", Variant(std::string("vpn"))},
              {"timestamp", Variant(timestamp)}}},
            {"vpn",
             {{"service-type", Variant(service)},
              {"data", Variant(std::map<std::string, std::string>{{"connection-type", "password"}})}}},
        };
        nm->add(path, {{fakenm::connection, {{"Flags", Variant(uint32_t(0))}}}});
    }

    void addWireGuard(const std::string& path, const std::string& name, uint64_t timestamp)
    {
        nm->settings[path] = {
            {"connection",
             {{"id", Variant(name)},
              {"uuid", Variant(std::string("home wg"))},
              {"type", Variant(std::string("wireguard"))},
              {"timestamp", Variant(timestamp)}}},
            {"wireguard", {{"listen-port", Variant(uint32_t(0))}}},
        };
        nm->add(path, {{fakenm::connection, {{"Flags", Variant(uint32_t(0))}}}});
    }

    std::unique_ptr<Lingmo::Base::dbus::Connection> server;
    std::unique_ptr<Lingmo::Base::dbus::Connection> clientBus;
    std::unique_ptr<FakeNetworkManager> nm;
};

} // namespace

static Settings settingsOf(const fakenm::Call& call)
{
    return call.arguments.empty() ? Settings() : call.arguments[0].get<Settings>().value_or(Settings());
}

static std::string pathArgument(const fakenm::Call& call, size_t index)
{
    return index < call.arguments.size() ? toString(call.arguments[index]) : std::string();
}


TEST_F(ClientTest, ListsNetworksByName)
{
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));

    // Both of its access points make one network, shown by the stronger one.
    const auto home = row(model, "Home");
    ASSERT_TRUE(home);
    EXPECT_FALSE(home->connectionPath.empty());
    EXPECT_EQ(home->connectionPath, homeProfile);
    EXPECT_EQ(home->specificPath, homeStrong);
    EXPECT_EQ(home->signal, 70);
    EXPECT_EQ(home->securityType, SecurityType::Wpa2Psk);

    const auto cafeRow = row(model, "Cafe");
    ASSERT_TRUE(cafeRow);
    EXPECT_TRUE(cafeRow->connectionPath.empty());
    EXPECT_EQ(cafeRow->securityType, SecurityType::None);
    EXPECT_EQ(row(model, "Library")->securityType, SecurityType::Wpa2Eap);

    const auto wired = row(model, "Wired connection 1");
    ASSERT_TRUE(wired);
    EXPECT_EQ(wired->type, ConnectionType::Wired);
    EXPECT_EQ(wired->connectionState, ConnectionStatus::Activated);
    EXPECT_EQ(wired->ipV4Address, "192.168.1.5");
    EXPECT_EQ(wired->gateway, "192.168.1.1");
    EXPECT_EQ(wired->nameServer, "192.168.1.1");
    EXPECT_EQ(wired->macAddress, "02:00:00:00:00:01");
}

TEST_F(ClientTest, HiddenAccessPointJoinsOnceNamed)
{
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));
    nm->set(hidden, fakenm::accessPoint, "Ssid", fakenm::bytes("Garden"));
    ASSERT_TRUE(waitFor([&] { return row(model, "Garden").has_value(); }));
    EXPECT_EQ(model.items().size(), 6u);
}

TEST_F(ClientTest, RowStaysWhileItsAccessPointsChange)
{
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));
    const std::string key = row(model, "Home")->key;

    nm->set(wifiDevice, fakenm::wireless, "AccessPoints", fakenm::paths({homeWeak, cafe, hidden, library, office}));
    ASSERT_TRUE(waitFor([&] { return row(model, "Home")->specificPath == homeWeak; }));
    EXPECT_EQ(row(model, "Home")->key, key);
    EXPECT_EQ(row(model, "Home")->signal, 40);
}

TEST_F(ClientTest, AgedOutNetworkStaysUntilScansMissIt)
{
    if (bootTimeMs() < kOldScanMs)
        GTEST_SKIP() << "booted less than a minute ago";
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));
    scannedAgo(kOldScanMs);

    dropAccessPoint(cafe);
    ASSERT_TRUE(waitFor([] { return Client::instance()->remembers(cafe); }));
    wait(100ms);
    ASSERT_TRUE(row(model, "Cafe"));
    EXPECT_EQ(row(model, "Cafe")->specificPath, cafe);
    EXPECT_EQ(row(model, "Cafe")->signal, 50);

    // NetworkManager drops an access point that two scans missed; so does the list.
    scannedAgo(0);
    wait(100ms);
    EXPECT_TRUE(row(model, "Cafe"));
    scannedAgo(0, 1);
    EXPECT_TRUE(waitFor([&] { return !row(model, "Cafe"); }));
    EXPECT_FALSE(Client::instance()->remembers(cafe));
}

TEST_F(ClientTest, RememberedNetworkIsJoinedOnceScanned)
{
    if (bootTimeMs() < kOldScanMs)
        GTEST_SKIP() << "booted less than a minute ago";
    Handler handler;
    scannedAgo(kOldScanMs);
    const fakenm::Interfaces cafeAgain = {{fakenm::accessPoint, nm->objects[cafe][fakenm::accessPoint]}};
    dropAccessPoint(cafe);
    ASSERT_TRUE(waitFor([] { return Client::instance()->remembers(cafe); }));

    handler.addAndActivateConnection(wifiDevice, cafe);
    ASSERT_TRUE(waitFor([&] { return nm->count("RequestScan") == 1; }));
    wait(100ms);
    EXPECT_EQ(nm->count("AddAndActivateConnection"), 0);

    const std::string found = fakenm::managerPath + "/AccessPoint/7";
    nm->add(found, cafeAgain);
    nm->set(wifiDevice, fakenm::wireless, "AccessPoints", fakenm::paths({homeStrong, homeWeak, hidden, library, office, found}));
    scannedAgo(0);
    ASSERT_TRUE(waitFor([&] { return nm->count("AddAndActivateConnection") == 1; }));
    EXPECT_EQ(pathArgument(nm->last("AddAndActivateConnection"), 2), found);
    EXPECT_FALSE(Client::instance()->remembers(cafe));
}

TEST_F(ClientTest, UnreadableProfileStillShowsItsNetworkInUse)
{
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));

    const std::string theirs = fakenm::managerPath + "/Settings/50";
    const std::string activePath = fakenm::managerPath + "/ActiveConnection/50";
    nm->privateConnections.insert(theirs);
    nm->add(theirs, {{fakenm::connection, {{"Flags", Variant(uint32_t(0))}}}});
    nm->add(activePath, {{fakenm::active,
                          {{"Connection", fakenm::path(theirs)},
                           {"SpecificObject", fakenm::path(cafe)},
                           {"Devices", fakenm::paths({wifiDevice})},
                           {"State", Variant(uint32_t(2))},
                           {"Type", Variant(std::string("802-11-wireless"))},
                           {"Vpn", Variant(false)}}}});
    nm->set(wifiDevice, fakenm::wireless, "ActiveAccessPoint", fakenm::path(cafe));
    nm->set(wifiDevice, fakenm::device, "ActiveConnection", fakenm::path(activePath));

    ASSERT_TRUE(waitFor([&] { return row(model, "Cafe")->connectionState == ConnectionStatus::Activated; }));
    EXPECT_TRUE(row(model, "Cafe")->connectionPath.empty());
    EXPECT_FALSE(Client::instance()->readable(theirs));
}

TEST_F(ClientTest, JoiningSendsATypedPartialProfile)
{
    Handler handler;
    handler.addAndActivateConnection(wifiDevice, office, "secret pass");
    ASSERT_TRUE(waitFor([&] { return nm->count("AddAndActivateConnection") == 1; }));

    const fakenm::Call& call = nm->last("AddAndActivateConnection");
    EXPECT_EQ(call.signature, "a{sa{sv}}oo");
    EXPECT_EQ(pathArgument(call, 2), office);
    const Settings settings = settingsOf(call);

    // NetworkManager fills in the SSID's bytes and the key management from the access point.
    EXPECT_FALSE(settings.count("802-11-wireless"));
    const VariantMap& security = settings.at("802-11-wireless-security");
    EXPECT_FALSE(contains(security, "key-mgmt"));
    EXPECT_EQ(toString(value(security, "psk")), "secret pass");
    EXPECT_EQ(value(security, "psk-flags").signature(), "u");
    EXPECT_EQ(toUInt(value(security, "psk-flags")), 1u);
    EXPECT_EQ(toString(value(settings.at("connection"), "id")), "Office");
    EXPECT_EQ(toStringList(value(settings.at("connection"), "permissions")),
              std::vector<std::string>{Client::userPermission()});
}

TEST_F(ClientTest, FailedJoinIsForgotten)
{
    Handler handler;
    handler.addAndActivateConnection(wifiDevice, office, "wrong pass");
    ASSERT_TRUE(waitFor([&] { return nm->count("AddAndActivateConnection") == 1; }));
    ASSERT_TRUE(waitFor([] { return Client::instance()->activeConnections().size() == 2; }));
    const std::string activePath = otherActive();
    ASSERT_FALSE(activePath.empty());

    // NM_ACTIVE_CONNECTION_STATE_DEACTIVATED for NO_SECRETS.
    nm->emitSignal(activePath, fakenm::active, "StateChanged", {Variant(uint32_t(4)), Variant(uint32_t(9))});
    ASSERT_TRUE(waitFor([&] { return nm->count("Delete") == 1; }));
    EXPECT_TRUE(nm->last("Delete").path.starts_with(fakenm::managerPath + "/Settings/"));
}

TEST_F(ClientTest, EnterpriseJoinSendsOnlyTheSignIn)
{
    Handler handler;
    const auto network = handler.wirelessNetwork(library);
    ASSERT_TRUE(network);
    EXPECT_EQ(network->device, wifiDevice);
    EXPECT_EQ(network->name, "Library");
    EXPECT_EQ(network->security, SecurityType::Wpa2Eap);
    EXPECT_FALSE(handler.wirelessNetwork(fakenm::managerPath + "/AccessPoint/99"));

    handler.addAndActivateEnterprise(wifiDevice, library, "alice", "secret");
    ASSERT_TRUE(waitFor([&] { return nm->count("AddAndActivateConnection") == 1; }));

    const fakenm::Call& call = nm->last("AddAndActivateConnection");
    EXPECT_EQ(pathArgument(call, 2), library);
    const Settings settings = settingsOf(call);
    // Not a hidden network: NetworkManager fills in the SSID and key management.
    EXPECT_FALSE(settings.count("802-11-wireless"));
    EXPECT_FALSE(settings.count("802-11-wireless-security"));
    const VariantMap& eap = settings.at("802-1x");
    EXPECT_EQ(toStringList(value(eap, "eap")), (std::vector<std::string>{"peap", "ttls"}));
    EXPECT_EQ(toString(value(eap, "phase2-auth")), "mschapv2");
    EXPECT_EQ(toString(value(eap, "identity")), "alice");
    EXPECT_EQ(toString(value(eap, "password")), "secret");
    EXPECT_EQ(value(eap, "password-flags").signature(), "u");
    EXPECT_EQ(toUInt(value(eap, "password-flags")), 1u);

    // NetworkManager's completion took it, and made it a WPA-EAP profile for that network.
    ASSERT_TRUE(waitFor([] { return Client::instance()->activeConnections().size() == 2; }));
    const std::string profile = objectPath(nm->objects[otherActive()][fakenm::active]["Connection"]);
    EXPECT_EQ(toBytes(value(nm->settings[profile]["802-11-wireless"], "ssid")), "Library");
    EXPECT_EQ(toString(value(nm->settings[profile]["802-11-wireless-security"], "key-mgmt")), "wpa-eap");

    // A personal network is not joined this way, nor an enterprise one without a user name.
    handler.addAndActivateEnterprise(wifiDevice, office, "alice", "secret");
    handler.addAndActivateConnection(wifiDevice, library);
    wait(100ms);
    EXPECT_EQ(nm->count("AddAndActivateConnection"), 1);
}

TEST_F(ClientTest, HiddenNetworkIsSpeltOut)
{
    Handler handler;
    EXPECT_TRUE(handler.addOtherConnection("Garden", {}, "garden pass", "wpa2-wpa3"));
    ASSERT_TRUE(waitFor([&] { return nm->count("AddAndActivateConnection") == 1; }));
    // A hidden network answers only probes that name it.
    ASSERT_EQ(nm->count("RequestScan"), 1);
    const fakenm::Call& scan = nm->last("RequestScan");
    EXPECT_EQ(scan.signature, "a{sv}");
    const VariantMap options = scan.arguments[0].get<VariantMap>().value_or(VariantMap());
    EXPECT_EQ(toBytesList(value(options, "ssids")), std::vector<std::string>{"Garden"});

    const Settings settings = settingsOf(nm->last("AddAndActivateConnection"));
    EXPECT_EQ(toBytes(value(settings.at("802-11-wireless"), "ssid")), "Garden");
    EXPECT_TRUE(toBool(value(settings.at("802-11-wireless"), "hidden")));
    EXPECT_EQ(toString(value(settings.at("802-11-wireless-security"), "key-mgmt")), "wpa-psk");
    EXPECT_EQ(toStringList(value(settings.at("802-11-wireless-security"), "proto")), std::vector<std::string>{"rsn"});
    EXPECT_FALSE(handler.addOtherConnection(std::string(33, 'x'), {}, {}, "none"));
}

TEST_F(ClientTest, UpdateKeepsWhatItDoesNotChange)
{
    ConnectionSettings item(homeProfile);
    EXPECT_TRUE(item.autoConnect());
    EXPECT_FALSE(item.availableToOtherUsers());

    item.setAutoConnect(false);
    // An unrelated change arriving before the profile is re-read must not put
    // the switch back to what the stale cache says.
    Client::instance()->changed.emit();
    EXPECT_FALSE(item.autoConnect());
    ASSERT_TRUE(waitFor([&] { return nm->count("Update") == 1; }));

    const Settings settings = settingsOf(nm->last("Update"));
    EXPECT_FALSE(toBool(value(settings.at("connection"), "autoconnect"), true));
    EXPECT_EQ(toBytes(value(settings.at("802-11-wireless"), "ssid")), "Home");
    EXPECT_EQ(value(settings.at("802-11-wireless-security"), "psk-flags").signature(), "u");
    // A value NetworkManager sent in a container type goes back in that type.
    EXPECT_EQ(value(settings.at("ipv4"), "address-data").signature(), "aa{sv}");

    // The profile NetworkManager reports after the update is what the switch shows.
    wait(100ms);
    EXPECT_FALSE(item.autoConnect());
}

TEST_F(ClientTest, IpSettingsAreSavedAndReapplied)
{
    Handler handler;
    std::vector<std::string> saved;
    auto subscription = handler.ipSaved.connect([&](const std::string& connection) { saved.push_back(connection); });

    const ip::Form home = handler.ipForm(homeProfile);
    EXPECT_EQ(home.ipv4Method, "manual");
    EXPECT_EQ(home.ipv4Address, "10.0.0.2");
    EXPECT_EQ(home.ipv4Netmask, "255.255.255.0");
    EXPECT_TRUE(handler.currentAddresses(homeProfile).ipv4Address.empty());

    // The wired profile is up; its form offers the addresses it has now.
    ip::Form form = handler.ipForm(wiredProfile);
    const ip::Form current = handler.currentAddresses(wiredProfile);
    EXPECT_EQ(current.ipv4Address, "192.168.1.5");
    EXPECT_EQ(current.ipv4Netmask, "255.255.255.0");
    EXPECT_EQ(current.ipv4Gateway, "192.168.1.1");

    form.ipv4Method = "manual";
    form.ipv4Address = current.ipv4Address;
    form.ipv4Netmask = current.ipv4Netmask;
    form.ipv4Gateway = current.ipv4Gateway;
    form.automaticDns = false;
    form.dns = "1.1.1.1";
    handler.saveIp(wiredProfile, form);

    ASSERT_TRUE(waitFor([&] { return saved.size() == 1; }));
    EXPECT_EQ(saved[0], wiredProfile);
    EXPECT_EQ(nm->last("Update").path, wiredProfile);
    const VariantMap ipv4 = settingsOf(nm->last("Update")).at("ipv4");
    EXPECT_EQ(toString(value(ipv4, "method")), "manual");
    EXPECT_EQ(toString(value(ipv4, "gateway")), "192.168.1.1");
    EXPECT_EQ(value(ipv4, "address-data").signature(), "aa{sv}");
    EXPECT_EQ(value(ipv4, "dns").signature(), "au");
    EXPECT_TRUE(toBool(value(ipv4, "ignore-auto-dns")));

    // The device it is up on takes the change without disconnecting.
    ASSERT_TRUE(waitFor([&] { return nm->count("Reapply") == 1; }));
    EXPECT_EQ(nm->last("Reapply").path, wiredDevice);
    EXPECT_EQ(nm->last("Reapply").signature, "a{sa{sv}}tu");
    EXPECT_EQ(nm->count("ActivateConnection"), 0);

    // A profile that isn't up is only saved.
    form = handler.ipForm(homeProfile);
    form.ipv4Address = "10.0.0.3";
    handler.saveIp(homeProfile, form);
    ASSERT_TRUE(waitFor([&] { return saved.size() == 2; }));
    EXPECT_EQ(nm->count("Update"), 2);
    wait(50ms);
    EXPECT_EQ(nm->count("Reapply"), 1);

    // Saving what is there changes nothing.
    ASSERT_TRUE(waitFor([&] { return handler.ipForm(homeProfile).ipv4Address == "10.0.0.3"; }));
    handler.saveIp(homeProfile, handler.ipForm(homeProfile));
    EXPECT_EQ(saved.size(), 3u);
    EXPECT_EQ(nm->count("Update"), 2);
}

TEST_F(ClientTest, ScansOnlyDevicesThatCan)
{
    Handler handler;
    handler.requestScan();
    ASSERT_TRUE(waitFor([&] { return nm->count("RequestScan") == 1; }));
    EXPECT_EQ(nm->last("RequestScan").path, wifiDevice);

    nm->set(wifiDevice, fakenm::device, "State", Variant(uint32_t(20)));
    ASSERT_TRUE(waitFor([] { return Client::instance()->deviceState(wifiDevice) == 20u; }));
    handler.requestScan();
    wait(200ms);
    EXPECT_EQ(nm->count("RequestScan"), 1);
}

TEST_F(ClientTest, SwitchesGoToNetworkManager)
{
    EnabledConnections enabled;
    EXPECT_TRUE(enabled.isReady());
    EXPECT_TRUE(enabled.isWirelessEnabled());
    EXPECT_TRUE(enabled.isWiredAvailable());
    EXPECT_TRUE(enabled.isWiredEnabled());

    Handler handler;
    handler.enableWireless(false);
    ASSERT_TRUE(waitFor([&] { return !enabled.isWirelessEnabled(); }));
    // The wired device is up: turning Ethernet off disconnects it.
    handler.enableWired(false);
    ASSERT_TRUE(waitFor([&] { return nm->count("Disconnect") == 1; }));
    EXPECT_EQ(nm->last("Disconnect").path, wiredDevice);
}

TEST_F(ClientTest, HotspotKeepsOneProfile)
{
    Handler handler;
    ASSERT_TRUE(waitFor([&] { return handler.hotspotCapable(); }));
    EXPECT_TRUE(handler.hotspotSupported());
    EXPECT_TRUE(handler.hotspotReplaces().empty());
    EXPECT_FALSE(handler.hotspotActive());

    HotspotConfiguration* configuration = HotspotConfiguration::instance();
    configuration->setName("Desk");
    configuration->setPassword("good password");
    handler.createHotspot();
    ASSERT_TRUE(waitFor([&] { return nm->count("AddAndActivateConnection") == 1; }));

    const fakenm::Call& call = nm->last("AddAndActivateConnection");
    EXPECT_EQ(pathArgument(call, 1), wifiDevice);
    const Settings settings = settingsOf(call);
    EXPECT_EQ(toString(value(settings.at("802-11-wireless"), "mode")), "ap");
    EXPECT_EQ(toBytes(value(settings.at("802-11-wireless"), "ssid")), "Desk");
    EXPECT_EQ(toString(value(settings.at("802-11-wireless-security"), "psk")), "good password");
    EXPECT_EQ(toString(value(settings.at("ipv4"), "method")), "shared");
    ASSERT_TRUE(waitFor([&] { return handler.hotspotActive(); }));

    // The next start updates that profile rather than adding another.
    nm->remove(otherActive());
    ASSERT_TRUE(waitFor([&] { return !handler.hotspotActive(); }));
    configuration->setName("Desk 2");
    handler.createHotspot();
    ASSERT_TRUE(waitFor([&] { return nm->count("ActivateConnection") == 1; }));
    EXPECT_EQ(nm->count("AddAndActivateConnection"), 1);
    EXPECT_EQ(toBytes(value(settingsOf(nm->last("Update")).at("802-11-wireless"), "ssid")), "Desk 2");
    EXPECT_EQ(pathArgument(nm->last("ActivateConnection"), 0), nm->last("Update").path);
    EXPECT_TRUE(waitFor([&] { return handler.hotspotActive(); }));
}

TEST_F(ClientTest, HotspotThatNeverComesUpIsReported)
{
    Handler handler;
    int failures = 0;
    auto subscription = handler.hotspotFailed.connect([&](const std::string&) { ++failures; });
    handler.createHotspot();
    ASSERT_TRUE(waitFor([] { return Client::instance()->activeConnections().size() == 2; }));
    const std::string activePath = otherActive();

    // NM_ACTIVE_CONNECTION_STATE_DEACTIVATED for DEVICE_DISCONNECTED.
    wait(100ms);
    nm->emitSignal(activePath, fakenm::active, "StateChanged", {Variant(uint32_t(4)), Variant(uint32_t(3))});
    EXPECT_TRUE(waitFor([&] { return failures == 1; }));
}

TEST_F(ClientTest, HotspotGoneBeforeReplyIsReported)
{
    Handler handler;
    ASSERT_TRUE(waitFor([&] { return handler.hotspotCapable(); }));
    int failures = 0;
    auto subscription = handler.hotspotFailed.connect([&](const std::string&) { ++failures; });
    nm->dropActivations = true;
    handler.createHotspot();
    EXPECT_TRUE(waitFor([&] { return failures == 1; }));
    EXPECT_FALSE(handler.hotspotActive());
}

TEST_F(ClientTest, HotspotNamesTheNetworkItWouldEnd)
{
    Handler handler;
    ASSERT_TRUE(waitFor([&] { return handler.hotspotCapable(); }));

    nm->add(homeActive, {{fakenm::active,
                          {{"Connection", fakenm::path(homeProfile)},
                           {"Id", Variant(std::string("Home"))},
                           {"Devices", fakenm::paths({wifiDevice})},
                           {"State", Variant(uint32_t(2))},
                           {"Type", Variant(std::string("802-11-wireless"))},
                           {"Vpn", Variant(false)}}}});
    nm->set(wifiDevice, fakenm::device, "ActiveConnection", fakenm::path(homeActive));
    ASSERT_TRUE(waitFor([&] { return handler.hotspotReplaces() == "Home"; }));
    EXPECT_TRUE(handler.hotspotSupported());

    nm->permissions["org.freedesktop.NetworkManager.wifi.share.protected"] = "no";
    nm->emitSignal(fakenm::managerPath, fakenm::manager, "CheckPermissions", {});
    ASSERT_TRUE(waitFor([&] { return !handler.hotspotAllowed(); }));
    EXPECT_FALSE(handler.hotspotSupported());
    EXPECT_TRUE(handler.hotspotCapable());
}

TEST_F(ClientTest, VpnRowsFollowTheirState)
{
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));
    addVpn(officeVpn, "Office VPN", "org.freedesktop.NetworkManager.openvpn", 3000);
    addVpn(branchVpn, "Branch", "org.freedesktop.NetworkManager.l2tp", 2000);
    addWireGuard(homeVpn, "Home WG", 1000);
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 8; }));

    const auto officeRow = row(model, "Office VPN");
    EXPECT_EQ(officeRow->type, ConnectionType::Vpn);
    EXPECT_EQ(officeRow->vpnType, "OpenVPN");
    EXPECT_TRUE(officeRow->canActivate);
    EXPECT_EQ(officeRow->connectionState, ConnectionStatus::Deactivated);

    // No l2tp plugin is installed in this test.
    const auto branchRow = row(model, "Branch");
    EXPECT_FALSE(branchRow->canActivate);
    EXPECT_NE(branchRow->statusMessage.find("network-manager-l2tp"), std::string::npos);
    EXPECT_EQ(row(model, "Home WG")->vpnType, "WireGuard");

    // The tunnel is up when the plugin says so, not when the active connection exists.
    const std::string activePath = fakenm::managerPath + "/ActiveConnection/60";
    nm->add(activePath, {{fakenm::active,
                          {{"Connection", fakenm::path(officeVpn)},
                           {"Devices", fakenm::paths({})},
                           {"State", Variant(uint32_t(2))},
                           {"Type", Variant(std::string("vpn"))},
                           {"Vpn", Variant(true)}}},
                         {vpnInterface, {{"VpnState", Variant(uint32_t(3))}}}});
    ASSERT_TRUE(waitFor([&] { return row(model, "Office VPN")->connectionState == ConnectionStatus::Activating; }));
    nm->set(activePath, vpnInterface, "VpnState", Variant(uint32_t(5)));
    ASSERT_TRUE(waitFor([&] { return row(model, "Office VPN")->connectionState == ConnectionStatus::Activated; }));

    // NM_ACTIVE_CONNECTION_STATE_REASON_LOGIN_FAILED stays on the row until the next try.
    nm->emitSignal(activePath, fakenm::active, "StateChanged", {Variant(uint32_t(4)), Variant(uint32_t(10))});
    nm->remove(activePath);
    ASSERT_TRUE(waitFor([&] { return !row(model, "Office VPN")->statusMessage.empty(); }));
    EXPECT_EQ(row(model, "Office VPN")->connectionState, ConnectionStatus::Deactivated);
}

TEST_F(ClientTest, VpnFailureIsReported)
{
    Handler handler;
    std::vector<std::pair<std::string, std::string>> failures;
    auto subscription = handler.vpnFailed.connect(
        [&](const std::string& name, const std::string& message) { failures.emplace_back(name, message); });
    addVpn(officeVpn, "Office VPN", "org.freedesktop.NetworkManager.openvpn", 1000);
    const std::string activePath = fakenm::managerPath + "/ActiveConnection/61";
    nm->add(activePath, {{fakenm::active,
                          {{"Connection", fakenm::path(officeVpn)},
                           {"Devices", fakenm::paths({})},
                           {"Id", Variant(std::string("Office VPN"))},
                           {"State", Variant(uint32_t(1))},
                           {"Type", Variant(std::string("vpn"))},
                           {"Vpn", Variant(true)}}},
                         {vpnInterface, {{"VpnState", Variant(uint32_t(3))}}}});
    ASSERT_TRUE(waitFor([&] {
        const auto list = Client::instance()->activeConnections();
        return std::find(list.begin(), list.end(), activePath) != list.end();
    }));

    // NO_SECRETS (the prompt was cancelled) and USER_DISCONNECTED are not failures.
    for (const uint32_t reason : {9u, 2u, 10u})
        nm->emitSignal(activePath, fakenm::active, "StateChanged", {Variant(uint32_t(4)), Variant(reason)});
    ASSERT_TRUE(waitFor([&] { return failures.size() == 1; }));
    EXPECT_EQ(failures[0].first, "Office VPN");
    EXPECT_EQ(failures[0].second, vpn::failureMessage(10));
}

TEST_F(ClientTest, VpnFormIsSavedInTheProfile)
{
    NetworkModel model;
    addVpn(officeVpn, "Office VPN", "org.freedesktop.NetworkManager.openvpn", 1000);
    ASSERT_TRUE(waitFor([&] { return row(model, "Office VPN").has_value(); }));
    EXPECT_TRUE(row(model, "Office VPN")->vpnUsesUserName);
    EXPECT_TRUE(row(model, "Office VPN")->vpnUserName.empty());

    Handler handler;
    const VpnForm form = handler.vpnForm({}, officeVpn);
    EXPECT_EQ(form.kind, "openvpn");
    EXPECT_EQ(form.name, "Office VPN");
    EXPECT_TRUE(form.shared);
    EXPECT_FALSE(form.ownNetworkOnly);
    EXPECT_TRUE(form.exportable);
    vpn::schema::Values values;
    for (const vpn::schema::FormField& field : form.fields)
        values[field.key] = field.type == vpn::schema::Type::Bool ? (field.checked ? "true" : "false") : field.value;
    EXPECT_EQ(values["connection-type"], "password");

    std::vector<std::string> saved;
    auto subscription = handler.vpnSaved.connect([&](const std::string& connection) { saved.push_back(connection); });
    values["remote"] = "vpn.example.com";
    values["username"] = "alice";
    VpnValues edit;
    edit.name = "Office HQ";
    edit.shared = false;
    edit.ownNetworkOnly = true;
    edit.values = values;
    edit.storage = {{"password", SecretSystemOwned}};
    edit.secrets = {{"password", "s3cret"}};
    handler.saveVpn(officeVpn, edit);
    ASSERT_TRUE(waitFor([&] { return saved.size() == 1; }));
    EXPECT_EQ(nm->count("Update"), 1);
    const Settings settings = settingsOf(nm->last("Update"));
    const VariantMap& connection = settings.at("connection");
    EXPECT_EQ(toString(value(connection, "id")), "Office HQ");
    EXPECT_EQ(toString(value(connection, "uuid")), "office vpn");
    EXPECT_EQ(toStringList(value(connection, "permissions")), std::vector<std::string>{Client::userPermission()});
    EXPECT_TRUE(toBool(value(settings.at("ipv4"), "never-default")));
    const Variant& data = value(settings.at("vpn"), "data");
    EXPECT_EQ(data.signature(), "a{ss}");
    const auto dataMap = toStringMap(data);
    EXPECT_EQ(dataMap.at("username"), "alice");
    // Private now, so the password is the agent's rather than the system's.
    EXPECT_EQ(dataMap.at("password-flags"), "1");
    EXPECT_EQ(toStringMap(value(settings.at("vpn"), "secrets")).at("password"), "s3cret");
    EXPECT_TRUE(waitFor([&] { return row(model, "Office HQ") && row(model, "Office HQ")->vpnUserName == "alice"; }));

    std::vector<std::string> failed;
    auto failure = handler.vpnSaveFailed.connect([&](const std::string& message) { failed.push_back(message); });
    values["remote"] = "";
    VpnValues broken;
    broken.values = values;
    handler.saveVpn(officeVpn, broken);
    EXPECT_EQ(failed.size(), 1u);
    EXPECT_EQ(nm->count("Update"), 1);
}

TEST_F(ClientTest, WireGuardReadsBackFromDBus)
{
    const std::string path = fakenm::managerPath + "/Settings/33";
    nm->settings[path] = {
        {"connection",
         {{"id", Variant(std::string("Work WG"))},
          {"uuid", Variant(std::string("work wg"))},
          {"type", Variant(std::string("wireguard"))},
          {"interface-name", Variant(std::string("wg0"))}}},
        {"wireguard",
         {{"private-key-flags", Variant(uint32_t(SecretSystemOwned))},
          {"mtu", Variant(uint32_t(1280))},
          {"peers", Variant(std::vector<VariantMap>{{
                        {"public-key", Variant(std::string("xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg="))},
                        {"preshared-key-flags", Variant(uint32_t(SecretSystemOwned))},
                        {"endpoint", Variant(std::string("vpn.example:51820"))},
                        {"allowed-ips", Variant(std::vector<std::string>{"0.0.0.0/0"})},
                        {"persistent-keepalive", Variant(uint32_t(25))},
                    }})}}},
        {"ipv4",
         {{"method", Variant(std::string("manual"))},
          {"address-data", addressData("10.0.0.2")},
          {"dns", Variant(std::vector<uint32_t>{htobe32(0x0a000001u)})},
          {"dns-search", Variant(std::vector<std::string>{"corp.example"})}}},
        {"ipv6",
         {{"method", Variant(std::string("manual"))},
          {"address-data",
           Variant(std::vector<VariantMap>{{{"address", Variant(std::string("fd00::2"))}, {"prefix", Variant(uint32_t(64))}}})},
          {"dns", Variant(std::vector<std::vector<uint8_t>>{{0xfd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}})}}},
    };
    nm->add(path, {{fakenm::connection, {{"Flags", Variant(uint32_t(0))}}}});
    ASSERT_TRUE(waitFor([&] { return !Client::instance()->settings(path).empty(); }));
    const Settings settings = Client::instance()->settings(path);

    const vpn::WireGuardForm form = vpn::wireGuardForm(settings);
    EXPECT_EQ(form.addresses, "10.0.0.2/24, fd00::2/64");
    EXPECT_EQ(form.dns, "10.0.0.1, corp.example, fd00::1");
    EXPECT_EQ(form.mtu, "1280");
    ASSERT_EQ(form.peers.size(), 1u);
    EXPECT_EQ(form.peers[0].publicKey, "xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=");
    EXPECT_EQ(form.peers[0].endpoint, "vpn.example:51820");
    EXPECT_EQ(form.peers[0].allowedIps, "0.0.0.0/0");
    EXPECT_EQ(form.peers[0].keepalive, "25");
    EXPECT_TRUE(form.peers[0].hasPresharedKey);

    // Saved again with the keys left empty, the retained preshared key stays.
    std::string error;
    const auto edited = vpn::wireGuardSettings(form, settings, &error);
    ASSERT_TRUE(edited) << error;
    const auto peers = toMapList(value(edited->at("wireguard"), "peers"));
    ASSERT_EQ(peers.size(), 1u);
    EXPECT_EQ(toUInt(value(peers[0], "preshared-key-flags"), 99), uint32_t(SecretSystemOwned));

    const std::string config = vpn::wireGuardConfig(settings);
    EXPECT_NE(config.find("Address = 10.0.0.2/24, fd00::2/64"), std::string::npos);
    EXPECT_NE(config.find("[Peer]\nPublicKey = xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg="), std::string::npos);
    EXPECT_NE(config.find("AllowedIPs = 0.0.0.0/0"), std::string::npos);
}

TEST_F(ClientTest, VpnAddedByHandIsPrivate)
{
    Handler handler;
    std::vector<std::string> added;
    auto subscription = handler.vpnAdded.connect([&](const std::string& uuid) { added.push_back(uuid); });
    VpnValues branch;
    branch.kind = "l2tp";
    branch.name = "Branch";
    branch.values = {{"gateway", "vpn.example.com"}, {"user", "alice"}, {"ipsec-enabled", "true"}};
    branch.secrets = {{"password", "s3cret"}, {"ipsec-psk", "shared"}};
    handler.saveVpn({}, branch);
    ASSERT_TRUE(waitFor([&] { return added.size() == 1; }));

    const Settings settings = settingsOf(nm->last("AddConnection"));
    const VariantMap& connection = settings.at("connection");
    EXPECT_EQ(toString(value(connection, "id")), "Branch");
    EXPECT_EQ(toString(value(connection, "type")), "vpn");
    EXPECT_FALSE(toBool(value(connection, "autoconnect"), true));
    EXPECT_EQ(toStringList(value(connection, "permissions")), std::vector<std::string>{Client::userPermission()});

    const VariantMap& vpnSetting = settings.at("vpn");
    EXPECT_EQ(toString(value(vpnSetting, "service-type")), "org.freedesktop.NetworkManager.l2tp");
    EXPECT_EQ(value(vpnSetting, "data").signature(), "a{ss}");
    EXPECT_EQ(value(vpnSetting, "secrets").signature(), "a{ss}");
    const auto data = toStringMap(value(vpnSetting, "data"));
    const auto secrets = toStringMap(value(vpnSetting, "secrets"));
    EXPECT_EQ(data.at("user"), "alice");
    EXPECT_EQ(data.at("password-flags"), "1");
    EXPECT_EQ(data.at("ipsec-enabled"), "yes");
    EXPECT_EQ(secrets.at("ipsec-psk"), "shared");
    EXPECT_EQ(toString(value(settings.at("ipv4"), "method")), "auto");

    // Unnamed, it goes by its server; shared, it belongs to everyone.
    VpnValues pptp;
    pptp.kind = "pptp";
    pptp.shared = true;
    pptp.values = {{"gateway", "pptp.example.com"}};
    handler.saveVpn({}, pptp);
    ASSERT_TRUE(waitFor([&] { return added.size() == 2; }));
    const Settings second = settingsOf(nm->last("AddConnection"));
    EXPECT_EQ(toString(value(second.at("connection"), "id")), "pptp.example.com");
    EXPECT_TRUE(toStringList(value(second.at("connection"), "permissions")).empty());
    EXPECT_EQ(toStringMap(value(second.at("vpn"), "data")).at("password-flags"), "0");

    std::vector<std::string> failed;
    auto failure = handler.vpnAddFailed.connect([&](const std::string& message) { failed.push_back(message); });
    VpnValues openvpn;
    openvpn.kind = "openvpn";
    handler.saveVpn({}, openvpn);
    EXPECT_EQ(failed.size(), 1u);
}

TEST_F(ClientTest, WireGuardImportsAsPrivateAndManual)
{
    char pattern[] = "/tmp/wireguard-import-XXXXXX";
    const std::string directory = mkdtemp(pattern);
    const std::string file = directory + "/Home Office.conf";
    {
        std::ofstream out(file);
        out << "[Interface]\nPrivateKey = yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=\n"
               "Address = 10.99.0.2/24\nDNS = 10.99.0.1\n\n"
               "[Peer]\nPublicKey = xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=\n"
               "AllowedIPs = 10.99.0.0/24\nEndpoint = 192.0.2.1:51820\n";
    }

    Handler handler;
    std::vector<std::string> imported;
    auto subscription = handler.vpnAdded.connect([&](const std::string& uuid) { imported.push_back(uuid); });
    handler.importVpn(file);
    ASSERT_TRUE(waitFor([&] { return imported.size() == 1; }));
    EXPECT_EQ(nm->count("AddConnection"), 1);

    const fakenm::Call& call = nm->last("AddConnection");
    EXPECT_EQ(call.signature, "a{sa{sv}}");
    const Settings settings = settingsOf(call);
    const VariantMap& connection = settings.at("connection");
    EXPECT_EQ(toString(value(connection, "id")), "Home Office");
    EXPECT_FALSE(toBool(value(connection, "autoconnect"), true));
    EXPECT_EQ(toStringList(value(connection, "permissions")), std::vector<std::string>{Client::userPermission()});
    EXPECT_EQ(toString(value(connection, "uuid")), imported[0]);
    EXPECT_EQ(value(settings.at("wireguard"), "peers").signature(), "aa{sv}");
    EXPECT_EQ(value(settings.at("ipv4"), "address-data").signature(), "aa{sv}");
    EXPECT_EQ(value(settings.at("ipv4"), "dns").signature(), "au");
    std::filesystem::remove_all(directory);
}

TEST_F(ClientTest, ReloadsAfterRestart)
{
    NetworkModel model;
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));

    nm->stop();
    ASSERT_TRUE(waitFor([] { return !Client::instance()->ready(); }));
    ASSERT_TRUE(waitFor([&] { return model.items().empty(); }));

    ASSERT_TRUE(nm->serve());
    ASSERT_TRUE(waitFor([] { return Client::instance()->ready(); }));
    ASSERT_TRUE(waitFor([&] { return model.items().size() == 5; }));
    EXPECT_EQ(nm->count("GetManagedObjects"), 2);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    EventLoop eventLoop;
    loop = &eventLoop;
    // Only OpenVPN's plugin is installed.
    char pattern[] = "/tmp/vpn-plugins-XXXXXX";
    const std::string plugins = mkdtemp(pattern);
    {
        std::ofstream name(plugins + "/nm-openvpn-service.name");
        name << "[VPN Connection]\nname=openvpn\nservice=org.freedesktop.NetworkManager.openvpn\n";
    }
    vpn::setPluginDirectories({plugins});
    const int result = RUN_ALL_TESTS();
    std::filesystem::remove_all(plugins);
    return result;
}
