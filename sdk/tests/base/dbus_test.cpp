// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <future>
#include <string>
#include <thread>
#include <unistd.h>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"

using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace std::chrono_literals;

// Names of this process's own: ctest runs each test in a process of its own,
// several at once on one bus.
static const std::string kService = "org.lingmo.ToolkitTest.p" + std::to_string(getpid());
static constexpr const char* kPath = "/Test";
static constexpr const char* kInterface = "org.lingmo.ToolkitTest";

TEST(DBusTypesTest, Signatures)
{
    EXPECT_EQ(Type<VariantMap>::signature(), "a{sv}");
    EXPECT_EQ((Type<std::map<ObjectPath, std::map<std::string, VariantMap>>>::signature()), "a{oa{sa{sv}}}");
    EXPECT_EQ((Type<std::tuple<std::string, int32_t, std::vector<uint8_t>>>::signature()), "(siay)");
    EXPECT_EQ((signatureOf<bool, uint32_t, const char*, ObjectPath>()), "buso");
}

TEST(DBusTypesTest, VariantsConvertBack)
{
    const VariantMap map{{"Name", std::string("Office")}, {"Strength", uint8_t(72)},
                         {"Ssid", std::vector<uint8_t>{'a', 'b'}}, {"Nested", Type<Variant>::toVariant(int32_t(5))}};
    const Variant variant(map);
    EXPECT_EQ(variant.signature(), "a{sv}");
    const auto back = variant.get<VariantMap>();
    ASSERT_TRUE(back);
    EXPECT_EQ(back->at("Name").get<std::string>(), "Office");
    EXPECT_EQ(back->at("Strength").get<uint8_t>(), 72);
    EXPECT_EQ(back->at("Ssid").bytes(), "ab");
    // Type mismatches give nothing rather than a converted value.
    EXPECT_FALSE(back->at("Strength").get<uint32_t>());
    EXPECT_EQ(back->at("Nested").get<Variant>()->get<int32_t>(), 5);
    EXPECT_EQ(*back, map);
}

namespace {

// A service on a thread of its own, so the test thread can block on calls.
class TestService
{
public:
    TestService()
    {
        std::promise<bool> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        m_ready = ready.get_future().get();
    }
    ~TestService()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    bool isReady() const { return m_ready; }
    // Why it is not ready.
    const std::string& failure() const { return m_failure; }
    void releaseName()
    {
        std::promise<void> done;
        m_handle.post([this, &done] {
            m_connection->releaseName(kService);
            done.set_value();
        });
        done.get_future().get();
    }

private:
    void run(std::promise<bool>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<Connection> connection = Connection::open(Connection::Bus::Session, &loop);
        if (!connection) {
            m_failure = "cannot connect to the session bus";
            ready.set_value(false);
            return;
        }
        if (!connection->requestName(kService)) {
            m_failure = "cannot own " + kService;
            ready.set_value(false);
            return;
        }
        m_connection = connection.get();

        Object object(connection.get(), kPath, kInterface);
        object.addMethod("Echo", [](const std::string& text, int32_t number) {
            return std::make_tuple(text + "!", number * 2);
        });
        object.addMethod("Sum", [](const std::vector<int32_t>& numbers) {
            int64_t sum = 0;
            for (const int32_t number : numbers)
                sum += number;
            return sum;
        });
        object.addMethod("EchoMap", [](const VariantMap& map) { return map; });
        object.addMethod("Fail", "", "", [](MethodCall& call) {
            call.replyError("org.lingmo.ToolkitTest.Error.Failed", "As asked");
        });
        object.addMethod("Later", "u", "u", [&loop](MethodCall& call) {
            uint32_t value = 0;
            call.read(value);
            loop.startTimer(20ms, false, [call, value]() mutable { call.reply(value + 1); });
        });
        object.addMethod("ReadFd", [](const UnixFd& fd) {
            char buffer[16] = {};
            const ssize_t size = ::read(fd.fd(), buffer, sizeof(buffer) - 1);
            return std::string(buffer, size > 0 ? size_t(size) : 0);
        });
        object.addMethod("Emit", [&object](const std::string& text) { object.emitSignal("Changed", text); });
        object.addMethod("Interactive", "", "b", [](MethodCall& call) {
            call.reply(call.message().allowsInteractiveAuthorization());
        });
        object.addProperty("Name", [] { return std::string("Test"); });
        object.addProperty("Count", [this] { return m_count; }, [this, &object](int32_t count) {
            if (count < 0)
                return false;
            m_count = count;
            object.emitPropertiesChanged({"Count"});
            return true;
        });
        object.addSignal("Changed", "s");
        object.addMethod(
                "Named",
                [](const std::string& text, int32_t count) { return std::tuple<std::string, bool>(text, count > 0); },
                ArgumentNames{{"text", "count"}, {"result", "ok"}});
        object.addMethod("NamedRaw", "a{sv}(ii)", "", [](MethodCall& call) { call.reply(); },
                         ArgumentNames{{"options", "pair"}, {}});
        object.addMethod("Mismatched", "ss", "", [](MethodCall& call) { call.reply(); },
                         ArgumentNames{{"only"}, {}});
        object.addSignal("NamedChanged", "sa(ii)", {"name", "items"});
        if (!object.publish())
            m_failure = "cannot publish " + std::string(kPath);
        ready.set_value(m_failure.empty());
        loop.run();
        m_connection = nullptr;
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    Connection* m_connection = nullptr;
    int32_t m_count = 3;
    bool m_ready = false;
    std::string m_failure;
};

class DBusTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_connection = Connection::open(Connection::Bus::Session, &m_loop);
        if (!m_connection || m_connection->uniqueName().empty())
            GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";
        m_service = std::make_unique<TestService>();
        ASSERT_TRUE(m_service->isReady()) << m_service->failure();
        m_proxy = Proxy(m_connection.get(), kService, kPath, kInterface);
    }

