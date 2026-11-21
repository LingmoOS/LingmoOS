// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "lingmo/accounts/user_account.h"

namespace Lingmo::Accounts {

// The accounts org.freedesktop.Accounts lists, kept up to date as the service
// adds and deletes them, and the calls that add and delete accounts. One
// UserAccount is kept per account and shared.
class AccountsManager
{
public:
    using AccountPtr = std::shared_ptr<UserAccount>;

    explicit AccountsManager(base::dbus::Connection* bus = base::dbus::Connection::system());
    ~AccountsManager();

    AccountsManager(const AccountsManager&) = delete;
    AccountsManager& operator=(const AccountsManager&) = delete;

    // In the order the service listed or added them; filled once the first
    // listing answers.
    const std::vector<AccountPtr>& accounts() const { return m_accounts; }
    bool isLoaded() const { return m_loaded; }

    // These wait for the service.
    AccountPtr findUserById(int64_t uid);
    AccountPtr findUserByName(const std::string& userName);

    // Calls back with the new account, or with the error the service gave;
    // asking polkit may take a while.
    void createUser(const std::string& userName, const std::string& fullName, UserAccount::AccountType type,
                    std::function<void(const AccountPtr& account, const std::string& error)> done);
    // Calls back with the error, empty when the account is gone.
    void deleteUser(int64_t uid, bool removeFiles, std::function<void(const std::string& error)> done = {});

    // After accounts() changed: an account added or deleted, or the first
    // listing.
    base::Signal<> accountsChanged;
    base::Signal<const AccountPtr&> userAdded;
    base::Signal<int64_t> userDeleted;

private:
    // The shared account at the path, unless the service deleted it after
    // the request that names it was made.
    AccountPtr account(const std::string& path, uint64_t requestSerial);
    void addToList(const AccountPtr& account);
    void addedByService(const std::string& path);
    void removedByService(const std::string& path);
    uint64_t startCall();
    void endCall(uint64_t id);

    base::dbus::Connection* m_bus;
    base::dbus::Proxy m_service;
    std::vector<base::dbus::Match> m_matches;
    std::map<uint64_t, base::dbus::PendingCall> m_calls;
    uint64_t m_nextCall = 0;
    std::map<std::string, AccountPtr> m_byPath;
    std::vector<AccountPtr> m_accounts;
    bool m_loaded = false;
    // Increases with every request and every deletion, so a reply can tell
    // whether the account it names was deleted after it was asked for.
    uint64_t m_serial = 0;
    std::map<std::string, uint64_t> m_deletionSerials;
};

} // namespace Lingmo::Accounts
