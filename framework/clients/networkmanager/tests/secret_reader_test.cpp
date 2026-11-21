// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"
#include "lingmo/networkmanager/secret_reader.h"

using namespace std::chrono_literals;
using namespace Lingmo::Base;
using Lingmo::networkmanager::SecretReader;

using Paths = std::vector<dbus::ObjectPath>;

static EventLoop* loop = nullptr;

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

// A keyring whose only item is locked, and whose unlock prompt the user dismisses.
class FakeSecretService
{
public:
    FakeSecretService()
        : m_bus(dbus::Connection::open(dbus::Connection::Bus::Session))
        , m_service(m_bus.get(), "/org/freedesktop/secrets", "org.freedesktop.Secret.Service")
        , m_prompt(m_bus.get(), "/prompt/1", "org.freedesktop.Secret.Prompt")
    {
        m_service.addMethod("OpenSession", [](const std::string&, const dbus::Variant&) {
            return std::tuple<dbus::Variant, dbus::ObjectPath>{dbus::Variant(std::string()),
                                                               dbus::ObjectPath{"/session/1"}};
        });
        m_service.addMethod("SearchItems", [](const std::map<std::string, std::string>&) {
            return std::tuple<Paths, Paths>{Paths(), Paths{dbus::ObjectPath{"/item/1"}}};
        });
        m_service.addMethod("Unlock", [](const Paths&) {
            return std::tuple<Paths, dbus::ObjectPath>{Paths(), dbus::ObjectPath{"/prompt/1"}};
        });
        m_service.publish();

        m_prompt.addMethod("Prompt", [this](const std::string&) {
            ++prompts;
            m_bus->emitSignal("/prompt/1", "org.freedesktop.Secret.Prompt", "Completed", true, dbus::Variant(Paths()));
        });
        m_prompt.addSignal("Completed", "bv");
        m_prompt.publish();
    }

    bool serve() { return m_bus->requestName("org.freedesktop.secrets"); }

    int prompts = 0;

private:
    std::unique_ptr<dbus::Connection> m_bus;
    dbus::Object m_service;
    dbus::Object m_prompt;
};

TEST(SecretReaderTest, DismissedPromptAnswersWithNothing)
{
    FakeSecretService service;
    ASSERT_TRUE(service.serve());

    std::optional<std::map<std::string, std::string>> loaded;
    {
        SecretReader reader;
        reader.load("uuid", "vpn", [&](const std::map<std::string, std::string>& secrets) { loaded = secrets; });
        ASSERT_TRUE(waitFor([&] { return loaded.has_value(); }));
    }

    EXPECT_EQ(service.prompts, 1);
    EXPECT_TRUE(loaded->empty());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    EventLoop eventLoop;
    loop = &eventLoop;
    return RUN_ALL_TESTS();
}