    // Runs the loop until quit or the time is up.
    // Runs the loop until the condition holds, checked every 10 ms, or
    // the time is up.
    void runUntil(const std::function<bool()>& done, std::chrono::milliseconds limit = 5s)
    {
        const auto end = std::chrono::steady_clock::now() + limit;
        while (!done() && std::chrono::steady_clock::now() < end)
            runFor(10ms);
    }

    void runFor(std::chrono::milliseconds limit)
    {
        const int timer = m_loop.startTimer(limit, false, [this] { m_loop.quit(); });
        m_loop.run();
        m_loop.stopTimer(timer);
    }

    EventLoop m_loop;
    std::unique_ptr<Connection> m_connection;
    std::unique_ptr<TestService> m_service;
    Proxy m_proxy;
};

} // namespace

TEST_F(DBusTest, CallsWithTypedArguments)
{
    const Reply echo = m_proxy.call("Echo", "hi", int32_t(21));
    ASSERT_FALSE(echo.isError()) << echo.error().message;
    std::string text;
    int32_t number = 0;
    ASSERT_TRUE(echo.read(text, number));
    EXPECT_EQ(text, "hi!");
    EXPECT_EQ(number, 42);

    const VariantMap map{{"Id", std::string("Home")}, {"Ports", std::vector<uint16_t>{80, 443}}};
    EXPECT_EQ(m_proxy.call("EchoMap", map).value<VariantMap>(), map);

    const Reply wrong = m_proxy.call("Echo", int32_t(1));
    EXPECT_TRUE(wrong.isError());
}

TEST_F(DBusTest, AsyncCallsAndErrors)
{
    int64_t sum = 0;
    Error error;
    uint32_t later = 0;
    int replies = 0;
    auto done = [&] {
        if (++replies == 3)
            m_loop.quit();
    };
    PendingCall first = m_proxy.callAsync("Sum", [&](const Reply& reply) {
        sum = reply.value<int64_t>().value_or(-1);
        done();
    }, std::vector<int32_t>{1, 2, 3});
    PendingCall second = m_proxy.callAsync("Fail", [&](const Reply& reply) {
        error = reply.error();
        done();
    });
    PendingCall third = m_proxy.callAsync("Later", [&](const Reply& reply) {
        later = reply.value<uint32_t>().value_or(0);
        done();
    }, uint32_t(9));
    runFor(2s);
    EXPECT_EQ(sum, 6);
    EXPECT_EQ(error.name, "org.lingmo.ToolkitTest.Error.Failed");
    EXPECT_EQ(error.message, "As asked");
    EXPECT_EQ(later, 10u);
}

