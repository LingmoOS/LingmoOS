// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"

namespace Lingmo::Networkmanager {

// Reads connection secrets from the user's keyring through the freedesktop
// Secret Service, by nm-applet's attributes, which the shell's secret agent
// stores them with.
class SecretReader
{
public:
    SecretReader();
    ~SecretReader();

    SecretReader(const SecretReader&) = delete;
    SecretReader& operator=(const SecretReader&) = delete;

    // Hands over key → value for one setting of a connection; empty when none are stored.
    void load(const std::string& uuid, const std::string& settingName,
              std::function<void(const std::map<std::string, std::string>&)> done);

private:
    using Paths = std::vector<base::dbus::ObjectPath>;
    using Reply = std::function<void(const base::dbus::Reply&)>;

    void call(const std::string& path, const std::string& interface, const std::string& method,
              base::dbus::Variant::Items arguments, Reply done);
    void search(const std::map<std::string, std::string>& attributes, std::function<void(const Paths&)> done);
    void unlock(const Paths& objects, std::function<void(const Paths&)> done);
    void prompt(const std::string& prompt, std::function<void(bool dismissed, const Paths& result)> done);
    void closeSession(const std::string& session);

    base::dbus::Connection* m_bus;
    std::shared_ptr<char> m_alive = std::make_shared<char>();
    std::map<uint64_t, base::dbus::PendingCall> m_pending;
    std::map<uint64_t, base::dbus::Match> m_prompts;
    uint64_t m_nextId = 0;
};

} // namespace Lingmo::Networkmanager
