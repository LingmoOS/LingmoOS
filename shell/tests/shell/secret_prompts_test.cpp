// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The session's secret agents and their prompts on a private bus, which
// stands in for the system bus too: a fake NetworkManager, Secret Service and
// BlueZ, each on a connection of its own.

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <systemd/sd-bus.h>

#include "base/dbus.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "lingmo/bluez/manager.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/variants.h"
#include "dbus_test_client.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/secrets/bluetooth_pairing_dialog.h"
#include "shell/secrets/network_dialogs.h"
#include "shell/secrets/network_secret_agent.h"
#include "shell/secrets/secret_prompts.h"
#include "shell/secrets/secret_store.h"
#include "shell/secrets/vpn_secret_fields.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "shell_test_util.h"
#include "temporary_directory.h"
#include "ui/application.h"
#include "ui/check_box.h"
#include "ui/dialog_window.h"
#include "ui/embedded_window.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/text_field.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::ObjectPath;
using base::dbus::Variant;
using base::dbus::VariantMap;
using networkmanager::Settings;

namespace {

const std::string kAgentPath = "/org/freedesktop/NetworkManager/SecretAgent";
const std::string kAgentInterface = "org.freedesktop.NetworkManager.SecretAgent";
const std::string kConnectionPath = "/org/freedesktop/NetworkManager/Settings/7";
const std::string kUuid = "5e3c1b64-3b43-4a2c-9d53-0d6c1a1f4f10";
const std::string kWirelessSecurity = "802-11-wireless-security";
const std::string kDevice = "/org/freedesktop/NetworkManager/Devices/2";
const std::string kAccessPoint = "/org/freedesktop/NetworkManager/AccessPoint/5";
const std::string kOpenVpn = "org.freedesktop.NetworkManager.openvpn";

constexpr uint32_t kAllowInteraction = 0x1;
constexpr uint32_t kRequestNew = 0x2;
constexpr uint32_t kNoErrors = 0x40000000;

const base::Rect kOutput(0, 0, 1440, 960);

Settings wpaConnection(uint32_t pskFlags = 0)
{
    return {{"connection", {{"id", Variant(std::string("Cafe"))}, {"uuid", Variant(kUuid)}}},
            {kWirelessSecurity, {{"key-mgmt", Variant(std::string("wpa-psk"))}, {"psk-flags", Variant(pskFlags)}}}};
}

Settings vpnConnection(const std::map<std::string, std::string> &data)
{
    return {{"connection", {{"id", Variant(std::string("Office VPN"))}, {"uuid", Variant(kUuid)}}},
            {"vpn", {{"service-type", Variant(kOpenVpn)}, {"data", Variant(data)}}}};
}

const std::map<std::string, std::string> kPasswordData {{"connection-type", "password"}, {"password-flags", "1"}};

// A service on a connection of its own, answering every method below `root`
// through `handle`, which replies or returns false for an unknown method.
class FakeService
{
public:
    FakeService(const std::string &name, const std::string &root)
        : bus(base::dbus::Connection::open(base::dbus::Connection::Bus::Session, core::mainLoop()))
        , m_name(name)
    {
        sd_bus_add_fallback(bus->bus(), &m_slot, root.c_str(), &FakeService::dispatch, this);
    }
    virtual ~FakeService() { sd_bus_slot_unref(m_slot); }

    bool serve() { return bus->requestName(m_name); }
    void stop() { bus->releaseName(m_name); }

    int count(const std::string &member) const
    {
        int result = 0;
        for (const auto &call : calls)
            result += call.member() == member;
        return result;
    }

    const base::dbus::Message *last(const std::string &member) const
    {
        for (auto it = calls.rbegin(); it != calls.rend(); ++it) {
            if (it->member() == member)
                return &*it;
        }
        return nullptr;
    }

    std::unique_ptr<base::dbus::Connection> bus;
    std::vector<base::dbus::Message> calls;

protected:
    virtual bool handle(MethodCall &call) = 0;

private:
    static int dispatch(sd_bus_message *raw, void *data, sd_bus_error *)
    {
        if (!sd_bus_message_is_method_call(raw, nullptr, nullptr))
            return 0;
        base::dbus::Message message(raw);
        const std::string member = message.member();
        if (member == "Introspect" || member == "Ping" || member == "GetMachineId")
            return 0;
        auto *self = static_cast<FakeService *>(data);
        self->calls.push_back(message);
        MethodCall call(message);
        return self->handle(call) ? 1 : 0;
    }

    std::string m_name;
    sd_bus_slot *m_slot = nullptr;
};

// NetworkManager: its objects, the agent manager, and AddAndActivate.
class FakeNetworkManager final : public FakeService
{
public:
    FakeNetworkManager()
        : FakeService("org.freedesktop.NetworkManager", "/org/freedesktop")
    {
        objects["/org/freedesktop/NetworkManager"]["org.freedesktop.NetworkManager"] = {
            {"Version", Variant(std::string("1.52.1"))},
            {"State", Variant(uint32_t(70))},
            {"NetworkingEnabled", Variant(true)},
            {"WirelessEnabled", Variant(true)},
            {"WirelessHardwareEnabled", Variant(true)},
            {"Devices", Variant(std::vector<ObjectPath>())},
            {"ActiveConnections", Variant(std::vector<ObjectPath>())},
        };
        objects["/org/freedesktop/NetworkManager/Settings"]["org.freedesktop.NetworkManager.Settings"] = {
            {"Connections", Variant(std::vector<ObjectPath>())}};
    }

