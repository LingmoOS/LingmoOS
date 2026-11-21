// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The portals on a private session bus, served from this process on its own
// connection, with mock Lingmo services, ScreenSaver and file chooser.

#include "filechooserportal.h"
#include "inhibitportal.h"
#include "screencastportal.h"
#include "settingsportal.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/json.h"
#include "base/process.h"
#include "base/url.h"

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <poll.h>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

using namespace Lingmo;
using base::dbus::Connection;
using base::dbus::MethodCall;
using base::dbus::ObjectPath;
using base::dbus::Reply;
using base::dbus::Variant;
using base::dbus::VariantMap;
using namespace std::chrono_literals;

extern char **environ;

static const std::string kService = "org.freedesktop.impl.portal.desktop.lingmo";
static const std::string kPath = "/org/freedesktop/portal/desktop";
static const std::string kSettings = "org.freedesktop.impl.portal.Settings";
static const std::string kNamespace = "org.freedesktop.appearance";
static const std::string kRequest = "/org/freedesktop/portal/desktop/request/test/";
static const std::string kSession = "/org/freedesktop/portal/desktop/session/test/";
static const std::string kScreenCast = "org.freedesktop.impl.portal.ScreenCast";
static const std::string kSessionInterface = "org.freedesktop.impl.portal.Session";
static const std::string kFrontend = "/usr/libexec/xdg-desktop-portal";

using Filter = std::tuple<std::string, std::vector<std::tuple<uint32_t, std::string>>>;

static base::EventLoop *g_loop = nullptr;

// Runs the loop until done() holds or the timeout passes; returns done().
static bool runUntil(const std::function<bool()> &done, std::chrono::milliseconds timeout = 5s)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    g_loop->dispatch();
    while (!done()) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline)
            return done();
        pollfd ready {g_loop->fd(), POLLIN, 0};
        poll(&ready, 1, 5);
        g_loop->dispatch();
    }
    return true;
}

static void runFor(std::chrono::milliseconds duration)
{
    runUntil([] { return false; }, duration);
}

static std::unique_ptr<Connection> openConnection()
{
    return Connection::open(Connection::Bus::Session, g_loop);
}

// Another process on the bus: calls that wait on the loop, so the portal in
// this process answers meanwhile.
struct Client
{
    std::unique_ptr<Connection> connection = openConnection();

    template<typename... Args>
    base::dbus::PendingCall callAsync(std::optional<Reply> &reply, const std::string &service, const std::string &path,
                                      const std::string &interface, const std::string &method, const Args &...args)
    {
        base::dbus::Message message = connection->newMethodCall(service, path, interface, method);
        message.append(args...);
        return connection->callAsync(message, [&reply](const Reply &r) { reply = r; });
    }

    template<typename... Args>
    Reply callService(const std::string &service, const std::string &path, const std::string &interface,
                      const std::string &method, const Args &...args)
    {
        std::optional<Reply> reply;
        base::dbus::PendingCall pending = callAsync(reply, service, path, interface, method, args...);
        EXPECT_TRUE(runUntil([&] { return reply.has_value(); })) << method;
        return reply.value_or(Reply());
    }

    template<typename... Args>
    Reply call(const std::string &interface, const std::string &method, const Args &...args)
    {
        return callService(kService, kPath, interface, method, args...);
    }

    template<typename... Args>
    Reply callAt(const std::string &path, const std::string &interface, const std::string &method,
                 const Args &...args)
    {
        return callService(kService, path, interface, method, args...);
    }

    bool owned(const std::string &name)
    {
        return callService("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameHasOwner",
                           name)
                .value<bool>()
                .value_or(false);
    }
};

// org.lingmo.Services' appearance object.
struct AppearanceService
{
    std::unique_ptr<Connection> connection = openConnection();
    base::dbus::Object object {connection.get(), "/org/lingmo/Services/Appearance",
                               "org.lingmo.Services.Appearance"};
    bool dark = false;

    AppearanceService()
    {
        object.addProperty("isDarkMode", [this] { return dark; });
        object.addSignal("darkModeChanged", "b");
        object.publish();
    }

    void own(bool value)
    {
        dark = value;
        EXPECT_TRUE(connection->requestName("org.lingmo.Services"));
    }

    void release() { connection->releaseName("org.lingmo.Services"); }

    void change(bool value)
    {
        dark = value;
        object.emitSignal("darkModeChanged", value);
    }
};

// File Manager's org.lingmo.FileChooser; Open waits without a reply.
struct ChooserService
{
    std::unique_ptr<Connection> connection = openConnection();
    base::dbus::Object object {connection.get(), "/org/lingmo/FileChooser", "org.lingmo.FileChooser"};
    std::optional<std::string> reply;
    std::vector<std::pair<std::string, std::vector<std::string>>> calls;
    std::vector<MethodCall> pending;

