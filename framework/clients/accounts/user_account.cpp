// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/accounts/user_account.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace Lingmo::Base;

namespace Lingmo::Accounts {

static constexpr const char* kService = "org.freedesktop.Accounts";
static constexpr const char* kUserInterface = "org.freedesktop.Accounts.User";
static constexpr const char* kUserPathPrefix = "/org/freedesktop/Accounts/User";
// A change may wait for a polkit password.
static constexpr std::chrono::seconds kTimeout{120};

static std::string userPath(int64_t uid)
{
    return kUserPathPrefix + std::to_string(uid);
}

UserAccount::UserAccount(dbus::Connection* bus)
    : UserAccount(int64_t(getuid()), bus)
{
}

UserAccount::UserAccount(int64_t uid, dbus::Connection* bus)
    : UserAccount(userPath(uid), bus)
{
}

UserAccount::UserAccount(std::string objectPath, dbus::Connection* bus)
    : m_bus(bus)
    , m_path(std::move(objectPath))
{
    // Known from the path before the first read answers.
    const std::string prefix = kUserPathPrefix;
    if (m_path.starts_with(prefix)) {
        try {
            m_values["Uid"] = uint64_t(std::stoull(m_path.substr(prefix.size())));
        } catch (...) {
        }
    }
    if (!m_bus)
        return;
    m_changedMatch = m_bus->subscribe({.sender = kService, .path = m_path, .interface = kUserInterface,
                                       .member = "Changed"},
                                      [this](const dbus::Message&) { refresh(); });
    refresh();
}

UserAccount::~UserAccount() = default;

template<typename T>
T UserAccount::value(const char* property, const T& fallback) const
{
    const auto it = m_values.find(property);
    return it == m_values.end() ? fallback : it->second.value<T>(fallback);
}

void UserAccount::refresh()
{
    if (!m_bus)
        return;
    const uint64_t serial = ++m_refreshSerial;
    dbus::Proxy properties(m_bus, kService, m_path, kUserInterface);
    m_refresh = properties.getAllAsync([this, serial](const dbus::Reply& reply) {
        if (serial != m_refreshSerial)
            return;
        if (reply.isError()) {
            fprintf(stderr, "AccountsService read failed: %s\n", reply.error().message.c_str());
            return;
        }
        applyValues(reply.value<dbus::VariantMap>().value_or(dbus::VariantMap()));
    });
}

void UserAccount::applyValues(dbus::VariantMap values)
{
    for (const auto& [name, value] : m_pendingValues)
        values[name] = value;
    if (values == m_values)
        return;
    m_values = std::move(values);
    changed.emit();
}

void UserAccount::update(const char* property, dbus::Variant value, const char* method)
{
    if (!m_bus)
        return;
    dbus::Message message = m_bus->newMethodCall(kService, m_path, kUserInterface, method);
    message.appendAs(value);
    m_pendingValues[property] = value;
    m_values[property] = std::move(value);
    call(property, std::move(message));
    changed.emit();
}

void UserAccount::call(const char* property, dbus::Message message)
{
    message.setAllowInteractiveAuthorization(true);
    const std::string key = property ? property : "";
    if (!key.empty())
        ++m_pendingCalls[key];
    const uint64_t id = ++m_nextCall;
    m_calls[id] = m_bus->callAsync(message, [this, key, id](const dbus::Reply& reply) {
        // Only this property's value goes: another call for it still in flight keeps its own.
        if (const auto calls = m_pendingCalls.find(key); calls != m_pendingCalls.end() && --calls->second == 0) {
            m_pendingCalls.erase(calls);
            m_pendingValues.erase(key);
        }
        if (reply.isError())
            fprintf(stderr, "AccountsService update failed: %s\n", reply.error().message.c_str());
        // The value already shows, so this stays quiet unless the service settled on another.
        refresh();
        m_calls.erase(id);
    }, kTimeout);
}

int64_t UserAccount::userId() const
{
    return int64_t(value<uint64_t>("Uid", 0));
}

int64_t UserAccount::groupId() const
{
    std::vector<char> storage(16384);
    struct passwd data;
    struct passwd* result = nullptr;
    for (;;) {
        const int error = getpwuid_r(uid_t(userId()), &data, storage.data(), storage.size(), &result);
        if (error == 0)
            return result ? result->pw_gid : 0;
        if (error != ERANGE || storage.size() >= 1024 * 1024)
            return 0;
        storage.resize(storage.size() * 2);
    }
}

UserAccount::AccountType UserAccount::accountType() const
{
    return AccountType(value<int32_t>("AccountType", Standard));
}

void UserAccount::setAccountType(AccountType type)
{
    if (accountType() != type)
        update("AccountType", int32_t(type), "SetAccountType");
}

bool UserAccount::isLocked() const
{
    return value<bool>("Locked", false);
}

void UserAccount::setLocked(bool locked)
{
    if (isLocked() != locked)
        update("Locked", locked, "SetLocked");
}

bool UserAccount::automaticLogin() const
{
    return value<bool>("AutomaticLogin", false);
}

void UserAccount::setAutomaticLogin(bool enabled)
{
    if (automaticLogin() != enabled)
        update("AutomaticLogin", enabled, "SetAutomaticLogin");
}

int64_t UserAccount::loginFrequency() const
{
    return int64_t(value<uint64_t>("LoginFrequency", 0));
}

int64_t UserAccount::loginTime() const
{
    return value<int64_t>("LoginTime", 0);
}

UserAccount::PasswordMode UserAccount::passwordMode() const
{
    return PasswordMode(value<int32_t>("PasswordMode", RegularPassword));
}

void UserAccount::setPasswordMode(PasswordMode mode)
{
    if (passwordMode() != mode)
        update("PasswordMode", int32_t(mode), "SetPasswordMode");
}

std::string UserAccount::passwordHint() const
{
    return value<std::string>("PasswordHint", {});
}

bool UserAccount::isLocalAccount() const
{
    return value<bool>("LocalAccount", false);
}

bool UserAccount::isSystemAccount() const
{
    return value<bool>("SystemAccount", false);
}

std::string UserAccount::userName() const
{
    return value<std::string>("UserName", {});
}

void UserAccount::setUserName(const std::string& name)
{
    if (userName() != name)
        update("UserName", name, "SetUserName");
}

std::string UserAccount::realName() const
{
    return value<std::string>("RealName", {});
}

void UserAccount::setRealName(const std::string& name)
{
    if (realName() != name)
        update("RealName", name, "SetRealName");
}

std::string UserAccount::displayName() const
{
    const std::string real = realName();
    return real.empty() ? userName() : real;
}

std::string UserAccount::homeDirectory() const
{
    return value<std::string>("HomeDirectory", {});
}

void UserAccount::setHomeDirectory(const std::string& path)
{
    if (homeDirectory() != path)
        update("HomeDirectory", path, "SetHomeDirectory");
}

std::string UserAccount::shell() const
{
    return value<std::string>("Shell", {});
}

void UserAccount::setShell(const std::string& path)
{
    if (shell() != path)
        update("Shell", path, "SetShell");
}

std::string UserAccount::iconFileName() const
{
    const std::string path = value<std::string>("IconFile", {});
    struct stat info;
    return !path.empty() && stat(path.c_str(), &info) == 0 ? path : std::string();
}

void UserAccount::setIconFileName(const std::string& path)
{
    if (iconFileName() != path)
        update("IconFile", path, "SetIconFile");
}

std::string UserAccount::email() const
{
    return value<std::string>("Email", {});
}

void UserAccount::setEmail(const std::string& address)
{
    if (email() != address)
        update("Email", address, "SetEmail");
}

std::string UserAccount::language() const
{
    return value<std::string>("Language", {});
}

void UserAccount::setLanguage(const std::string& locale)
{
    if (language() != locale)
        update("Language", locale, "SetLanguage");
}

std::string UserAccount::location() const
{
    return value<std::string>("Location", {});
}

void UserAccount::setLocation(const std::string& place)
{
    if (location() != place)
        update("Location", place, "SetLocation");
}

std::string UserAccount::xsession() const
{
    return value<std::string>("XSession", {});
}

void UserAccount::setXSession(const std::string& session)
{
    if (xsession() != session)
        update("XSession", session, "SetXSession");
}

void UserAccount::setPassword(const std::string& hashed, const std::string& hint)
{
    if (!m_bus)
        return;
    dbus::Message message = m_bus->newMethodCall(kService, m_path, kUserInterface, "SetPassword");
    message.append(hashed, hint);
    call(nullptr, std::move(message));
}

void UserAccount::setPasswordHint(const std::string& hint)
{
    if (!m_bus || passwordHint() == hint)
        return;
    dbus::Message message = m_bus->newMethodCall(kService, m_path, kUserInterface, "SetPasswordHint");
    message.append(hint);
    call(nullptr, std::move(message));
}

} // namespace Lingmo::Accounts