    // A Wi-Fi device listing one WEP access point.
    void addWirelessDevice()
    {
        objects[kAccessPoint]["org.freedesktop.NetworkManager.AccessPoint"] = {
            {"Ssid", Variant(std::vector<uint8_t> {'S', 't', 'a', 't', 'i', 'o', 'n'})},
            {"Flags", Variant(uint32_t(1))},
            {"WpaFlags", Variant(uint32_t(0))},
            {"RsnFlags", Variant(uint32_t(0))},
            {"Strength", Variant(uint8_t(80))},
            {"HwAddress", Variant(std::string("00:11:22:33:44:55"))},
            {"Mode", Variant(uint32_t(2))},
        };
        objects[kDevice]["org.freedesktop.NetworkManager.Device"] = {
            {"DeviceType", Variant(uint32_t(2))},
            {"State", Variant(uint32_t(30))},
            {"Interface", Variant(std::string("wlan0"))},
            {"ActiveConnection", Variant(ObjectPath {"/"})},
        };
        objects[kDevice]["org.freedesktop.NetworkManager.Device.Wireless"] = {
            {"AccessPoints", Variant(std::vector<ObjectPath> {{kAccessPoint}})},
            {"WirelessCapabilities", Variant(uint32_t(0x3f))},
        };
        objects["/org/freedesktop/NetworkManager"]["org.freedesktop.NetworkManager"]["Devices"] =
                Variant(std::vector<ObjectPath> {{kDevice}});
    }

    std::map<std::string, std::map<std::string, VariantMap>> objects;
    std::string agentIdentifier;

protected:
    bool handle(MethodCall &call) override
    {
        const std::string member = call.message().member();
        if (member == "GetManagedObjects") {
            std::map<ObjectPath, std::map<std::string, VariantMap>> managed;
            for (const auto &[path, interfaces] : objects)
                managed[ObjectPath {path}] = interfaces;
            call.reply(managed);
        } else if (member == "GetPermissions") {
            call.reply(std::map<std::string, std::string>());
        } else if (member == "RegisterWithCapabilities") {
            agentIdentifier = call.message().argument<std::string>(0).value_or(std::string());
            call.reply();
        } else if (member == "AddAndActivateConnection") {
            call.reply(ObjectPath {"/org/freedesktop/NetworkManager/Settings/9"},
                       ObjectPath {"/org/freedesktop/NetworkManager/ActiveConnection/9"});
        } else if (member == "Unregister" || member == "RequestScan") {
            call.reply();
        } else {
            return false;
        }
        return true;
    }
};

// The Secret Service: items in one collection, never locked.
class FakeSecretService final : public FakeService
{
public:
    struct Item {
        std::map<std::string, std::string> attributes;
        std::string label;
        std::string value;
    };

    FakeSecretService()
        : FakeService("org.freedesktop.secrets", "/org/freedesktop/secrets")
    {
    }

    std::string store(const std::map<std::string, std::string> &attributes, const std::string &value)
    {
        const std::string path = "/org/freedesktop/secrets/collection/login/" + std::to_string(++m_next);
        items[path] = {attributes, {}, value};
        return path;
    }

    std::map<std::string, Item> items;
    // Unlocking asks the user through a prompt, which grants it.
    bool locked = false;

protected:
    bool handle(MethodCall &call) override
    {
        const base::dbus::Message &message = call.message();
        const std::string member = message.member();
        const std::string path = message.path();
        if (member == "OpenSession") {
            call.reply(Variant(std::string()), ObjectPath {"/org/freedesktop/secrets/session/1"});
        } else if (member == "Close") {
            call.reply();
        } else if (member == "SearchItems") {
            const auto wanted = message.argument<std::map<std::string, std::string>>(0).value_or(
                    std::map<std::string, std::string>());
            std::vector<ObjectPath> found;
            for (const auto &[item, data] : items) {
                bool matches = true;
                for (const auto &[key, value] : wanted) {
                    const auto it = data.attributes.find(key);
                    matches &= it != data.attributes.end() && it->second == value;
                }
                if (matches)
                    found.push_back({item});
            }
            call.reply(found, std::vector<ObjectPath>());
        } else if (member == "Unlock") {
            const auto objects = message.argument<std::vector<ObjectPath>>(0).value_or(std::vector<ObjectPath>());
            if (locked) {
                m_unlocking = objects;
                call.reply(std::vector<ObjectPath>(), ObjectPath {kPrompt});
            } else {
                call.reply(objects, ObjectPath {"/"});
            }
        } else if (member == "Prompt" && path == kPrompt) {
            call.reply();
            locked = false;
            bus->emitSignal(kPrompt, "org.freedesktop.Secret.Prompt", "Completed", false,
                            Variant(std::exchange(m_unlocking, {})));
        } else if (member == "ReadAlias") {
            call.reply(ObjectPath {"/org/freedesktop/secrets/collection/login"});
        } else if (member == "CreateItem") {
            VariantMap properties;
            std::tuple<ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> secret;
            message.read(properties, secret);
            const auto attributes = properties["org.freedesktop.Secret.Item.Attributes"]
                                            .get<std::map<std::string, std::string>>()
                                            .value_or(std::map<std::string, std::string>());
            const auto &bytes = std::get<2>(secret);
            const std::string item = store(attributes, std::string(bytes.begin(), bytes.end()));
            items[item].label = properties["org.freedesktop.Secret.Item.Label"].value<std::string>();
            call.reply(ObjectPath {item}, ObjectPath {"/"});
        } else if (member == "Get" && items.count(path)) {
            call.reply(Variant(items[path].attributes));
        } else if (member == "GetSecret" && items.count(path)) {
            const std::string &value = items[path].value;
            call.reply(std::make_tuple(ObjectPath {"/org/freedesktop/secrets/session/1"}, std::vector<uint8_t>(),
                                       std::vector<uint8_t>(value.begin(), value.end()), std::string("text/plain")));
        } else if (member == "Delete" && items.count(path)) {
            items.erase(path);
            call.reply(ObjectPath {"/"});
        } else {
            return false;
        }
        return true;
    }

private:
    static constexpr const char *kPrompt = "/org/freedesktop/secrets/prompt/1";