    explicit ChooserService(std::optional<std::string> answer = std::nullopt)
        : reply(std::move(answer))
    {
        object.addMethod("Open", "ss", "s", [this](MethodCall &call) {
            std::string id;
            std::string payload;
            call.read(id, payload);
            calls.push_back({"Open", {id, payload}});
            if (reply)
                call.reply(*reply);
            else
                pending.push_back(call);
        });
        object.addMethod("Close", "s", "", [this](MethodCall &call) {
            std::string id;
            call.read(id);
            calls.push_back({"Close", {id}});
            call.reply();
        });
        object.publish();
        EXPECT_TRUE(connection->requestName("org.lingmo.FileChooser"));
    }

    base::json::Value request() const
    {
        EXPECT_EQ(calls.size(), 1u);
        if (calls.empty())
            return {};
        EXPECT_EQ(calls[0].first, "Open");
        return base::json::Value::parse(calls[0].second[1]).value_or(base::json::Value());
    }
};

// org.freedesktop.ScreenSaver, handing out cookies.
struct ScreenSaverService
{
    std::unique_ptr<Connection> connection = openConnection();
    base::dbus::Object object {connection.get(), "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver"};
    std::map<uint32_t, std::pair<std::string, std::string>> cookies;
    uint32_t nextCookie = 0;
    bool delay = false;
    bool fail = false;
    std::optional<std::pair<MethodCall, uint32_t>> delayed;

    ScreenSaverService()
    {
        object.addMethod("Inhibit", "ss", "u", [this](MethodCall &call) {
            if (fail) {
                call.replyError("org.freedesktop.DBus.Error.Failed", "Unavailable");
                return;
            }
            std::string app;
            std::string reason;
            call.read(app, reason);
            cookies[++nextCookie] = {app, reason};
            if (delay)
                delayed.emplace(call, nextCookie);
            else
                call.reply(nextCookie);
        });
        object.addMethod("UnInhibit", "u", "", [this](MethodCall &call) {
            uint32_t cookie = 0;
            call.read(cookie);
            cookies.erase(cookie);
            call.reply();
        });
        object.publish();
        EXPECT_TRUE(connection->requestName("org.freedesktop.ScreenSaver"));
    }
};

class PortalTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        stopFrontend();
        stopPortal();
    }

    // The portal service as main() runs it, on a connection of its own;
    // `saved` writes lingmo-services' settings file first.
    void startPortal(std::optional<bool> saved = std::nullopt)
    {
        const std::string config = directory.path() + "/services.conf";
        if (saved)
            ASSERT_TRUE(base::file::writeAtomically(config, std::string("[Appearance]\nDarkMode=")
                                                                    + (*saved ? "true" : "false") + "\n"));
        portalBus = openConnection();
        ASSERT_TRUE(portalBus);
        settings = std::make_unique<SettingsPortal>(portalBus.get(), config);
        inhibit = std::make_unique<InhibitPortal>(portalBus.get());
        fileChooser = std::make_unique<FileChooserPortal>(portalBus.get());
        screenCast = std::make_unique<ScreenCastPortal>(portalBus.get());
        ASSERT_TRUE(settings->publish() && inhibit->publish() && fileChooser->publish() && screenCast->publish());
        // The previous test's connection may still be leaving the bus.
        ASSERT_TRUE(runUntil([&] { return portalBus->requestName(kService); }));
        runFor(20ms);
    }

    void stopPortal()
    {
        screenCast.reset();
        fileChooser.reset();
        inhibit.reset();
        settings.reset();
        portalBus.reset();
        runFor(10ms);
    }

    std::string startFrontend(const std::string &preferred)
    {
        const std::string configDirectory = directory.path() + "/xdg-desktop-portal";
        EXPECT_TRUE(base::file::writeAtomically(configDirectory + "/lingmo-portals.conf",
                                                "[preferred]\ndefault=none;\n" + preferred));
        std::vector<std::string> environment;
        for (char **variable = environ; *variable; ++variable) {
            const std::string entry = *variable;
            if (entry.rfind("XDG_CONFIG_HOME=", 0) && entry.rfind("XDG_CURRENT_DESKTOP=", 0)
                && entry.rfind("XDG_DESKTOP_PORTAL_DIR=", 0))
                environment.push_back(entry);
        }
        environment.push_back("XDG_CONFIG_HOME=" + directory.path());
        environment.push_back("XDG_CURRENT_DESKTOP=Lingmo");
        environment.push_back(std::string("XDG_DESKTOP_PORTAL_DIR=") + PORTAL_DATA_DIR);
        frontend = std::make_unique<base::Process>(g_loop);
        frontend->setEnvironment(environment);
        EXPECT_TRUE(frontend->start(kFrontend));
        const std::string service = "org.freedesktop.portal.Desktop";
        EXPECT_TRUE(runUntil([&] { return client.owned(service); }, 10s));
        return service;
    }

    void stopFrontend()
    {
        if (!frontend)
            return;
        frontend->terminate();
        frontend->waitForFinished(5s);
        frontend.reset();
    }

    uint32_t read(const std::string &service = kService, const std::string &interface = kSettings,
                  const std::string &method = "Read")
    {
        const Reply reply =
                client.callService(service, kPath, interface, method, kNamespace, std::string("color-scheme"));
        EXPECT_EQ(reply.message().signature(), "v") << reply.error().message;
        const std::optional<Variant> value = reply.value<Variant>();
        EXPECT_TRUE(value && value->signature() == "u");
        return value ? value->value<uint32_t>() : 0;
    }

    std::vector<base::dbus::Message> settingChanges()
    {
        base::dbus::MatchRule rule;
        rule.interface = kSettings;
        rule.member = "SettingChanged";
        events.clear();
        subscriptions.push_back(client.connection->subscribe(
                rule, [this](const base::dbus::Message &message) { events.push_back(message); }));
        // Behind the match on the same connection, so the bus has it.
        client.callService("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");
        return events;
    }

    Reply fileRequest(const std::string &method = "OpenFile", const VariantMap &options = {},
                      const std::string &handle = kRequest + "chooser")
    {
        return client.call("org.freedesktop.impl.portal.FileChooser", method, ObjectPath {handle},
                           std::string("org.example.Editor"), std::string(), std::string("Choose a file"), options);
    }

    base::file::TemporaryDirectory directory {"/tmp", "lingmo-portal-test"};
    Client client;
    std::unique_ptr<Connection> portalBus;
    std::unique_ptr<SettingsPortal> settings;
    std::unique_ptr<InhibitPortal> inhibit;
    std::unique_ptr<FileChooserPortal> fileChooser;
    std::unique_ptr<ScreenCastPortal> screenCast;
    std::unique_ptr<base::Process> frontend;
    std::vector<base::dbus::Message> events;
    std::vector<base::dbus::Match> subscriptions;
};