TEST_F(DBusTest, DroppedPendingCallNeverCallsBack)
{
    bool called = false;
    {
        PendingCall call = m_proxy.callAsync("Later", [&](const Reply&) { called = true; }, uint32_t(1));
    }
    runFor(100ms);
    EXPECT_FALSE(called);
}

TEST_F(DBusTest, PropertiesAndTheirChanges)
{
    EXPECT_EQ(m_proxy.property<std::string>("Name"), "Test");
    EXPECT_EQ(m_proxy.property<int32_t>("Count"), 3);
    const auto all = m_proxy.getAll().value<VariantMap>();
    ASSERT_TRUE(all);
    EXPECT_EQ(all->size(), 2u);

    VariantMap changed;
    Match match = m_proxy.onPropertiesChanged([&](const VariantMap& properties, const std::vector<std::string>&) {
        changed = properties;
        m_loop.quit();
    });
    // The match is added asynchronously; a round trip makes sure it is in.
    EXPECT_TRUE(m_proxy.call("Echo", "", int32_t(0)));
    EXPECT_FALSE(m_proxy.setProperty("Count", int32_t(7)).isError());
    EXPECT_TRUE(m_proxy.setProperty("Count", int32_t(-1)).isError());
    EXPECT_TRUE(m_proxy.setProperty("Name", std::string("x")).isError());
    runFor(2s);
    ASSERT_EQ(changed.count("Count"), 1u);
    EXPECT_EQ(changed.at("Count").get<int32_t>(), 7);
}

TEST_F(DBusTest, SignalsReachSubscribers)
{
    std::string received;
    Match match = m_proxy.onSignal("Changed", [&](const Message& signal) {
        received = signal.argument<std::string>().value_or("");
        m_loop.quit();
    });
    EXPECT_TRUE(m_proxy.call("Echo", "", int32_t(0)));
    EXPECT_TRUE(m_proxy.call("Emit", "news"));
    runFor(2s);
    EXPECT_EQ(received, "news");

    received.clear();
    match.reset();
    EXPECT_TRUE(m_proxy.call("Emit", "again"));
    runFor(100ms);
    EXPECT_TRUE(received.empty());
}

TEST_F(DBusTest, InteractiveAuthorizationReachesTheService)
{
    EXPECT_EQ(m_proxy.call("Interactive").value<bool>(), false);
    Proxy interactive = m_proxy;
    interactive.setInteractiveAuthorizationAllowed(true);
    interactive.setTimeout(std::chrono::seconds(120));
    EXPECT_EQ(interactive.call("Interactive").value<bool>(), true);
    bool asynchronous = false;
    PendingCall call = interactive.callAsync("Interactive", [&](const Reply& reply) {
        asynchronous = reply.value<bool>().value_or(false);
        m_loop.quit();
    });
    runFor(2s);
    EXPECT_TRUE(asynchronous);
}

TEST_F(DBusTest, FileDescriptorsPass)
{
    int pipe[2];
    ASSERT_EQ(::pipe(pipe), 0);
    ASSERT_EQ(::write(pipe[1], "piped", 5), 5);
    close(pipe[1]);
    const Reply reply = m_proxy.call("ReadFd", UnixFd::adopt(pipe[0]));
    EXPECT_EQ(reply.value<std::string>(), "piped");
}

