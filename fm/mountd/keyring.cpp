// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyring.h"

#include "base/dbus.h"
#include "base/event_loop.h"

#include <map>

using namespace Lingmo::base;

static const char s_service[] = "org.freedesktop.secrets";
static const char s_servicePath[] = "/org/freedesktop/secrets";
static const char s_serviceInterface[] = "org.freedesktop.Secret.Service";
static const char s_schema[] = "org.lingmo.ServerPassword";
static constexpr std::chrono::seconds kTimeout(5);

using Attributes = std::map<std::string, std::string>;
// (session, parameters, value, content type)
using Secret = std::tuple<dbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>;

namespace {

// A connection of the calling thread's own, which the connecting threads block on.
struct Bus
{
    EventLoop loop;
    std::unique_ptr<dbus::Connection> connection = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);

    template<typename... Args>
    dbus::Reply call(const std::string &path, const std::string &interface, const std::string &method,
                     const Args &...args)
    {
        if (!connection)
            return {};
        dbus::Message message = connection->newMethodCall(s_service, path, interface, method);
        message.append(args...);
        return connection->call(message, kTimeout);
    }
};

} // namespace

static std::string openSession(Bus &bus)
{
    const dbus::Reply reply = bus.call(s_servicePath, s_serviceInterface, "OpenSession", std::string("plain"),
                                       dbus::Variant(std::string()));
    const auto session = reply.value<dbus::ObjectPath>(1);
    return session ? session->path : std::string();
}

static void closeSession(Bus &bus, const std::string &session)
{
    bus.call(session, "org.freedesktop.Secret.Session", "Close");
}

static Attributes attributesFor(const RemoteUrl &url)
{
    Attributes attributes {
        {"xdg:schema", s_schema},
        {"protocol", url.scheme},
        {"server", url.host},
    };
    if (url.port > 0)
        attributes["port"] = std::to_string(url.port);

    return attributes;
}

bool Keyring::find(const RemoteUrl &url, Credentials *credentials)
{
    Bus bus;
    if (!bus.connection)
        return false;

    Attributes attributes = attributesFor(url);
    const std::string user = !credentials->user.empty() ? credentials->user : url.user;
    if (!user.empty())
        attributes["user"] = user;

    const auto items = bus.call(s_servicePath, s_serviceInterface, "SearchItems", attributes)
                               .value<std::vector<dbus::ObjectPath>>(0);
    if (!items || items->empty())
        return false;

    const std::string session = openSession(bus);
    if (session.empty())
        return false;

    const std::string item = items->front().path;
    const dbus::Reply secretReply =
            bus.call(item, "org.freedesktop.Secret.Item", "GetSecret", dbus::ObjectPath{session});
    const dbus::Reply attributesReply = bus.call(item, "org.freedesktop.DBus.Properties", "Get",
                                                 std::string("org.freedesktop.Secret.Item"),
                                                 std::string("Attributes"));
    closeSession(bus, session);

    const auto secret = secretReply.value<Secret>(0);
    if (!secret)
        return false;

    Attributes stored;
    if (const auto value = attributesReply.value<dbus::Variant>(0))
        stored = value->value<Attributes>();

    const auto storedUser = stored.find("user");
    credentials->user = storedUser != stored.end() ? storedUser->second : user;
    credentials->domain = stored.contains("domain") ? stored["domain"] : std::string();
    const std::vector<uint8_t> &value = std::get<2>(*secret);
    credentials->password.assign(value.begin(), value.end());
    return true;
}

void Keyring::store(const RemoteUrl &url, const Credentials &credentials)
{
    Bus bus;
    if (!bus.connection)
        return;

    const auto collection =
            bus.call(s_servicePath, s_serviceInterface, "ReadAlias", std::string("default")).value<dbus::ObjectPath>(0);
    if (!collection || collection->path == "/")
        return;

    const std::string session = openSession(bus);
    if (session.empty())
        return;

    Attributes attributes = attributesFor(url);
    attributes["user"] = credentials.user;
    if (!credentials.domain.empty())
        attributes["domain"] = credentials.domain;

    const std::string account =
            credentials.domain.empty() ? credentials.user : credentials.domain + '\\' + credentials.user;
    const dbus::VariantMap properties {
        {"org.freedesktop.Secret.Item.Label", dbus::Variant(account + " on " + url.host)},
        {"org.freedesktop.Secret.Item.Attributes", dbus::Variant(attributes)},
    };
    const Secret secret {dbus::ObjectPath{session}, {},
                         std::vector<uint8_t>(credentials.password.begin(), credentials.password.end()),
                         std::string("text/plain")};

    // A locked collection answers with a prompt, which is left unanswered.
    bus.call(collection->path, "org.freedesktop.Secret.Collection", "CreateItem", properties, secret, true);
    closeSession(bus, session);
}
