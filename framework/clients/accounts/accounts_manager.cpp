// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/accounts/accounts_manager.h"

#include <algorithm>
#include <chrono>
#include <cstdio>

#include "framework_i18n.h"

using namespace Lingmo::Base;
using Lingmo::framework::tr;

namespace Lingmo::Accounts {

static constexpr const char* kService = "org.freedesktop.Accounts";
static constexpr const char* kPath = "/org/freedesktop/Accounts";
static constexpr const char* kInterface = "org.freedesktop.Accounts";
static constexpr const char* kUserPathPrefix = "/org/freedesktop/Accounts/User";
// Adding and deleting ask polkit, which may take a password's worth of time.
static constexpr std::chrono::seconds kTimeout{120};
// Only replies still in flight can be older than a deletion; no more history is needed.
static constexpr size_t kMaxDeletionSerials = 64;

AccountsManager::AccountsManager(dbus::Connection* bus)
    : m_bus(bus)
    , m_service(bus, kService, kPath, kInterface)
{
    if (!m_bus)
        return;
    m_service.setTimeout(kTimeout);
    m_matches.push_back(m_service.onSignal("UserAdded", [this](const dbus::Message& message) {
        if (const auto path = message.argument<dbus::ObjectPath>())
            addedByService(path->path);
    }));
    m_matches.push_back(m_service.onSignal("UserDeleted", [this](const dbus::Message& message) {
        if (const auto path = message.argument<dbus::ObjectPath>())
            removedByService(path->path);
    }));

    const uint64_t started = m_serial;
    const uint64_t id = startCall();
    m_calls[id] = m_service.callAsync("ListCachedUsers", [this, started, id](const dbus::Reply& reply) {
        if (reply.isError()) {
            fprintf(stderr, "ListCachedUsers failed: %s\n", reply.error().message.c_str());
        } else {
            for (const dbus::ObjectPath& path : reply.value<std::vector<dbus::ObjectPath>>().value_or(
                         std::vector<dbus::ObjectPath>())) {
                if (const AccountPtr user = account(path.path, started))
                    addToList(user);
            }
        }
        m_loaded = true;
        accountsChanged.emit();
        endCall(id);
    });
}

AccountsManager::~AccountsManager() = default;

uint64_t AccountsManager::startCall()
{
    return ++m_nextCall;
}

void AccountsManager::endCall(uint64_t id)
{
    m_calls.erase(id);
}

AccountsManager::AccountPtr AccountsManager::account(const std::string& path, uint64_t requestSerial)
{
    if (path.empty())
        return {};
    if (const auto deleted = m_deletionSerials.find(path);
        deleted != m_deletionSerials.end() && deleted->second > requestSerial)
        return {};
    AccountPtr& user = m_byPath[path];
    if (!user)
        user = std::make_shared<UserAccount>(path, m_bus);
    return user;
}

void AccountsManager::addToList(const AccountPtr& account)
{
    if (std::find(m_accounts.begin(), m_accounts.end(), account) == m_accounts.end())
        m_accounts.push_back(account);
}

void AccountsManager::addedByService(const std::string& path)
{
    const AccountPtr user = account(path, ++m_serial);
    if (!user)
        return;
    const bool listed = std::find(m_accounts.begin(), m_accounts.end(), user) != m_accounts.end();
    addToList(user);
    userAdded.emit(user);
    if (!listed)
        accountsChanged.emit();
}

void AccountsManager::removedByService(const std::string& path)
{
    m_deletionSerials[path] = ++m_serial;
    while (m_deletionSerials.size() > kMaxDeletionSerials) {
        auto oldest = m_deletionSerials.begin();
        for (auto it = m_deletionSerials.begin(); it != m_deletionSerials.end(); ++it) {
            if (it->second < oldest->second)
                oldest = it;
        }
        m_deletionSerials.erase(oldest);
    }

    int64_t uid = -1;
    const std::string prefix = kUserPathPrefix;
    if (path.starts_with(prefix)) {
        try {
            uid = std::stoll(path.substr(prefix.size()));
        } catch (...) {
        }
    }
    const auto found = m_byPath.find(path);
    AccountPtr user;
    if (found != m_byPath.end()) {
        user = found->second;
        m_byPath.erase(found);
    }
    if (uid < 0 && user)
        uid = user->userId();
    const auto listed = std::remove_if(m_accounts.begin(), m_accounts.end(), [&](const AccountPtr& account) {
        return account == user || (uid >= 0 && account->userId() == uid);
    });
    const bool changed = listed != m_accounts.end();
    m_accounts.erase(listed, m_accounts.end());
    if (uid >= 0)
        userDeleted.emit(uid);
    if (changed)
        accountsChanged.emit();
}

AccountsManager::AccountPtr AccountsManager::findUserById(int64_t uid)
{
    const uint64_t started = m_serial;
    const dbus::Reply reply = m_service.call("FindUserById", uid);
    if (reply.isError()) {
        fprintf(stderr, "FindUserById failed: %s\n", reply.error().message.c_str());
        return {};
    }
    return account(reply.value<dbus::ObjectPath>().value_or(dbus::ObjectPath()).path, started);
}

AccountsManager::AccountPtr AccountsManager::findUserByName(const std::string& userName)
{
    const uint64_t started = m_serial;
    const dbus::Reply reply = m_service.call("FindUserByName", userName);
    if (reply.isError()) {
        fprintf(stderr, "FindUserByName failed: %s\n", reply.error().message.c_str());
        return {};
    }
    return account(reply.value<dbus::ObjectPath>().value_or(dbus::ObjectPath()).path, started);
}

void AccountsManager::createUser(const std::string& userName, const std::string& fullName,
                                 UserAccount::AccountType type,
                                 std::function<void(const AccountPtr& account, const std::string& error)> done)
{
    if (!m_bus) {
        if (done)
            done({}, tr("The accounts service cannot be reached"));
        return;
    }
    dbus::Message message = m_bus->newMethodCall(kService, kPath, kInterface, "CreateUser");
    message.append(userName, fullName, int32_t(type));
    message.setAllowInteractiveAuthorization(true);
    const uint64_t started = m_serial;
    const uint64_t id = startCall();
    m_calls[id] = m_bus->callAsync(message, [this, started, id, done](const dbus::Reply& reply) {
        if (reply.isError()) {
            if (done)
                done({}, reply.error().message);
        } else if (const AccountPtr user =
                           account(reply.value<dbus::ObjectPath>().value_or(dbus::ObjectPath()).path, started)) {
            if (done)
                done(user, {});
        } else if (done) {
            done({}, tr("Created user was removed"));
        }
        endCall(id);
    }, kTimeout);
}

void AccountsManager::deleteUser(int64_t uid, bool removeFiles, std::function<void(const std::string& error)> done)
{
    if (!m_bus)
        return;
    dbus::Message message = m_bus->newMethodCall(kService, kPath, kInterface, "DeleteUser");
    message.append(uid, removeFiles);
    message.setAllowInteractiveAuthorization(true);
    const uint64_t id = startCall();
    m_calls[id] = m_bus->callAsync(message, [this, id, done](const dbus::Reply& reply) {
        if (reply.isError())
            fprintf(stderr, "DeleteUser failed: %s\n", reply.error().message.c_str());
        if (done)
            done(reply.isError() ? reply.error().message : std::string());
        endCall(id);
    }, kTimeout);
}

} // namespace Lingmo::Accounts