// Settings

TEST_F(PortalTest, DefaultLightAndVersion)
{
    startPortal();
    EXPECT_EQ(read(), 2u);
    const Reply version = client.call("org.freedesktop.DBus.Properties", "Get", kSettings, std::string("version"));
    const std::optional<Variant> value = version.value<Variant>();
    ASSERT_TRUE(value);
    EXPECT_EQ(value->signature(), "u");
    EXPECT_EQ(value->value<uint32_t>(), 1u);
}

TEST_F(PortalTest, SavedDarkWithoutService)
{
    startPortal(true);
    EXPECT_EQ(read(), 1u);
}

TEST_F(PortalTest, LivePropertyOverridesStaleFile)
{
    AppearanceService appearance;
    appearance.own(true);
    startPortal(false);
    EXPECT_TRUE(runUntil([&] { return read() == 1; }));
}

TEST_F(PortalTest, ReadAllFiltersAndTypes)
{
    startPortal();
    using Namespaces = std::vector<std::string>;
    for (const Namespaces &namespaces : {Namespaces {}, Namespaces {""}, Namespaces {"*"}, Namespaces {kNamespace},
                                         Namespaces {"org.*"}, Namespaces {"org.freedesktop.*"},
                                         Namespaces {"other", kNamespace}}) {
        const Reply reply = client.call(kSettings, "ReadAll", namespaces);
        EXPECT_EQ(reply.message().signature(), "a{sa{sv}}");
        const auto all = reply.value<std::map<std::string, VariantMap>>();
        ASSERT_TRUE(all);
        ASSERT_EQ(all->size(), 1u);
        EXPECT_EQ(all->at(kNamespace).at("color-scheme"), Variant(uint32_t(2)));
    }
    for (const std::string pattern :
         {"other", "org.freedesktop.appearance.*", "org.*.appearance", "org.freedesktop.appear*", "*appearance"}) {
        const Reply reply = client.call(kSettings, "ReadAll", Namespaces {pattern});
        const auto all = reply.value<std::map<std::string, VariantMap>>();
        ASSERT_TRUE(all);
        EXPECT_TRUE(all->empty()) << pattern;
    }
}

TEST_F(PortalTest, UnknownSettingsReturnNotFound)
{
    startPortal();
    for (const auto &[nameSpace, key] : {std::pair<std::string, std::string> {"unknown", "color-scheme"},
                                         std::pair<std::string, std::string> {kNamespace, "unknown"}}) {
        const Reply reply = client.call(kSettings, "Read", nameSpace, key);
        EXPECT_EQ(reply.error().name, "org.freedesktop.portal.Error.NotFound");
        EXPECT_EQ(reply.error().message, "Unknown setting: " + nameSpace + "/" + key);
    }
}

TEST_F(PortalTest, LiveSignalsAndDuplicateSuppression)
{
    AppearanceService appearance;
    appearance.own(false);
    startPortal();
    settingChanges();
    int index = 0;
    for (const bool dark : {true, false}) {
        appearance.change(dark);
        ++index;
        ASSERT_TRUE(runUntil([&] { return int(events.size()) == index; }));
        const base::dbus::Message &event = events.back();
        EXPECT_EQ(event.signature(), "ssv");
        std::string nameSpace;
        std::string key;
        Variant value;
        ASSERT_TRUE(event.read(nameSpace, key, value));
        EXPECT_EQ(nameSpace, kNamespace);
        EXPECT_EQ(key, "color-scheme");
        EXPECT_EQ(value, Variant(uint32_t(dark ? 1 : 2)));
        EXPECT_EQ(read(), dark ? 1u : 2u);
    }
    appearance.change(false);
    runFor(150ms);
    EXPECT_EQ(events.size(), 2u);
}

