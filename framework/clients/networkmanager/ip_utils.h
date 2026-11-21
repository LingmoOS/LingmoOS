// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager {

class Client;

namespace ip {

struct Method
{
    // NetworkManager's.
    std::string value;
    std::string label;
};

// A profile's addressing as its form shows it, the text as typed. Only the
// first address of a family is shown.
struct Form
{
    std::string ipv4Method;
    std::string ipv4Address;
    std::string ipv4Netmask;
    std::string ipv4Gateway;
    std::string ipv6Method;
    std::string ipv6Address;
    std::string ipv6Prefix;
    std::string ipv6Gateway;
    bool automaticDns = true;
    // Comma-separated.
    std::string dns;
    // The choices, with a method the form doesn't offer kept when the profile has it.
    std::vector<Method> ipv4Methods;
    std::vector<Method> ipv6Methods;
};

struct Check
{
    bool complete = false;
    // What is wrong with a value that was typed; a required value left empty
    // only makes the form incomplete.
    std::string message;
};

Form form(const Settings& settings);
// The addresses an active profile has now, in the form's address fields, for
// filling in a manual form.
Form currentAddresses(const Client& client, const std::string& connection);
Check check(const Form& form);
// The profile with the form's addressing. Only the parts of the form that
// differ from the profile are written, so whatever the form doesn't show
// stays as it is. Nothing, with `error` set, when the form is invalid;
// `changed` says whether anything differs.
std::optional<Settings> settings(const Form& form, const Settings& existing, bool* changed, std::string* error);
// Whether the family takes name servers: it asks the network for its
// addresses, or has its own.
bool configurable(const std::string& method);

} // namespace ip
} // namespace Lingmo::Networkmanager