TEST_F(DBusTest, NamesAreWatchedAndOwnedOnce)
{
    EXPECT_FALSE(m_connection->requestName(kService));

    std::vector<std::string> owners;
    NameWatch watch = m_connection->watchName(kService, [&](const std::string& owner) {
        owners.push_back(owner);
        m_loop.quit();
    });
    runFor(2s);
    ASSERT_EQ(owners.size(), 1u);
    EXPECT_FALSE(owners[0].empty());

    m_service->releaseName();
    runFor(2s);
    ASSERT_EQ(owners.size(), 2u);
    EXPECT_TRUE(owners[1].empty());

    // Free again, so this connection takes it; asking again keeps it.
    EXPECT_TRUE(m_connection->requestName(kService));
    EXPECT_TRUE(m_connection->requestName(kService));
}

TEST_F(DBusTest, SharedConnectionLivesWithItsLoop)
{
    Connection* shared = nullptr;
    {
        EventLoop loop;
        shared = Connection::session(&loop);
        ASSERT_NE(shared, nullptr);
        EXPECT_EQ(Connection::session(&loop), shared);
        Proxy proxy(shared, kService, kPath, kInterface);
        EXPECT_EQ(proxy.call("Echo", "a", int32_t(1)).value<std::string>(), "a!");
    }
    EXPECT_NE(Connection::session(&m_loop), nullptr);
}

TEST_F(DBusTest, IntrospectionCarriesArgumentNames)
{
    Proxy introspectable(m_connection.get(), kService, kPath, "org.freedesktop.DBus.Introspectable");
    const std::optional<std::string> xml = introspectable.call("Introspect").value<std::string>();
    ASSERT_TRUE(xml);
    const auto has = [&xml](const std::string& text) { return xml->find(text) != std::string::npos; };
    EXPECT_TRUE(has(R"x(<arg type="s" name="text" direction="in"/>)x")) << *xml;
    EXPECT_TRUE(has(R"x(<arg type="i" name="count" direction="in"/>)x"));
    EXPECT_TRUE(has(R"x(<arg type="s" name="result" direction="out"/>)x"));
    EXPECT_TRUE(has(R"x(<arg type="b" name="ok" direction="out"/>)x"));
    EXPECT_TRUE(has(R"x(<arg type="a{sv}" name="options" direction="in"/>)x"));
    EXPECT_TRUE(has(R"x(<arg type="(ii)" name="pair" direction="in"/>)x"));
    EXPECT_TRUE(has(R"x(<arg type="s" name="name"/>)x"));
    EXPECT_TRUE(has(R"x(<arg type="a(ii)" name="items"/>)x"));
    // A list that does not match its signature is dropped.
    EXPECT_FALSE(has(R"x(name="only")x"));
    // Members without names still work.
    EXPECT_TRUE(has(R"x(<method name="Echo">)x"));

    const Reply named = m_proxy.call("Named", "x", int32_t(2));
    std::string text;
    bool ok = false;
    ASSERT_TRUE(named.read(text, ok));
    EXPECT_EQ(text, "x");
    EXPECT_TRUE(ok);
}