TEST_F(PortalTest, LateServiceAndRestart)
{
    startPortal();
    settingChanges();
    AppearanceService appearance;
    appearance.own(true);
    ASSERT_TRUE(runUntil([&] { return events.size() == 1; }));
    EXPECT_EQ(read(), 1u);
    appearance.release();
    EXPECT_EQ(read(), 1u);
    appearance.own(false);
    ASSERT_TRUE(runUntil([&] { return events.size() == 2; }));
    EXPECT_EQ(read(), 2u);
    appearance.change(true);
    ASSERT_TRUE(runUntil([&] { return events.size() == 3; }));
    EXPECT_EQ(read(), 1u);
}

TEST_F(PortalTest, SecondInstanceFails)
{
    startPortal();
    base::Process second(g_loop);
    ASSERT_TRUE(second.start(PORTAL_EXECUTABLE));
    std::optional<int> exitCode;
    second.onFinished = [&](int code, bool) { exitCode = code; };
    ASSERT_TRUE(runUntil([&] { return exitCode.has_value(); }));
    EXPECT_NE(*exitCode, 0);
    EXPECT_NE(second.standardError().find("Cannot own"), std::string::npos) << second.standardError();
    EXPECT_EQ(read(), 2u);
}

// File chooser

TEST_F(PortalTest, FileChooserFiltersChoicesAndEncodedUris)
{
    startPortal();
    ChooserService chooser(R"({"response": 0, "uris": ["file:///tmp/Hello%20world.txt"],)"
                           R"( "filter_index": 0, "choices": {"encoding": "utf8"}})");
    const Filter selected {"Text", {{0, "*.txt"}, {1, "text/plain"}}};
    using Choice = std::tuple<std::string, std::string, std::vector<std::tuple<std::string, std::string>>, std::string>;
    const Reply reply = fileRequest(
            "OpenFile",
            {{"filters", Variant(std::vector<Filter> {selected})},
             {"current_filter", Variant(selected)},
             {"choices", Variant(std::vector<Choice> {{"encoding", "Encoding", {{"utf8", "UTF-8"}}, "utf8"}})},
             {"current_folder", Variant(std::vector<uint8_t> {'/', 't', 'm', 'p', 0})},
             {"multiple", Variant(true)}});
    EXPECT_EQ(reply.message().signature(), "ua{sv}");
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(reply.read(response, results));
    EXPECT_EQ(response, 0u);
    EXPECT_EQ(results["uris"], Variant(std::vector<std::string> {"file:///tmp/Hello%20world.txt"}));
    EXPECT_EQ(results["current_filter"], Variant(selected));
    EXPECT_EQ(results["choices"],
              Variant(std::vector<std::tuple<std::string, std::string>> {{"encoding", "utf8"}}));
    const base::json::Value payload = chooser.request();
    EXPECT_EQ(chooser.calls[0].second[0], kRequest + "chooser");
    EXPECT_EQ(payload["mode"].toString(), "OpenFile");
    EXPECT_EQ(payload["current_folder"].toString(), "/tmp");
    EXPECT_EQ(payload["filters"][0]["rules"][1]["type"].toInt(), 1);
    EXPECT_EQ(payload["filters"][0]["rules"][1]["value"].toString(), "text/plain");
    EXPECT_TRUE(payload["multiple"].toBool());
    EXPECT_TRUE(payload["modal"].toBool());
    EXPECT_EQ(payload["filter_index"].toInt(), 0);
    // The keys arrive in the order the chooser has always had them in.
    EXPECT_EQ(base::json::Value::parse(chooser.calls[0].second[1])->toObject().front().first, "accept_label");
    EXPECT_EQ(read(), 2u);
}

