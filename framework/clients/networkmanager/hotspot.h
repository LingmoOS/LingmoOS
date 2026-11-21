// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager {

class Client;

// The hotspot's name and password, the same for everything in the process.
// They live in NetworkManager's hotspot profile once it exists; until then
// the machine's name and a generated password stand in.
class HotspotConfiguration
{
public:
    static HotspotConfiguration* instance();

    HotspotConfiguration();
    ~HotspotConfiguration();

    HotspotConfiguration(const HotspotConfiguration&) = delete;
    HotspotConfiguration& operator=(const HotspotConfiguration&) = delete;

    std::string name() const;
    void setName(const std::string& name);
    std::string password() const;
    void setPassword(const std::string& password);

    // 1 to 32 bytes, the SSID limit.
    static bool nameValid(const std::string& name);
    // A WPA passphrase: 8 to 63 bytes, or 64 hex digits.
    static bool passwordValid(const std::string& password);
    static std::string generatePassword();

    base::Signal<> changed;

private:
    std::string connection() const;
    void load();

    Client* m_client;
    CallContext m_context;
    std::vector<base::Subscription> m_subscriptions;
    std::string m_loadedProfile;
    Settings m_loadedSettings;
    std::string m_name;
    std::string m_password;
    std::string m_profilePassword;
    const std::string m_generatedPassword;
};

} // namespace Lingmo::Networkmanager