// Two objects at one path under two names: each proxy hears its own.
TEST_F(DBusTest, ProxySignalsComeFromTheirNamesOwner)
{
    const std::string kOther = kService + ".Other";
    std::unique_ptr<Connection> other = Connection::open(Connection::Bus::Session, &m_loop);
    ASSERT_TRUE(other);
    ASSERT_TRUE(other->requestName(kOther, Connection::DoNotQueue));
    Proxy otherProxy(m_connection.get(), kOther, kPath, kInterface);

    std::vector<std::string> mine;
    std::vector<std::string> theirs;
    Match ours = m_proxy.onSignal("Changed", [&](const Message& signal) {
        mine.push_back(signal.argument<std::string>().value_or(""));
    });
    Match others = otherProxy.onSignal("Changed", [&](const Message& signal) {
        theirs.push_back(signal.argument<std::string>().value_or(""));
    });
    // The matches are added asynchronously, and a signal from another
    // connection may reach the bus first; the bus handles one connection's
    // messages in order, so they are in place once a later call returns.
    Proxy bus(m_connection.get(), "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    ASSERT_TRUE(bus.call("GetId"));
    // Signals sent before the owners are known wait for them.
    EXPECT_TRUE(other->emitSignal(kPath, kInterface, "Changed", std::string("early")));
    runFor(300ms);
    EXPECT_TRUE(other->emitSignal(kPath, kInterface, "Changed", std::string("other")));
    runUntil([&] { return theirs.size() >= 2; });
    EXPECT_TRUE(mine.empty());
    EXPECT_EQ(theirs, (std::vector<std::string>{"early", "other"}));

    EXPECT_TRUE(m_proxy.call("Emit", "own"));
    runUntil([&] { return !mine.empty(); });
    EXPECT_EQ(mine, std::vector<std::string>{"own"});
    EXPECT_EQ(theirs.size(), 2u);

    // Properties too.
    int changes = 0;
    Match properties = otherProxy.onPropertiesChanged([&](const VariantMap&, const std::vector<std::string>&) {
        ++changes;
    });
    runFor(100ms);
    EXPECT_FALSE(m_proxy.setProperty("Count", int32_t(9)).isError());
    runFor(300ms);
    EXPECT_EQ(changes, 0);
}

TEST_F(DBusTest, NamesAreTakenOverWhenAllowed)
{
    const std::string kName = kService + ".Replaceable";
    std::unique_ptr<Connection> first = Connection::open(Connection::Bus::Session, &m_loop);
    ASSERT_TRUE(first->requestName(kName, Connection::AllowReplacement | Connection::DoNotQueue));
    // Replacing the owner, which does not allow replacement in turn.
    EXPECT_TRUE(m_connection->requestName(kName, Connection::ReplaceExisting | Connection::DoNotQueue));
    // Not given up again.
    EXPECT_FALSE(first->requestName(kName, Connection::ReplaceExisting | Connection::DoNotQueue));
    // Asked again by the owner: still its.
    EXPECT_TRUE(m_connection->requestName(kName, Connection::DoNotQueue));
}

// An object destroyed or published again from one of its own handlers: the
// call that did it is answered, the calls right behind it no longer find the
// old members, and the new table is there by the next call.
TEST_F(DBusTest, ObjectsEndFromTheirOwnHandlers)
{
    std::unique_ptr<Connection> service = Connection::open(Connection::Bus::Session, &m_loop);
    ASSERT_TRUE(service);
    auto doomed = std::make_unique<Object>(service.get(), "/Doomed", kInterface);
    doomed->addMethod("Close", [&doomed] { doomed.reset(); });
    doomed->addProperty("Name", [] { return std::string("Doomed"); });
    ASSERT_TRUE(doomed->publish());

    Object renamed(service.get(), "/Renamed", kInterface);
    renamed.addMethod("Rename", [&renamed] {
        renamed.addMethod("Renamed", [] { return true; });
        renamed.publish();
    });
    ASSERT_TRUE(renamed.publish());

    Proxy doomedProxy(m_connection.get(), service->uniqueName(), "/Doomed", kInterface);
    Proxy renamedProxy(m_connection.get(), service->uniqueName(), "/Renamed", kInterface);
    std::vector<std::string> results;
    const auto record = [&](const Reply& reply) {
        results.push_back(reply.isError() ? reply.error().name : "ok");
        if (results.size() == 5)
            m_loop.quit();
    };
    std::vector<PendingCall> calls;
    calls.push_back(doomedProxy.callAsync("Close", record));
    calls.push_back(doomedProxy.callAsync("Close", record));
    calls.push_back(doomedProxy.getPropertyAsync("Name", [&](const Reply& reply) { record(reply); }));
    // Published again from the handler: there once the handler returned and
    // the loop turned.
    calls.push_back(renamedProxy.callAsync("Rename", [&](const Reply& reply) {
        record(reply);
        calls.push_back(renamedProxy.callAsync("Renamed", record));
    }));
    runFor(2s);
    ASSERT_EQ(results.size(), 5u);
    EXPECT_EQ(doomed, nullptr);
    EXPECT_EQ(results[0], "ok");
    EXPECT_NE(results[1], "ok");
    EXPECT_NE(results[2], "ok");
    EXPECT_EQ(results[3], "ok");
    EXPECT_EQ(results[4], "ok");
}
