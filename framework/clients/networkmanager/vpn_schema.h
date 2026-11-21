// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// What each kind of NetworkManager VPN plugin can be told, as the GNOME and
// KDE editors lay it out: a field per vpn.data key, or per vpn.secrets key
// with its `-flags` entry in vpn.data. The forms in Settings are drawn from
// these tables.
namespace Lingmo::Networkmanager::vpn::schema {

enum class Type {
    Text,
    // A vpn.secrets key, kept as its storage choice says.
    Password,
    File,
    Choice,
    Bool,
    Number,
};

struct Choice
{
    std::string value;
    std::string label;
};

struct Field
{
    std::string key;
    std::string label;
    Type type = Type::Text;
    // Advanced fields sit behind "Show advanced options", grouped by section.
    bool advanced = false;
    std::string section;
    bool required = false;
    std::string placeholder;
    std::vector<Choice> choices;
    // The value an absent key stands for: a Choice's default, a Number's placeholder.
    std::string defaultValue;
    // Shown only while another field has one of these values: "key=a|b".
    std::string showWhen;
    // Bools: what is written for true (removed for false, unless falseValue is set).
    std::string trueValue = "yes";
    std::string falseValue;
    // Bools whose key says the opposite of the label, e.g. "nobsdcomp" for "BSD compression".
    bool inverted = false;
    // Bools that set other keys along with their own, e.g. the LCP echo interval and failure count.
    std::map<std::string, std::string> alsoSet;
    // Choices whose values are keys: the chosen one is set to "yes", the others removed.
    bool exclusiveKeys = false;
    // Passwords the plugin can do without, as a storage choice.
    bool allowNotRequired = false;
    // A legacy key some plugins keep next to the flags: save / ask / unused.
    std::string legacyTypeKey;
    int minimum = 0;
    int maximum = 0;
};

// A field with its value from a profile, as the form starts out: Bools in
// `checked`, already turned round when inverted; passwords empty, with
// `storage` set to their flags; a Choice's value from another editor added to
// its choices as itself.
struct FormField : Field
{
    std::string value;
    bool checked = false;
    uint32_t storage = 0;
};

// The form's values per key: text as typed, and "true" or "false" for Bools.
using Values = std::map<std::string, std::string>;

// Every kind a form exists for, in the order the Add VPN list shows them;
// "wireguard" has its own form and isn't here.
std::vector<std::string> kinds();
std::vector<Field> fields(const std::string& kind);
// Where the plugin's reader and writer can take a file from and to, as
// nmcli's import and export do: name filters like "*.ovpn", empty when the
// plugin has none.
std::vector<std::string> importPatterns(const std::string& kind);
bool exportable(const std::string& kind);

std::vector<FormField> formFields(const std::string& kind, const std::map<std::string, std::string>& data);

// Whether the field's condition holds for these values.
bool shown(const Field& field, const std::vector<Field>& fields, const Values& values);

// vpn.data and vpn.secrets after the form's values are written over the
// existing ones; keys the form doesn't know stay. `storage` holds the flags
// chosen per password, `secrets` the passwords typed, which with `shared` set
// are system-owned rather than this user's. False, with `error` set, when a
// required field is empty or a number is out of range.
bool apply(const std::string& kind, const Values& values, const std::map<std::string, uint32_t>& storage,
           const std::map<std::string, std::string>& secrets, bool shared, std::map<std::string, std::string>& data,
           std::map<std::string, std::string>& vpnSecrets, std::string* error);

} // namespace Lingmo::Networkmanager::vpn::schema
