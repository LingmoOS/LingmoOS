// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/secrets/secret_store.h"

#include <chrono>
#include <tuple>

#include "base/logging.h"
#include "base/timer.h"
#include "base/event_loop.h"
#include "core/main_loop.h"

namespace Lingmo::Shell {

using namespace std::chrono_literals;
using base::dbus::ObjectPath;
using base::dbus::Variant;

static const base::LogCategory lingmoSecrets("lingmo.shell.secrets");

static const std::string kService = "org.freedesktop.secrets";
static const std::string kServicePath = "/org/freedesktop/secrets";
static const std::string kServiceInterface = "org.freedesktop.Secret.Service";
static const std::string kCollectionInterface = "org.freedesktop.Secret.Collection";
static const std::string kItemInterface = "org.freedesktop.Secret.Item";
static const std::string kPromptInterface = "org.freedesktop.Secret.Prompt";
static const std::string kSessionInterface = "org.freedesktop.Secret.Session";
static const std::string kPropertiesInterface = "org.freedesktop.DBus.Properties";
static const std::string kNoPrompt = "/";

// Completed arrives on the prompt object after the user answered it.
static constexpr auto kPromptTimeout = 10min;

// (session, parameters, value, content type)
using Secret = std::tuple<ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>;

SecretStore::SecretStore(base::dbus::Connection *bus)
    : m_bus(bus)
{
}

SecretStore::~SecretStore() = default;

void SecretStore::call(const std::string &path, const std::string &interface, const std::string &method,
                       Variant::Items arguments, Reply done)
{
    // Without a session bus every answer is an error, on a later turn as a
    // reply would come, so NetworkManager is never left waiting on the agent.
    if (!m_bus) {
        std::weak_ptr<char> alive = m_alive;
        core::mainLoop()->post([alive, done = std::move(done)] {
            if (!alive.expired())
                done(base::dbus::Reply({}, {"org.freedesktop.DBus.Error.Disconnected", "There is no session bus"}));
        });
        return;
    }
    base::dbus::Message message = m_bus->newMethodCall(kService, path, interface, method);
    for (const Variant &argument : arguments)
        message.appendAs(argument);
    const uint64_t id = ++m_nextId;
    auto finished = std::make_shared<bool>(false);
    base::dbus::PendingCall pending = m_bus->callAsync(message, [this, id, finished, method, done](const base::dbus::Reply &reply) {
        *finished = true;
        if (reply.isError())
            LM_WARNING(lingmoSecrets) << "Secret Service:" << method << reply.error().message;
        // The slot goes after the callback returns.
        base::dbus::PendingCall slot;
        if (auto it = m_pending.find(id); it != m_pending.end()) {
            slot = std::move(it->second);
            m_pending.erase(it);
        }
        done(reply);
    });
    if (!*finished)
        m_pending.emplace(id, std::move(pending));
}

void SecretStore::openSession(std::function<void(const std::string &)> done)
{
    // "plain" sends the secret unencrypted, which is what libsecret falls
    // back to as well; the session bus never leaves the machine.
    call(kServicePath, kServiceInterface, "OpenSession",
         {Variant(std::string("plain")), base::dbus::Type<Variant>::toVariant(Variant(std::string()))},
         [done](const base::dbus::Reply &reply) {
             const auto session = reply.value<ObjectPath>(1);
             done(session ? session->path : std::string());
         });
}

void SecretStore::closeSession(const std::string &session)
{
    call(session, kSessionInterface, "Close", {}, [](const base::dbus::Reply &) {});
}

void SecretStore::load(const std::string &uuid, const std::string &settingName,
                       std::function<void(const Secrets &)> done)
{
    openSession([this, uuid, settingName, done](const std::string &session) {
        if (session.empty()) {
            done({});
            return;
        }
        search({{"connection-uuid", uuid}, {"setting-name", settingName}}, [this, session, done](const Paths &items) {
            if (items.empty()) {
                closeSession(session);
                done({});
                return;
            }

            struct Pending {
                Secrets secrets;
                size_t calls = 0;
            };
            auto pending = std::make_shared<Pending>();
            pending->calls = items.size();
            const auto settle = [this, session, done, pending] {
                if (--pending->calls > 0)
                    return;
                closeSession(session);
                done(pending->secrets);
            };

            for (const ObjectPath &item : items) {
                call(item.path, kPropertiesInterface, "Get", {Variant(kItemInterface), Variant(std::string("Attributes"))},
                     [this, item, session, pending, settle](const base::dbus::Reply &reply) {
                         const auto value = reply.value<Variant>();
                         const auto attributes = value ? value->get<std::map<std::string, std::string>>() : std::nullopt;
                         const std::string name = attributes && attributes->count("setting-key")
                                 ? attributes->at("setting-key") : std::string();
                         // Items without a setting-key are not a connection's secret.
                         if (name.empty()) {
                             settle();
                             return;
                         }
                         call(item.path, kItemInterface, "GetSecret", {Variant(ObjectPath {session})},
                              [name, pending, settle](const base::dbus::Reply &reply) {
                                  // Unlocking refused, the item answers with no secret.
                                  if (const auto secret = reply.value<Secret>()) {
                                      const auto &value = std::get<2>(*secret);
                                      pending->secrets[name] = std::string(value.begin(), value.end());
                                  }
                                  settle();
                              });
                     });
            }
        });
    });
}

void SecretStore::save(const std::string &uuid, const std::string &connectionName, const std::string &settingName,
                       const std::string &key, const std::string &value, std::function<void()> done)
{
    const auto finish = [this, done](const std::string &session) {
        if (!session.empty())
            closeSession(session);
        if (done)
            done();
    };

    openSession([=, this](const std::string &session) {
        if (session.empty()) {
            finish(session);
            return;
        }
        call(kServicePath, kServiceInterface, "ReadAlias", {Variant(std::string("default"))},
             [=, this](const base::dbus::Reply &reply) {
                 const auto collection = reply.value<ObjectPath>();
                 if (!collection || collection->path == kNoPrompt) {
                     LM_WARNING(lingmoSecrets) << "Secret Service has no default collection to store" << key << "in";
                     finish(session);
                     return;
                 }
                 unlock({*collection}, [=, this](const Paths &unlocked) {
                     if (unlocked.empty()) {
                         finish(session);
                         return;
                     }
                     const std::map<std::string, std::string> attributes {
                         {"xdg:schema", "org.freedesktop.NetworkManager.Connection"},
                         {"connection-uuid", uuid},
                         {"setting-name", settingName},
                         {"setting-key", key},
                     };
                     const base::dbus::VariantMap properties {
                         {"org.freedesktop.Secret.Item.Label",
                          Variant("Network secret for " + connectionName + "/" + settingName + "/" + key)},
                         {"org.freedesktop.Secret.Item.Attributes", Variant(attributes)},
                     };
                     const Secret secret {ObjectPath {session}, {}, std::vector<uint8_t>(value.begin(), value.end()),
                                          "text/plain"};
                     call(collection->path, kCollectionInterface, "CreateItem",
                          {Variant(properties), Variant(secret), Variant(true)},
                          [=, this](const base::dbus::Reply &reply) {
                              const auto promptPath = reply.value<ObjectPath>(1);
                              if (promptPath && promptPath->path != kNoPrompt)
                                  prompt(promptPath->path, [=](bool, const Paths &) { finish(session); });
                              else
                                  finish(session);
                          });
                 });
             });
    });
}

void SecretStore::replace(const std::string &uuid, const std::string &connectionName,
                          const std::vector<Entry> &entries, std::function<void()> done)
{
    remove(uuid, [=, this] {
        if (entries.empty()) {
            if (done)
                done();
            return;
        }
        auto pending = std::make_shared<size_t>(entries.size());
        for (const Entry &entry : entries) {
            save(uuid, connectionName, entry.setting, entry.key, entry.value, [pending, done] {
                if (--*pending == 0 && done)
                    done();
            });
        }
    });
}

void SecretStore::remove(const std::string &uuid, std::function<void()> done)
{
    search({{"connection-uuid", uuid}}, [this, done](const Paths &items) {
        if (items.empty()) {
            if (done)
                done();
            return;
        }
        auto pending = std::make_shared<size_t>(items.size());
        const auto settle = [pending, done] {
            if (--*pending == 0 && done)
                done();
        };
        for (const ObjectPath &item : items) {
            call(item.path, kItemInterface, "Delete", {}, [this, settle](const base::dbus::Reply &reply) {
                const auto promptPath = reply.value<ObjectPath>();
                if (promptPath && promptPath->path != kNoPrompt)
                    prompt(promptPath->path, [settle](bool, const Paths &) { settle(); });
                else
                    settle();
            });
        }
    });
}

void SecretStore::search(const std::map<std::string, std::string> &attributes, std::function<void(const Paths &)> done)
{
    call(kServicePath, kServiceInterface, "SearchItems", {Variant(attributes)}, [this, done](const base::dbus::Reply &reply) {
        Paths unlocked;
        Paths locked;
        if (!reply.read(unlocked, locked)) {
            done({});
            return;
        }
        unlock(locked, [unlocked, done](const Paths &paths) {
            Paths all = unlocked;
            all.insert(all.end(), paths.begin(), paths.end());
            done(all);
        });
    });
}

void SecretStore::unlock(const Paths &objects, std::function<void(const Paths &)> done)
{
    if (objects.empty()) {
        done({});
        return;
    }
    call(kServicePath, kServiceInterface, "Unlock", {Variant(objects)}, [this, done](const base::dbus::Reply &reply) {
        Paths unlocked;
        ObjectPath promptPath;
        if (!reply.read(unlocked, promptPath)) {
            done({});
            return;
        }
        if (promptPath.path == kNoPrompt) {
            done(unlocked);
            return;
        }
        prompt(promptPath.path, [unlocked, done](bool dismissed, const Paths &result) {
            Paths all = unlocked;
            if (!dismissed)
                all.insert(all.end(), result.begin(), result.end());
            done(all);
        });
    });
}

void SecretStore::prompt(const std::string &prompt, std::function<void(bool dismissed, const Paths &result)> done)
{
    if (!m_bus) {
        done(true, {});
        return;
    }
    const uint64_t id = ++m_nextId;
    struct State {
        std::function<void(bool, const Paths &)> done;
        base::Timer timer;
    };
    auto state = std::make_shared<State>();
    state->done = std::move(done);
    std::weak_ptr<char> alive = m_alive;
    const auto finish = [this, id, state, alive](bool dismissed, const Paths &result) {
        if (!state->done)
            return;
        auto callback = std::move(state->done);
        state->done = nullptr;
        state->timer.stop();
        // The timer's callback holds the state; it lets go once this one,
        // which may be that callback, has returned.
        core::mainLoop()->post([state] { state->timer.onTimeout = nullptr; });
        if (alive.expired())
            return;
        // The match goes after the callback returns.
        base::dbus::Match match;
        if (auto it = m_prompts.find(id); it != m_prompts.end()) {
            match = std::move(it->second);
            m_prompts.erase(it);
        }
        callback(dismissed, result);
    };

    base::dbus::MatchRule rule;
    rule.sender = kService;
    rule.path = prompt;
    rule.interface = kPromptInterface;
    rule.member = "Completed";
    m_prompts.emplace(id, m_bus->subscribe(rule, [finish](const base::dbus::Message &message) {
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

    call(prompt, kPromptInterface, "Prompt", {Variant(std::string())}, [finish](const base::dbus::Reply &reply) {
        if (reply.isError())
            finish(true, {});
    });
}

} // namespace Lingmo::Shell
