// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"

namespace Lingmo::Shell {

// Connection secrets kept in the user's keyring through the freedesktop
// Secret Service (gnome-keyring here). Items carry nm-applet's attributes
// (connection-uuid, setting-name, setting-key), so secrets stored by either
// agent are found by the other. Every call is asynchronous; callbacks never
// run once the store is gone. Port of lingmo-framework's SecretStore.
class SecretStore
{
public:
    using Secrets = std::map<std::string, std::string>;

    struct Entry {
        std::string setting;
        std::string key;
        std::string value;
    };

    // `bus` is the session bus; null answers every call as an empty keyring.
    explicit SecretStore(base::dbus::Connection *bus);
    ~SecretStore();

    SecretStore(const SecretStore &) = delete;
    SecretStore &operator=(const SecretStore &) = delete;

    // Key to value for one setting of a connection; empty when none are stored.
    void load(const std::string &uuid, const std::string &settingName, std::function<void(const Secrets &)> done);
    void save(const std::string &uuid, const std::string &connectionName, const std::string &settingName,
              const std::string &key, const std::string &value, std::function<void()> done = {});
    // Drops every item of the connection first, so a key it no longer has goes too.
    void replace(const std::string &uuid, const std::string &connectionName, const std::vector<Entry> &entries,
                 std::function<void()> done = {});
    void remove(const std::string &uuid, std::function<void()> done = {});

private:
    using Paths = std::vector<base::dbus::ObjectPath>;
    using Reply = std::function<void(const base::dbus::Reply &)>;

    void call(const std::string &path, const std::string &interface, const std::string &method,
              base::dbus::Variant::Items arguments, Reply done);
    void openSession(std::function<void(const std::string &session)> done);
    void closeSession(const std::string &session);
    void search(const std::map<std::string, std::string> &attributes, std::function<void(const Paths &)> done);
    void unlock(const Paths &objects, std::function<void(const Paths &)> done);
    void prompt(const std::string &prompt, std::function<void(bool dismissed, const Paths &result)> done);

    base::dbus::Connection *m_bus;
    std::shared_ptr<char> m_alive = std::make_shared<char>();
    std::map<uint64_t, base::dbus::PendingCall> m_pending;
    std::map<uint64_t, base::dbus::Match> m_prompts;
    uint64_t m_nextId = 0;
};

} // namespace Lingmo::Shell