TEST_F(PortalTest, SaveFilesPassesSuggestedNames)
{
    startPortal();
    ChooserService chooser(R"({"response": 0, "uris": ["file:///tmp/one.txt", "file:///tmp/two.txt"]})");
    const Reply reply = fileRequest(
            "SaveFiles",
            {{"files", Variant(std::vector<std::vector<uint8_t>> {{'o', 'n', 'e', '.', 't', 'x', 't', 0},
                                                                   {'t', 'w', 'o', '.', 't', 'x', 't', 0}})}});
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(reply.read(response, results));
    EXPECT_EQ(response, 0u);
    EXPECT_EQ(results["uris"].value<std::vector<std::string>>().size(), 2u);
    const base::json::Value files = chooser.request()["files"];
    ASSERT_EQ(files.size(), 2u);
    EXPECT_EQ(files[0].toString(), "one.txt");
    EXPECT_EQ(files[1].toString(), "two.txt");
}

TEST_F(PortalTest, FileChooserRejectsNonlocalResult)
{
    startPortal();
    ChooserService chooser(R"({"response": 0, "uris": ["https://example.com/file"]})");
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(fileRequest().read(response, results));
    EXPECT_EQ(response, 2u);
    EXPECT_TRUE(results.empty());
}

TEST_F(PortalTest, FileChooserWithoutFileManager)
{
    startPortal();
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(fileRequest().read(response, results));
    EXPECT_EQ(response, 2u);
    EXPECT_TRUE(results.empty());
    EXPECT_EQ(read(), 2u);
}

TEST_F(PortalTest, FileChooserCancellationAndRequestCleanup)
{
    startPortal();
    ChooserService chooser;
    const std::string handle = kRequest + "cancel";
    std::optional<Reply> reply;
    base::dbus::PendingCall pending =
            client.callAsync(reply, kService, kPath, "org.freedesktop.impl.portal.FileChooser", "OpenFile",
                             ObjectPath {handle}, std::string("org.example.App"), std::string(), std::string("Open"),
                             VariantMap());
    ASSERT_TRUE(runUntil([&] { return !chooser.pending.empty(); }));
    EXPECT_EQ(read(), 2u);
    EXPECT_FALSE(client.callAt(handle, "org.freedesktop.impl.portal.Request", "Close").isError());
    ASSERT_TRUE(runUntil([&] { return reply && chooser.calls.size() == 2; }));
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(reply->read(response, results));
    EXPECT_EQ(response, 1u);
    EXPECT_TRUE(results.empty());
    EXPECT_EQ(chooser.calls[1].first, "Close");
    EXPECT_EQ(chooser.calls[1].second, std::vector<std::string> {handle});
    runFor(10ms);
    EXPECT_TRUE(client.callAt(handle, "org.freedesktop.impl.portal.Request", "Close").isError());
    // The daemon's late cancellation reply finds the request gone.
    chooser.pending[0].reply(std::string(R"({"response":1})"));
    EXPECT_EQ(read(), 2u);
}

TEST_F(PortalTest, FileChooserRequestsNeedARequestHandle)
{
    startPortal();
    ChooserService chooser(R"({"response": 0, "uris": ["file:///tmp/a"]})");
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(fileRequest("OpenFile", {}, "/somewhere/else").read(response, results));
    EXPECT_EQ(response, 2u);
    EXPECT_TRUE(chooser.calls.empty());
}

TEST(FileUrlTest, NormalizesAsTheChooserAlwaysHas)
{
    const std::vector<std::pair<std::string, std::optional<std::string>>> cases {
            {"file:///tmp/Hello%20world.txt", "file:///tmp/Hello%20world.txt"},
            {"file:///tmp/Hello world.txt", "file:///tmp/Hello%20world.txt"},
            {"FILE:///tmp/a", "file:///tmp/a"},
            {"file://localhost/tmp/a", std::nullopt},
            {"file:///tmp/%41%7e%2F%25x", "file:///tmp/A~%2F%25x"},
            {"file:///tmp/a?b c#d e", "file:///tmp/a?b%20c#d%20e"},
            {"file:///tmp/\xe4\xbd\xa0.txt", "file:///tmp/%E4%BD%A0.txt"},
            {"file:///tmp/%e4%bd%a0.txt", "file:///tmp/%E4%BD%A0.txt"},
            {"file:///tmp/a<b>\"c\\d^e`f{g|h}i[j]k", "file:///tmp/a%3Cb%3E%22c%5Cd%5Ee%60f%7Bg%7Ch%7Di[j]k"},
            {"file:///tmp/!$&'()*+,;=:@~", "file:///tmp/!$&'()*+,;=:@~"},
            {"file:///tmp/%zz", "file:///tmp/%25zz"},
            {"file:/tmp/x", "file:///tmp/x"},
            {"file:tmp/x", std::nullopt},
            {"file:///tmp/a%", "file:///tmp/a%25"},
            {"file:///tmp/a\tb", "file:///tmp/a%09b"},
            {"file:///tmp//double/./dot/../x", "file:///tmp//double/./dot/../x"},
            {"file://", std::nullopt},
            {"file:///", "file:///"},
            {"https://example.com/file", std::nullopt},
            {"", std::nullopt},
    };
    for (const auto &[input, expected] : cases)
        EXPECT_EQ(normalizedFileUrl(input), expected) << input;
}

// Inhibit

class InhibitTest : public PortalTest
{
protected:
    void SetUp() override { startPortal(); }

    Reply inhibitCall(Client &from, const std::string &name, uint32_t flags = 8)
    {
        return from.call("org.freedesktop.impl.portal.Inhibit", "Inhibit", ObjectPath {kRequest + name},
                         std::string("org.test.Player"), std::string(), flags,
                         VariantMap {{"reason", Variant(std::string("Playing video"))}});
    }

    Reply inhibitCall(const std::string &name, uint32_t flags = 8) { return inhibitCall(client, name, flags); }

    Reply close(const std::string &name, Client *from = nullptr)
    {
        return (from ? *from : client).callAt(kRequest + name, "org.freedesktop.impl.portal.Request", "Close");
    }

    ScreenSaverService screenSaver;
};

TEST_F(InhibitTest, MultipleRequestsCloseAndDuplicatePath)
{
    const Reply first = inhibitCall("one");
    EXPECT_FALSE(first.isError()) << first.error().message;
    EXPECT_EQ(first.message().signature(), "");
    inhibitCall("two");
    ASSERT_EQ(screenSaver.cookies.size(), 2u);
    for (const auto &[cookie, entry] : screenSaver.cookies)
        EXPECT_EQ(entry, std::make_pair(std::string("org.test.Player"), std::string("Playing video")));
    const Reply duplicate = inhibitCall("one");
    EXPECT_EQ(duplicate.error().name, "org.freedesktop.portal.Error.Failed");
    close("one");
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.size() == 1; }));
    close("two");
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.empty(); }));
}