    int m_next = 0;
    std::vector<ObjectPath> m_unlocking;
};

const std::string kAdapter = "/org/bluez/hci0";
const std::string kBluetoothDevice = kAdapter + "/dev_AA_BB_CC_DD_EE_FF";

// BlueZ: one adapter, one device, the agent manager.
class FakeBluez final : public FakeService
{
public:
    FakeBluez()
        : FakeService("org.bluez", "/")
    {
    }

    template<typename... Args>
    base::dbus::PendingCall ask(const std::string &method, std::optional<base::dbus::Reply> *reply,
                                const Args &...args)
    {
        base::dbus::Message message = bus->newMethodCall(agentOwner, agentPath, "org.bluez.Agent1", method);
        message.append(args...);
        return bus->callAsync(message, [reply](const base::dbus::Reply &r) { *reply = r; });
    }

    std::string agentOwner;
    std::string agentPath;

protected:
    bool handle(MethodCall &call) override
    {
        const base::dbus::Message &message = call.message();
        const std::string member = message.member();
        if (member == "GetManagedObjects") {
            std::map<ObjectPath, std::map<std::string, VariantMap>> objects;
            objects[{kAdapter}]["org.bluez.Adapter1"] = {{"Address", Variant(std::string("11:22:33:44:55:66"))},
                                                          {"Alias", Variant(std::string("Laptop"))},
                                                          {"Powered", Variant(true)}};
            objects[{kBluetoothDevice}]["org.bluez.Device1"] = {
                    {"Address", Variant(std::string("AA:BB:CC:DD:EE:FF"))},
                    {"Alias", Variant(std::string("Headphones"))},
                    {"Adapter", Variant(ObjectPath {kAdapter})},
                    {"Paired", Variant(false)},
                    {"Connected", Variant(false)}};
            call.reply(objects);
        } else if (member == "RegisterAgent") {
            agentOwner = message.sender();
            agentPath = message.argument<ObjectPath>(0).value_or(ObjectPath()).path;
            call.reply();
        } else {
            call.reply();
        }
        return true;
    }
};

class SecretPromptsTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    test::TemporaryDirectory directory;
    std::unique_ptr<shell::Shell> shell;

    std::unique_ptr<FakeNetworkManager> networkManager;
    std::unique_ptr<FakeSecretService> secrets;
    // The agent's "system" bus, private like the rest.
    std::unique_ptr<base::dbus::Connection> systemBus;
    networkmanager::Client *client = nullptr;
    shell::SecretPrompts *prompts = nullptr;
    shell::NetworkSecretAgent *agent = nullptr;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat,
                                             base::dbus::Connection::session(core::mainLoop())};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        shell->setApplications(std::make_unique<test::FakeApplications>());
        shell->setTaskWindows(std::make_unique<test::FakeTaskWindows>());

        secrets = std::make_unique<FakeSecretService>();
        ASSERT_TRUE(secrets->serve());
        networkManager = std::make_unique<FakeNetworkManager>();
        networkManager->addWirelessDevice();
        ASSERT_TRUE(networkManager->serve());
        systemBus = base::dbus::Connection::open(base::dbus::Connection::Bus::Session, core::mainLoop());
        networkmanager::Client::setInstanceBus(systemBus.get());
        client = networkmanager::Client::instance();
        ASSERT_TRUE(test::runUntil([this] { return client->ready(); }));
    }

    void TearDown() override
    {
        shell.reset();
        networkmanager::Client::setInstanceBus(nullptr);
        systemBus.reset();
        networkManager.reset();
        secrets.reset();
        seat.shutdown();
    }

    void start(std::shared_ptr<bluez::Manager> bluetooth = nullptr)
    {
        prompts = shell->addComponent(std::make_unique<shell::SecretPrompts>(
                shell.get(), client, base::dbus::Connection::session(core::mainLoop()), std::move(bluetooth)));
        prompts->start();
        agent = prompts->networkAgent();
        ASSERT_TRUE(test::runUntil([this] { return networkManager->count("RegisterWithCapabilities") == 1; }));
    }

    void frame() { compositor.updateTextures(renderer); }

    // NetworkManager asks the agent, from its own connection.
    base::dbus::PendingCall getSecrets(const Settings &connection, const std::string &setting, uint32_t flags,
                                       std::optional<base::dbus::Reply> *reply,
                                       const std::vector<std::string> &hints = {},
                                       const std::string &connectionPath = kConnectionPath)
    {
        base::dbus::Message message = networkManager->bus->newMethodCall(systemBus->uniqueName(), kAgentPath,
                                                                         kAgentInterface, "GetSecrets");
        message.append(connection, ObjectPath {connectionPath}, setting, hints, flags);
        return networkManager->bus->callAsync(message, [reply](const base::dbus::Reply &r) { *reply = r; });
    }

    base::dbus::Reply callAgent(const std::string &method, const Settings &connection)
    {
        std::optional<base::dbus::Reply> reply;
        base::dbus::Message message = networkManager->bus->newMethodCall(systemBus->uniqueName(), kAgentPath,
                                                                         kAgentInterface, method);
        message.append(connection, ObjectPath {kConnectionPath});
        base::dbus::PendingCall pending =
                networkManager->bus->callAsync(message, [&reply](const base::dbus::Reply &r) { reply = r; });
        EXPECT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
        return reply.value_or(base::dbus::Reply());
    }

    static std::string secretOf(const base::dbus::Reply &reply, const std::string &setting, const std::string &key)
    {
        const Settings secrets = reply.value<Settings>().value_or(Settings());
        const auto it = secrets.find(setting);
        if (it == secrets.end())
            return {};
        return networkmanager::variants::toString(networkmanager::variants::value(it->second, key));
    }

    static std::map<std::string, std::string> vpnSecretsOf(const base::dbus::Reply &reply)
    {
        const Settings secrets = reply.value<Settings>().value_or(Settings());
        const auto it = secrets.find("vpn");
        if (it == secrets.end())
            return {};
        return networkmanager::variants::toStringMap(networkmanager::variants::value(it->second, "secrets"));
    }

    wm::Window *dialogWindow(shell::SecretDialog *dialog) const
    {
        return dialog && dialog->window() ? dialog->window()->wmWindow() : nullptr;
    }
};

