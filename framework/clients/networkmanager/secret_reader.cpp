// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/secret_reader.h"
#include "lingmo/networkmanager/variants.h"

#include <chrono>

#include "base/timer.h"

namespace Lingmo::Networkmanager {

using namespace std::chrono_literals;
using base::dbus::Message;
using base::dbus::ObjectPath;
using base::dbus::Variant;

static const std::string service = "org.freedesktop.secrets";
static const std::string servicePath = "/org/freedesktop/secrets";
static const std::string serviceInterface = "org.freedesktop.Secret.Service";
static const std::string itemInterface = "org.freedesktop.Secret.Item";
static const std::string promptInterface = "org.freedesktop.Secret.Prompt";
static const std::string sessionInterface = "org.freedesktop.Secret.Session";
static const std::string propertiesInterface = "org.freedesktop.DBus.Properties";

// Completed arrives on the prompt object after the user answered it.
static constexpr auto kPromptTimeout = 10min;

using Secret = std::tuple<ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>;

SecretReader::SecretReader()
    : m_bus(base::dbus::Connection::session())
{
}

SecretReader::~SecretReader() = default;

void SecretReader::call(const std::string& path, const std::string& interface, const std::string& method,
                        Variant::Items arguments, Reply done)
{
    if (!m_bus) {
        done(base::dbus::Reply({}, {"org.freedesktop.DBus.Error.Disconnected", "There is no session bus"}));
        return;
    }
    Message message = m_bus->newMethodCall(service, path, interface, method);
    for (const Variant& argument : arguments)
        message.appendAs(argument);
    const uint64_t id = ++m_nextId;
    auto finished = std::make_shared<bool>(false);
    base::dbus::PendingCall pending = m_bus->callAsync(message, [this, id, finished, done](const base::dbus::Reply& reply) {
        *finished = true;
        m_pending.erase(id);
        done(reply);
    });
    if (!*finished)
        m_pending.emplace(id, std::move(pending));
}

void SecretReader::load(const std::string& uuid, const std::string& settingName,
                        std::function<void(const std::map<std::string, std::string>&)> done)
{
    // "plain" sends the secret unencrypted, which is what libsecret falls back
    // to as well; the session bus never leaves the machine.
    call(servicePath, serviceInterface, "OpenSession",
         {Variant(std::string("plain")), base::dbus::Type<Variant>::toVariant(Variant(std::string()))},
         [this, uuid, settingName, done](const base::dbus::Reply& reply) {
             const auto session = reply.value<ObjectPath>(1);
             if (!session) {
                 done({});
                 return;
             }
             const std::string sessionPath = session->path;
             search({{"connection-uuid", uuid}, {"setting-name", settingName}},
                    [this, sessionPath, done](const Paths& items) {
                        if (items.empty()) {
                            closeSession(sessionPath);
                            done({});
                            return;
                        }

                        struct Pending
                        {
                            std::map<std::string, std::string> secrets;
                            size_t calls = 0;
                        };
                        auto pending = std::make_shared<Pending>();
                        pending->calls = items.size();
                        auto settle = [this, sessionPath, done, pending] {
                            if (--pending->calls > 0)
                                return;
                            closeSession(sessionPath);
                            done(pending->secrets);
                        };

                        for (const ObjectPath& item : items) {
                            call(item.path, propertiesInterface, "Get",
                                 {Variant(itemInterface), Variant(std::string("Attributes"))},
                                 [this, item, sessionPath, pending, settle](const base::dbus::Reply& reply) {
                                     const auto attributes = reply.value<Variant>();
                                     const auto map = attributes ? attributes->get<std::map<std::string, std::string>>()
                                                                 : std::nullopt;
                                     const auto key = map ? map->find("setting-key") : std::map<std::string, std::string>::const_iterator();
                                     if (!map || key == map->end() || key->second.empty()) {
                                         settle();
                                         return;
                                     }
                                     const std::string name = key->second;
                                     call(item.path, itemInterface, "GetSecret", {Variant(ObjectPath{sessionPath})},
                                          [name, pending, settle](const base::dbus::Reply& reply) {
                                              if (const auto secret = reply.value<Secret>()) {
                                                  const auto& value = std::get<2>(*secret);
                                                  pending->secrets[name] = std::string(value.begin(), value.end());
                                              }
                                              settle();
                                          });
                                 });
                        }
                    });
         });
}

void SecretReader::search(const std::map<std::string, std::string>& attributes, std::function<void(const Paths&)> done)
{
    call(servicePath, serviceInterface, "SearchItems", {Variant(attributes)},
         [this, done](const base::dbus::Reply& reply) {
             Paths unlocked;
             Paths locked;
             if (!reply.read(unlocked, locked)) {
                 done({});
                 return;
             }
             unlock(locked, [unlocked, done](const Paths& paths) {
                 Paths all = unlocked;
                 all.insert(all.end(), paths.begin(), paths.end());
                 done(all);
             });
         });
}

void SecretReader::unlock(const Paths& objects, std::function<void(const Paths&)> done)
{
    if (objects.empty()) {
        done({});
        return;
    }

    call(servicePath, serviceInterface, "Unlock", {Variant(objects)}, [this, done](const base::dbus::Reply& reply) {
        Paths unlocked;
        ObjectPath promptPath;
        if (!reply.read(unlocked, promptPath)) {
            done({});
            return;
        }
        if (promptPath.path == "/") {
            done(unlocked);
            return;
        }
        prompt(promptPath.path, [unlocked, done](bool dismissed, const Paths& result) {
            Paths all = unlocked;
            if (!dismissed)
                all.insert(all.end(), result.begin(), result.end());
            done(all);
        });
    });
}

void SecretReader::prompt(const std::string& prompt, std::function<void(bool dismissed, const Paths& result)> done)
{
    const uint64_t id = ++m_nextId;
    struct State
    {
        std::function<void(bool, const Paths&)> done;
        base::Timer timer;
    };
    auto state = std::make_shared<State>();
    state->done = std::move(done);
    std::weak_ptr<char> alive = m_alive;
    // The match owns the state; the timer it holds must not, or neither goes.
    std::weak_ptr<State> weakState = state;
    const auto finish = [this, id, weakState, alive](bool dismissed, const Paths& result) {
        const std::shared_ptr<State> state = weakState.lock();
        if (!state || alive.expired() || !state->done)
            return;
        auto callback = std::move(state->done);
        state->done = nullptr;
        state->timer.stop();
        m_prompts.erase(id);
        callback(dismissed, result);
    };

    base::dbus::MatchRule rule;
    rule.sender = service;
    rule.path = prompt;
    rule.interface = promptInterface;
    rule.member = "Completed";
    m_prompts.emplace(id, m_bus->subscribe(rule, [finish, state](const Message& message) {
        bool dismissed = true;
        Variant result;
        message.read(dismissed, result);
        finish(dismissed, result.get<Paths>().value_or(Paths()));
    }));
    // A prompt the service never completes - it crashed, or the dialog was
    // lost - counts as dismissed instead of keeping the callback chain.
    state->timer.setSingleShot(true);
    state->timer.onTimeout = [finish] { finish(true, {}); };
    state->timer.start(kPromptTimeout);

    call(prompt, promptInterface, "Prompt", {Variant(std::string())}, [finish](const base::dbus::Reply& reply) {
        if (reply.isError())
            finish(true, {});
    });
}

void SecretReader::closeSession(const std::string& session)
{
    call(session, sessionInterface, "Close", {}, [](const base::dbus::Reply&) {});
}

} // namespace Lingmo::Networkmanager