TEST_F(InhibitTest, FlagsAndSourceFailure)
{
    for (const uint32_t flags : {0u, 1u, 2u, 4u, 16u})
        EXPECT_EQ(inhibitCall("flags", flags).error().name, "org.freedesktop.portal.Error.NotSupported") << flags;
    EXPECT_TRUE(screenSaver.cookies.empty());
    const Reply bad = client.call("org.freedesktop.impl.portal.Inhibit", "Inhibit", ObjectPath {"/bad/path"},
                                  std::string("a"), std::string(), uint32_t(8), VariantMap());
    EXPECT_EQ(bad.error().name, "org.freedesktop.portal.Error.InvalidArgument");
    EXPECT_FALSE(inhibitCall("both", 12).isError());
    close("both");
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.empty(); }));
    screenSaver.fail = true;
    const Reply failed = inhibitCall("failure");
    EXPECT_EQ(failed.error().name, "org.freedesktop.DBus.Error.Failed");
    EXPECT_EQ(failed.error().message, "Unavailable");
    screenSaver.fail = false;
    EXPECT_FALSE(inhibitCall("failure").isError());
    close("failure");
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.empty(); }));
}

TEST_F(InhibitTest, WithoutScreenSaver)
{
    screenSaver.connection->releaseName("org.freedesktop.ScreenSaver");
    const Reply reply = inhibitCall("none");
    EXPECT_EQ(reply.error().name, "org.freedesktop.portal.Error.Failed");
    EXPECT_EQ(reply.error().message, "ScreenSaver inhibition service is unavailable");
    EXPECT_TRUE(close("none").isError());
}

TEST_F(InhibitTest, OwnerAndDisconnect)
{
    auto owner = std::make_unique<Client>();
    EXPECT_FALSE(inhibitCall(*owner, "owner").isError());
    const Reply denied = close("owner");
    EXPECT_EQ(denied.error().name, "org.freedesktop.DBus.Error.AccessDenied");
    EXPECT_EQ(screenSaver.cookies.size(), 1u);
    owner.reset();
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.empty(); }));
}

TEST_F(InhibitTest, CloseBeforeCookieArrives)
{
    screenSaver.delay = true;
    std::optional<Reply> reply;
    base::dbus::PendingCall pending = client.callAsync(
            reply, kService, kPath, "org.freedesktop.impl.portal.Inhibit", "Inhibit", ObjectPath {kRequest + "pending"},
            std::string("org.test.Player"), std::string(), uint32_t(8), VariantMap());
    ASSERT_TRUE(runUntil([&] { return screenSaver.delayed.has_value(); }));
    close("pending");
    ASSERT_TRUE(runUntil([&] { return reply.has_value(); }));
    EXPECT_EQ(reply->error().name, "org.freedesktop.portal.Error.Cancelled");
    screenSaver.delayed->first.reply(screenSaver.delayed->second);
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.empty(); }));
}

TEST_F(InhibitTest, CloseRightAfterInhibit)
{
    // Sent back to back, as the frontend does when the app closes at once.
    std::optional<Reply> inhibited;
    std::optional<Reply> closed;
    base::dbus::PendingCall first = client.callAsync(
            inhibited, kService, kPath, "org.freedesktop.impl.portal.Inhibit", "Inhibit",
            ObjectPath {kRequest + "early"}, std::string("org.test.Player"), std::string(), uint32_t(8),
            VariantMap());
    base::dbus::PendingCall second =
            client.callAsync(closed, kService, kRequest + "early", "org.freedesktop.impl.portal.Request", "Close");
    ASSERT_TRUE(runUntil([&] { return inhibited && closed; }));
    EXPECT_FALSE(closed->isError()) << closed->error().message;
    runFor(50ms);
    EXPECT_TRUE(screenSaver.cookies.empty());
}