TEST(VpnSecretFieldsTest, HintsAndPluginRules)
{
    const auto fields = shell::vpnSecretFields(kOpenVpn, kPasswordData, {});
    ASSERT_EQ(1u, fields.size());
    EXPECT_EQ("password", fields[0].key);
    EXPECT_EQ("Password", fields[0].label);

    const auto hinted = shell::vpnSecretFields(
            kOpenVpn, kPasswordData, {"x-vpn-message:  Enter the code  ", "x-dynamic-challenge-echo:Code"});
    ASSERT_EQ(1u, hinted.size());
    EXPECT_EQ("Code", hinted[0].label);
    EXPECT_FALSE(hinted[0].secret);
    EXPECT_EQ("Enter the code", shell::vpnHintMessage({"x-vpn-message:  Enter the code  "}));

    const auto forti = shell::vpnSecretFields("org.freedesktop.NetworkManager.fortisslvpn", {{"otp-flags", "2"}}, {});
    ASSERT_EQ(2u, forti.size());
    EXPECT_EQ("otp", forti[1].key);
    EXPECT_TRUE(shell::vpnSecretFields("org.freedesktop.NetworkManager.openconnect", {}, {}).empty());
    EXPECT_TRUE(shell::vpnSecretFields(kOpenVpn, {{"connection-type", "password"}, {"password-flags", "4"}}, {}).empty());
}

TEST(WirelessPasswordTest, ValidKeysAndWhatCanBeTyped)
{
    using Kind = shell::NetworkSecretAgent::Kind;
    EXPECT_TRUE(shell::WirelessPasswordDialog::acceptable(Kind::WpaPassword, "hunter22"));
    EXPECT_FALSE(shell::WirelessPasswordDialog::acceptable(Kind::WpaPassword, "short"));
    EXPECT_TRUE(shell::WirelessPasswordDialog::acceptable(Kind::WpaPassword, std::string(64, 'a')));
    EXPECT_FALSE(shell::WirelessPasswordDialog::acceptable(Kind::WpaPassword, std::string(64, 'x')));
    EXPECT_FALSE(shell::WirelessPasswordDialog::intermediate(Kind::WpaPassword, std::string(64, 'x')));
    EXPECT_FALSE(shell::WirelessPasswordDialog::intermediate(Kind::WpaPassword, "caf\xc3\xa9"));
    EXPECT_TRUE(shell::WirelessPasswordDialog::acceptable(Kind::WepKey, "abcde"));
    EXPECT_TRUE(shell::WirelessPasswordDialog::acceptable(Kind::WepKey, "0123456789"));
    EXPECT_FALSE(shell::WirelessPasswordDialog::acceptable(Kind::WepKey, "abcdef"));
    EXPECT_TRUE(shell::WirelessPasswordDialog::intermediate(Kind::WepKey, std::string(20, 'f')));
    EXPECT_FALSE(shell::WirelessPasswordDialog::intermediate(Kind::WepKey, std::string(14, 'x')));
    EXPECT_TRUE(shell::WirelessPasswordDialog::acceptable(Kind::EapPassword, "x"));
    EXPECT_FALSE(shell::WirelessPasswordDialog::acceptable(Kind::EapPassword, ""));
}

TEST_F(SecretPromptsTest, RegistersAsTheSessionsAgent)
{
    start();
    EXPECT_EQ("org.lingmo.network", networkManager->agentIdentifier);
    EXPECT_EQ(1u, networkManager->last("RegisterWithCapabilities")->argument<uint32_t>(1).value_or(0));

    // A NetworkManager that starts again is registered with again.
    networkManager->stop();
    ASSERT_TRUE(test::runUntil([this] { return !client->ready(); }));
    ASSERT_TRUE(networkManager->serve());
    ASSERT_TRUE(test::runUntil([this] { return networkManager->count("RegisterWithCapabilities") == 2; }));

    test::DBusTestClient other;
    const std::string xml = other.introspect(systemBus->uniqueName(), kAgentPath);
    EXPECT_NE(std::string::npos, xml.find("<method name=\"GetSecrets\">")) << xml;
    EXPECT_NE(std::string::npos, xml.find("name=\"settingName\"")) << xml;
    const std::string prompt = other.introspect("org.lingmo.SecretAgent", "/SecretAgent");
    EXPECT_NE(std::string::npos, prompt.find("<method name=\"ConnectWireless\">")) << prompt;
    EXPECT_NE(std::string::npos, prompt.find("name=\"accessPointPath\"")) << prompt;
}

TEST_F(SecretPromptsTest, AsksWhenNothingIsStoredAndKeepsWhatWasTyped)
{
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(wpaConnection(1), kWirelessSecurity, kAllowInteraction, &reply);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));
    EXPECT_EQ("Cafe", agent->connectionName());
    EXPECT_EQ(shell::NetworkSecretAgent::Kind::WpaPassword, agent->kind());
    EXPECT_FALSE(agent->retry());

    // The dialog is up, above applications, and Connect waits for a valid passphrase.
    shell::WirelessPasswordDialog *dialog = prompts->wirelessDialog();
    ASSERT_TRUE(dialog->isOpen());
    frame();
    wm::Window *window = dialogWindow(dialog);
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Normal, window->role());
    EXPECT_TRUE(window->isProtectedFromControl());
    EXPECT_TRUE(window->isAlwaysOnTop());
    EXPECT_EQ(360, window->bounds().width());
    EXPECT_EQ("Wi-Fi Password Required", window->title());
    EXPECT_TRUE(dialog->field()->hasFocus());
    EXPECT_FALSE(dialog->primaryButton()->isEnabled());
    EXPECT_EQ(ui::TextField::EchoMode::Password, dialog->field()->echoMode());
    dialog->field()->setText("hunter22");
    EXPECT_TRUE(dialog->primaryButton()->isEnabled());
    dialog->primaryButton()->onClicked();

    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_EQ("hunter22", secretOf(*reply, kWirelessSecurity, "psk"));
    EXPECT_FALSE(agent->pending());
    EXPECT_FALSE(dialog->isOpen());

    // NetworkManager gives typed secrets back to no agent: the agent kept it.
    ASSERT_TRUE(test::runUntil([this] { return secrets->items.size() == 1; }));
    const auto &item = secrets->items.begin()->second;
    EXPECT_EQ("hunter22", item.value);
    EXPECT_EQ(kUuid, item.attributes.at("connection-uuid"));
    EXPECT_EQ(kWirelessSecurity, item.attributes.at("setting-name"));
    EXPECT_EQ("psk", item.attributes.at("setting-key"));
    EXPECT_EQ("org.freedesktop.NetworkManager.Connection", item.attributes.at("xdg:schema"));
}

TEST_F(SecretPromptsTest, AnswersFromTheKeyring)
{
    secrets->store({{"connection-uuid", kUuid}, {"setting-name", kWirelessSecurity}, {"setting-key", "psk"}},
                   "stored secret");
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(wpaConnection(1), kWirelessSecurity, kAllowInteraction, &reply);
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_EQ("stored secret", secretOf(*reply, kWirelessSecurity, "psk"));
    EXPECT_FALSE(agent->pending());
    EXPECT_FALSE(prompts->wirelessDialog()->isOpen());
}

TEST_F(SecretPromptsTest, SystemOwnedSecretsStayOutOfTheKeyring)
{
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(wpaConnection(0), kWirelessSecurity, kAllowInteraction, &reply);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));
    agent->respond("hunter22");
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    test::runFor(std::chrono::milliseconds(100));
    EXPECT_TRUE(secrets->items.empty());
}

TEST_F(SecretPromptsTest, ARejectedPasswordIsAskedForAgainAndClosingCancels)
{
    secrets->store({{"connection-uuid", kUuid}, {"setting-name", kWirelessSecurity}, {"setting-key", "psk"}}, "stale");
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(wpaConnection(1), kWirelessSecurity, kAllowInteraction | kRequestNew, &reply);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));
    EXPECT_TRUE(agent->retry());
    shell::WirelessPasswordDialog *dialog = prompts->wirelessDialog();
    EXPECT_EQ("The password for “Cafe” was not accepted. Try again.",
              shell::WirelessPasswordDialog::text(*agent));

    // Escape closes the dialog, which declines.
    frame();
    input.onKey(input::Timestamp(std::chrono::milliseconds(10)), KEY_ESC, true);
    input.onKey(input::Timestamp(std::chrono::milliseconds(11)), KEY_ESC, false);
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_EQ("org.freedesktop.NetworkManager.SecretAgent.UserCanceled", reply->error().name);
    EXPECT_FALSE(agent->pending());
    EXPECT_FALSE(dialog->isOpen());
}