TEST_F(InhibitTest, PublicFrontendInhibitAndClose)
{
    if (access(kFrontend.c_str(), X_OK) != 0)
        GTEST_SKIP() << "xdg-desktop-portal frontend is not installed";
    const std::string service = startFrontend("org.freedesktop.impl.portal.Inhibit=lingmo;\n");
    const Reply reply = client.callService(service, kPath, "org.freedesktop.portal.Inhibit", "Inhibit", std::string(),
                                           uint32_t(8),
                                           VariantMap {{"reason", Variant(std::string("Public portal test"))}});
    const std::optional<ObjectPath> handle = reply.value<ObjectPath>();
    ASSERT_TRUE(handle) << reply.error().message;
    ASSERT_TRUE(runUntil([&] { return screenSaver.cookies.size() == 1; }));
    EXPECT_EQ(screenSaver.cookies.begin()->second.second, "Public portal test");
    client.callService(service, handle->path, "org.freedesktop.portal.Request", "Close");
    EXPECT_TRUE(runUntil([&] { return screenSaver.cookies.empty(); }));
}

// Frontend

TEST_F(PortalTest, FrontendFileChooserResponse)
{
    if (access(kFrontend.c_str(), X_OK) != 0)
        GTEST_SKIP() << "xdg-desktop-portal frontend is not installed";
    const std::string selected = directory.path() + "/selected.txt";
    ASSERT_TRUE(base::file::writeAtomically(selected, "example"));
    startPortal();
    ChooserService chooser("{\"response\": 0, \"uris\": [\"" + base::url::fromLocalFile(selected) + "\"]}");
    const std::string service = startFrontend("org.freedesktop.impl.portal.Settings=lingmo;\n"
                                              "org.freedesktop.impl.portal.FileChooser=lingmo;\n");
    std::vector<base::dbus::Message> responses;
    base::dbus::MatchRule rule;
    rule.interface = "org.freedesktop.portal.Request";
    rule.member = "Response";
    base::dbus::Match match =
            client.connection->subscribe(rule, [&](const base::dbus::Message &message) { responses.push_back(message); });
    const Reply reply = client.callService(service, kPath, "org.freedesktop.portal.FileChooser", "OpenFile",
                                           std::string(), std::string("Open File"), VariantMap());
    EXPECT_EQ(reply.message().signature(), "o") << reply.error().message;
    ASSERT_TRUE(runUntil([&] { return !responses.empty(); }, 10s));
    uint32_t response = 9;
    VariantMap results;
    ASSERT_TRUE(responses[0].read(response, results));
    EXPECT_EQ(response, 0u);
    EXPECT_EQ(results["uris"].value<std::vector<std::string>>().size(), 1u);
}

TEST_F(PortalTest, FrontendForwardsReadsAndChanges)
{
    if (access(kFrontend.c_str(), X_OK) != 0)
        GTEST_SKIP() << "xdg-desktop-portal frontend is not installed";
    AppearanceService appearance;
    appearance.own(false);
    startPortal();
    const std::string service = startFrontend("org.freedesktop.impl.portal.Settings=lingmo;\n"
                                              "org.freedesktop.impl.portal.FileChooser=lingmo;\n");
    const std::string frontendInterface = "org.freedesktop.portal.Settings";
    EXPECT_EQ(read(service, frontendInterface, "ReadOne"), 2u);
    base::dbus::MatchRule rule;
    rule.interface = frontendInterface;
    rule.member = "SettingChanged";
    std::vector<base::dbus::Message> changes;
    base::dbus::Match match =
            client.connection->subscribe(rule, [&](const base::dbus::Message &message) { changes.push_back(message); });
    client.callService("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");
    appearance.change(true);
    ASSERT_TRUE(runUntil([&] { return !changes.empty(); }));
    std::string nameSpace;
    std::string key;
    Variant value;
    ASSERT_TRUE(changes.back().read(nameSpace, key, value));
    EXPECT_EQ(nameSpace, kNamespace);
    EXPECT_EQ(key, "color-scheme");
    EXPECT_EQ(value, Variant(uint32_t(1)));
    EXPECT_EQ(read(service, frontendInterface, "ReadOne"), 1u);
}

// Screen cast. No compositor or PipeWire is reachable here (run_session.sh
// gives the bus a runtime directory of its own), so Start can only fail
// cleanly; streaming is verified against a live session.

class ScreenCastTest : public PortalTest
{
protected:
    void SetUp() override
    {
        startPortal();
        base::dbus::MatchRule rule;
        rule.interface = kSessionInterface;
        rule.member = "Closed";
        subscriptions.push_back(client.connection->subscribe(
                rule, [this](const base::dbus::Message &message) { closed.push_back(message.path()); }));
        client.callService("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId");
    }

    static std::pair<uint32_t, VariantMap> result(const Reply &reply)
    {
        uint32_t response = 9;
        VariantMap results;
        EXPECT_TRUE(reply.read(response, results)) << reply.error().message;
        return {response, results};
    }

    Reply create(const std::string &name, Client *from = nullptr)
    {
        return (from ? *from : client)
                .call(kScreenCast, "CreateSession", ObjectPath {kRequest + name}, ObjectPath {kSession + name},
                      std::string("org.test.Recorder"), VariantMap());
    }

    Reply select(const std::string &name, const VariantMap &options, Client *from = nullptr)
    {
        return (from ? *from : client)
                .call(kScreenCast, "SelectSources", ObjectPath {kRequest + name}, ObjectPath {kSession + name},
                      std::string("org.test.Recorder"), options);
    }