TEST_F(SecretPromptsTest, WithoutInteractionThereIsNothingToOffer)
{
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(wpaConnection(1), kWirelessSecurity, 0, &reply);
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_EQ("org.freedesktop.NetworkManager.SecretAgent.NoSecrets", reply->error().name);

    // Settings.Connection.GetSecrets reads expect an answer, not an error.
    std::optional<base::dbus::Reply> read;
    auto reading = getSecrets(wpaConnection(1), kWirelessSecurity, kNoErrors, &read);
    ASSERT_TRUE(test::runUntil([&read] { return read.has_value(); }));
    EXPECT_FALSE(read->isError());
    EXPECT_FALSE(agent->pending());
}

TEST_F(SecretPromptsTest, NetworkManagerCancelsAndNewerRequestsReplace)
{
    start();
    std::optional<base::dbus::Reply> first;
    auto pendingFirst = getSecrets(wpaConnection(1), kWirelessSecurity, kAllowInteraction, &first);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));

    std::optional<base::dbus::Reply> second;
    auto pendingSecond = getSecrets(wpaConnection(1), kWirelessSecurity, kAllowInteraction | kRequestNew, &second);
    ASSERT_TRUE(test::runUntil([&first] { return first.has_value(); }));
    EXPECT_EQ("org.freedesktop.NetworkManager.SecretAgent.AgentCanceled", first->error().name);
    EXPECT_TRUE(agent->pending());
    EXPECT_TRUE(agent->retry());

    base::dbus::Message cancel = networkManager->bus->newMethodCall(systemBus->uniqueName(), kAgentPath,
                                                                    kAgentInterface, "CancelGetSecrets");
    cancel.append(ObjectPath {kConnectionPath}, kWirelessSecurity);
    networkManager->bus->send(cancel);
    ASSERT_TRUE(test::runUntil([&second] { return second.has_value(); }));
    EXPECT_EQ("org.freedesktop.NetworkManager.SecretAgent.AgentCanceled", second->error().name);
    EXPECT_FALSE(agent->pending());
    EXPECT_FALSE(prompts->wirelessDialog()->isOpen());
}

TEST_F(SecretPromptsTest, OnlyNetworkManagerMayAsk)
{
    start();
    test::DBusTestClient stranger;
    const base::dbus::Reply reply = stranger.call(systemBus->uniqueName(), kAgentPath, kAgentInterface, "GetSecrets",
                                                  wpaConnection(1), ObjectPath {kConnectionPath}, kWirelessSecurity,
                                                  std::vector<std::string>(), kAllowInteraction);
    EXPECT_EQ("org.freedesktop.NetworkManager.SecretAgent.PermissionDenied", reply.error().name);
    EXPECT_FALSE(agent->pending());
}

TEST_F(SecretPromptsTest, SavingReplacesAndDeletingRemoves)
{
    secrets->store({{"connection-uuid", kUuid}, {"setting-name", kWirelessSecurity}, {"setting-key", "wep-key0"}},
                   "old key");
    start();
    Settings connection = wpaConnection(1);
    connection[kWirelessSecurity]["psk"] = Variant(std::string("saved password"));
    EXPECT_FALSE(callAgent("SaveSecrets", connection).isError());
    ASSERT_EQ(1u, secrets->items.size());
    EXPECT_EQ("psk", secrets->items.begin()->second.attributes.at("setting-key"));
    EXPECT_EQ("saved password", secrets->items.begin()->second.value);

    EXPECT_FALSE(callAgent("DeleteSecrets", connection).isError());
    EXPECT_TRUE(secrets->items.empty());
}

TEST_F(SecretPromptsTest, AsksBeforeANetworkIsAddedAndAnotherNetworkTakesOver)
{
    start();
    test::DBusTestClient controlCentre;
    EXPECT_FALSE(controlCentre
                         .call("org.lingmo.SecretAgent", "/SecretAgent", "org.lingmo.SecretAgent", "ConnectWireless",
                               kDevice, kAccessPoint, std::string("Old router"), true)
                         .isError());
    ASSERT_TRUE(agent->pending());
    EXPECT_EQ("Old router", agent->connectionName());
    EXPECT_EQ(shell::NetworkSecretAgent::Kind::WepKey, agent->kind());
    shell::WirelessPasswordDialog *dialog = prompts->wirelessDialog();
    ASSERT_TRUE(dialog->isOpen());
    EXPECT_EQ("Enter the WEP key for “Old router”.", shell::WirelessPasswordDialog::text(*agent));

    // A NetworkManager request for Wi-Fi meanwhile is given up when another network is picked.
    std::optional<base::dbus::Reply> leaving;
    auto pending = getSecrets(wpaConnection(1), kWirelessSecurity, kAllowInteraction, &leaving);
    test::runFor(std::chrono::milliseconds(50));
    dialog->field()->setText("typed");
    agent->connectWireless(kDevice, kAccessPoint + "0", "Cafe", false);
    ASSERT_TRUE(test::runUntil([&leaving] { return leaving.has_value(); }));
    EXPECT_EQ("org.freedesktop.NetworkManager.SecretAgent.UserCanceled", leaving->error().name);
    EXPECT_EQ("Cafe", agent->connectionName());
    // The window stays and starts over.
    EXPECT_TRUE(dialog->isOpen());
    EXPECT_TRUE(dialog->field()->text().empty());
    agent->cancel();

    // The password, once given, creates and activates the profile.
    agent->connectWireless(kDevice, kAccessPoint, "Old router", true);
    dialog->field()->setText("abcde");
    dialog->primaryButton()->onClicked();
    EXPECT_FALSE(agent->pending());
    ASSERT_TRUE(test::runUntil([this] { return networkManager->count("AddAndActivateConnection") == 1; }));
    const auto settings = networkManager->last("AddAndActivateConnection")->argument<Settings>(0).value_or(Settings());
    ASSERT_TRUE(settings.count(kWirelessSecurity));
    EXPECT_EQ("abcde", networkmanager::variants::toString(
                               networkmanager::variants::value(settings.at(kWirelessSecurity), "wep-key0")));
}