    Reply start(const std::string &name)
    {
        return client.call(kScreenCast, "Start", ObjectPath {kRequest + name}, ObjectPath {kSession + name},
                           std::string("org.test.Recorder"), std::string(), VariantMap());
    }

    bool exists(const std::string &name)
    {
        return !client.callAt(kSession + name, "org.freedesktop.DBus.Properties", "Get", kSessionInterface,
                              std::string("version"))
                        .isError();
    }

    std::vector<std::string> closed;
};

TEST_F(ScreenCastTest, Properties)
{
    const Reply reply = client.call("org.freedesktop.DBus.Properties", "GetAll", kScreenCast);
    const auto values = reply.value<VariantMap>();
    ASSERT_TRUE(values);
    EXPECT_EQ(values->at("version"), Variant(uint32_t(4)));
    EXPECT_EQ(values->at("AvailableSourceTypes"), Variant(uint32_t(3)));
    EXPECT_EQ(values->at("AvailableCursorModes"), Variant(uint32_t(3)));
}

TEST_F(ScreenCastTest, SessionLifecycle)
{
    const auto [response, results] = result(create("one"));
    EXPECT_EQ(response, 0u);
    EXPECT_EQ(results.at("session_id"), Variant(std::string("one")));
    const Reply version = client.callAt(kSession + "one", "org.freedesktop.DBus.Properties", "Get",
                                        kSessionInterface, std::string("version"));
    EXPECT_EQ(version.value<Variant>(), Variant(uint32_t(1)));
    // The same handle cannot be created twice, and bad paths are refused.
    EXPECT_EQ(result(create("one")).first, 2u);
    EXPECT_EQ(result(client.call(kScreenCast, "CreateSession", ObjectPath {kRequest + "x"},
                                 ObjectPath {"/somewhere/else"}, std::string("org.test.Recorder"), VariantMap()))
                      .first,
              2u);

    Client other;
    EXPECT_EQ(other.callAt(kSession + "one", kSessionInterface, "Close").error().name,
              "org.freedesktop.DBus.Error.AccessDenied");
    EXPECT_TRUE(exists("one"));

    EXPECT_FALSE(client.callAt(kSession + "one", kSessionInterface, "Close").isError());
    runFor(10ms);
    EXPECT_FALSE(exists("one"));
    EXPECT_TRUE(closed.empty());
}

TEST_F(ScreenCastTest, OwnerDisconnectFreesSession)
{
    auto owner = std::make_unique<Client>();
    EXPECT_EQ(result(create("gone", owner.get())).first, 0u);
    EXPECT_TRUE(exists("gone"));
    owner.reset();
    EXPECT_TRUE(runUntil([&] { return !exists("gone"); }));
}

TEST_F(ScreenCastTest, SelectSourcesValidation)
{
    create("types");
    // Virtual sources alone are refused; monitors or windows are fine.
    EXPECT_EQ(result(select("types", {{"types", Variant(uint32_t(4))}})).first, 2u);
    ASSERT_TRUE(runUntil([&] { return closed == std::vector<std::string> {kSession + "types"}; }));
    runFor(10ms);
    EXPECT_FALSE(exists("types"));
    create("windows");
    EXPECT_EQ(result(select("windows", {{"types", Variant(uint32_t(2))}})).first, 0u);

    create("cursor");
    EXPECT_EQ(result(select("cursor", {{"cursor_mode", Variant(uint32_t(4))}})).first, 2u);
    ASSERT_TRUE(runUntil([&] { return closed.size() == 2 && closed[1] == kSession + "cursor"; }));

    create("ok");
    const VariantMap restore {{"monitors", Variant(std::vector<std::string> {"eDP-1"})}};
    EXPECT_EQ(result(select("ok", {{"types", Variant(uint32_t(3))},
                                   {"cursor_mode", Variant(uint32_t(2))},
                                   {"persist_mode", Variant(uint32_t(2))},
                                   {"restore_data", Variant(std::tuple<std::string, uint32_t, Variant>(
                                                            "Lingmo", 1, Variant(restore)))}}))
                      .first,
              0u);
    EXPECT_TRUE(exists("ok"));
    // Another connection may not drive this session.
    Client other;
    EXPECT_EQ(result(select("ok", {}, &other)).first, 2u);
}

TEST_F(ScreenCastTest, StartWithoutCompositorFailsCleanly)
{
    create("start");
    select("start", {});
    EXPECT_EQ(result(start("start")).first, 2u);
    // The request path is released again, and the session survives for the
    // frontend to close.
    runFor(10ms);
    EXPECT_TRUE(client.callAt(kRequest + "start", "org.freedesktop.impl.portal.Request", "Close").isError());
    EXPECT_TRUE(exists("start"));
    EXPECT_EQ(result(start("unknown")).first, 2u);
}

int main(int argc, char **argv)
{
    base::EventLoop loop;
    g_loop = &loop;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