TEST_F(SecretPromptsTest, VpnSecretsAreAskedForInTheirOwnWindowAndKept)
{
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(vpnConnection(kPasswordData), "vpn", kAllowInteraction, &reply);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));
    EXPECT_EQ(shell::NetworkSecretAgent::Kind::VpnSecrets, agent->kind());
    EXPECT_FALSE(prompts->wirelessDialog()->isOpen());
    shell::VpnPasswordDialog *dialog = prompts->vpnDialog();
    ASSERT_TRUE(dialog->isOpen());
    frame();
    EXPECT_EQ("VPN Password Required", dialogWindow(dialog)->title());
    EXPECT_TRUE(dialogWindow(dialog)->isProtectedFromControl());
    ASSERT_EQ(1u, dialog->fields().size());
    EXPECT_TRUE(dialog->fields()[0]->hasFocus());
    EXPECT_EQ(ui::TextField::EchoMode::Password, dialog->fields()[0]->echoMode());
    dialog->showPassword()->setChecked(true);
    dialog->showPassword()->onToggled(true);
    EXPECT_EQ(ui::TextField::EchoMode::Normal, dialog->fields()[0]->echoMode());
    EXPECT_FALSE(dialog->primaryButton()->isEnabled());
    dialog->fields()[0]->setText("s3cret");
    EXPECT_TRUE(dialog->primaryButton()->isEnabled());
    dialog->primaryButton()->onClicked();

    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_EQ("s3cret", vpnSecretsOf(*reply)["password"]);
    EXPECT_FALSE(dialog->isOpen());
    ASSERT_TRUE(test::runUntil([this] { return secrets->items.size() == 1; }));
    EXPECT_EQ("vpn", secrets->items.begin()->second.attributes.at("setting-name"));
    EXPECT_EQ("password", secrets->items.begin()->second.attributes.at("setting-key"));

    // Next time the keyring answers.
    std::optional<base::dbus::Reply> again;
    auto pendingAgain = getSecrets(vpnConnection(kPasswordData), "vpn", kAllowInteraction, &again);
    ASSERT_TRUE(test::runUntil([&again] { return again.has_value(); }));
    EXPECT_EQ("s3cret", vpnSecretsOf(*again)["password"]);
    EXPECT_FALSE(agent->pending());

    // Without interaction an answer with no secrets has NetworkManager ask again with it.
    secrets->items.clear();
    std::optional<base::dbus::Reply> quiet;
    auto pendingQuiet = getSecrets(vpnConnection(kPasswordData), "vpn", 0, &quiet);
    ASSERT_TRUE(test::runUntil([&quiet] { return quiet.has_value(); }));
    EXPECT_FALSE(quiet->isError());
    EXPECT_TRUE(vpnSecretsOf(*quiet).empty());
}

TEST_F(SecretPromptsTest, LanguageChangeKeepsThePromptAndWhatWasTyped)
{
    start();
    std::optional<base::dbus::Reply> reply;
    auto pending = getSecrets(vpnConnection(kPasswordData), "vpn", kAllowInteraction, &reply);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));
    shell::VpnPasswordDialog *dialog = prompts->vpnDialog();
    ASSERT_TRUE(dialog->isOpen());
    ASSERT_EQ(1u, dialog->fields().size());
    ui::TextField *field = dialog->fields()[0];
    field->setText("s3cret");
    dialog->showPassword()->setChecked(true);
    dialog->showPassword()->onToggled(true);

    ui::Application::instance()->languageChanged.emit();
    test::runFor(std::chrono::milliseconds(5));
    frame();
    EXPECT_TRUE(dialog->isOpen());
    EXPECT_TRUE(agent->pending());
    ASSERT_EQ(1u, dialog->fields().size());
    EXPECT_EQ(field, dialog->fields()[0]);
    EXPECT_EQ("s3cret", field->text());
    EXPECT_TRUE(dialog->showPassword()->isChecked());
    EXPECT_EQ(ui::TextField::EchoMode::Normal, field->echoMode());
    EXPECT_EQ("VPN Password Required", dialogWindow(dialog)->title());
    EXPECT_FALSE(reply.has_value());

    ASSERT_TRUE(dialog->primaryButton()->isEnabled());
    dialog->primaryButton()->onClicked();
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_EQ("s3cret", vpnSecretsOf(*reply)["password"]);
}

TEST_F(SecretPromptsTest, ReturnInAVpnFieldMovesOnToTheNextRequest)
{
    start();
    std::optional<base::dbus::Reply> first;
    auto pendingFirst = getSecrets(vpnConnection(kPasswordData), "vpn", kAllowInteraction, &first);
    ASSERT_TRUE(test::runUntil([this] { return agent->pending(); }));
    std::optional<base::dbus::Reply> second;
    auto pendingSecond = getSecrets(vpnConnection(kPasswordData), "vpn", kAllowInteraction, &second, {},
                                    "/org/freedesktop/NetworkManager/Settings/8");
    // Nothing stored: both are asked for, the second after the first.
    ASSERT_TRUE(test::runUntil([this] { return secrets->count("Close") == 2; }));
    test::runFor(std::chrono::milliseconds(50));

    // The next request rebuilds the fields while the first request's field
    // still handles the key.
    shell::VpnPasswordDialog *dialog = prompts->vpnDialog();
    ASSERT_TRUE(dialog->isOpen());
    ASSERT_EQ(1u, dialog->fields().size());
    ui::TextField *field = dialog->fields()[0];
    ASSERT_TRUE(field->hasFocus());
    field->setText("s3cret");
    // Runs after onAccepted, on the field.
    bool finished = false;
    field->onEditingFinished = [&finished] { finished = true; };
    auto *window = static_cast<ui::EmbeddedWindow *>(dialog->window()->window());
    window->key(ui::KeyEvent {ui::key::Return, KEY_ENTER, "\r"}, true);
    window->key(ui::KeyEvent {ui::key::Return, KEY_ENTER, "\r"}, false);

    ASSERT_TRUE(test::runUntil([&first] { return first.has_value(); }));
    EXPECT_EQ("s3cret", vpnSecretsOf(*first)["password"]);
    EXPECT_TRUE(finished);
    EXPECT_TRUE(dialog->isOpen());
    ASSERT_EQ(1u, dialog->fields().size());
    EXPECT_NE(field, dialog->fields()[0]);
    dialog->cancelButton()->onClicked();
    ASSERT_TRUE(test::runUntil([&second] { return second.has_value(); }));
    EXPECT_TRUE(second->isError());
}

TEST_F(SecretPromptsTest, ALockedKeyringIsUnlockedThroughItsPrompt)
{
    secrets->locked = true;
    shell::SecretStore store(base::dbus::Connection::session(core::mainLoop()));
    bool saved = false;
    store.save(kUuid, "Cafe", kWirelessSecurity, "psk", "hunter22", [&saved] { saved = true; });
    ASSERT_TRUE(test::runUntil([&saved] { return saved; }));
    EXPECT_EQ(1, secrets->count("Prompt"));
    ASSERT_EQ(1u, secrets->items.size());
    EXPECT_EQ("hunter22", secrets->items.begin()->second.value);
    // What the prompt held goes on the next turn.
    test::runFor(std::chrono::milliseconds(10));
}

TEST_F(SecretPromptsTest, BluetoothPairingIsConfirmedInTheDialog)
{
    FakeBluez bluez;
    ASSERT_TRUE(bluez.serve());
    auto bluetooth = std::make_shared<bluez::Manager>(systemBus.get());
    start(bluetooth);
    ASSERT_TRUE(test::runUntil([&] { return bluetooth->agentReady() && bluez.count("RequestDefaultAgent") == 1; }));

    std::optional<base::dbus::Reply> reply;
    auto pending = bluez.ask("RequestConfirmation", &reply, ObjectPath {kBluetoothDevice}, uint32_t(42));
    shell::BluetoothPairingDialog *dialog = prompts->pairingDialog();
    ASSERT_TRUE(test::runUntil([dialog] { return dialog->isOpen(); }));
    frame();
    EXPECT_EQ("Bluetooth Pairing Request", dialogWindow(dialog)->title());
    EXPECT_TRUE(dialogWindow(dialog)->isProtectedFromControl());
    EXPECT_EQ("000042", dialog->code()->text());
    EXPECT_TRUE(dialog->code()->isVisible());
    EXPECT_FALSE(dialog->codeField()->isVisible());
    EXPECT_EQ("Confirm that \u201cHeadphones\u201d shows this code.",
              shell::BluetoothPairingDialog::text("RequestConfirmation", "Headphones"));
    EXPECT_TRUE(dialog->primaryButton()->hasFocus());
    dialog->primaryButton()->onClicked();
    ASSERT_TRUE(test::runUntil([&reply] { return reply.has_value(); }));
    EXPECT_FALSE(reply->isError());
    EXPECT_FALSE(dialog->isOpen());

    // A passkey with letters is refused; the error shows in the open dialog.
    std::optional<base::dbus::Reply> passkey;
    auto pendingPasskey = bluez.ask("RequestPasskey", &passkey, ObjectPath {kBluetoothDevice});
    ASSERT_TRUE(test::runUntil([dialog] { return dialog->isOpen(); }));
    EXPECT_TRUE(dialog->codeField()->isVisible());
    EXPECT_TRUE(dialog->codeField()->hasFocus());
    EXPECT_FALSE(dialog->primaryButton()->isEnabled());
    const int before = dialog->window()->size().height();
    dialog->codeField()->setText("12a");
    dialog->primaryButton()->onClicked();
    EXPECT_TRUE(dialog->error()->isVisible());
    EXPECT_GT(dialog->window()->size().height(), before);
    EXPECT_FALSE(passkey.has_value());

    // Dismissing declines.
    dialog->cancelButton()->onClicked();
    ASSERT_TRUE(test::runUntil([&passkey] { return passkey.has_value(); }));
    EXPECT_TRUE(passkey->isError());
    EXPECT_FALSE(dialog->isOpen());
}

} // namespace
