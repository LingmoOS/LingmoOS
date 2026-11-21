// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::Accounts {

// One account as org.freedesktop.Accounts keeps it. Reads never wait on the
// bus: a getter returns the cached value, filled when the account is made
// and again on the service's Changed signal. A setter shows its value at once
// and keeps it until the service has answered.
class UserAccount
{
public:
    enum AccountType {
        Standard = 0,
        Administrator = 1,
    };

    enum PasswordMode {
        RegularPassword = 0,
        SetAtLogin = 1,
        NoPassword = 2,
    };

    // The account of the user running the process.
    explicit UserAccount(base::dbus::Connection* bus = base::dbus::Connection::system());
    UserAccount(int64_t uid, base::dbus::Connection* bus = base::dbus::Connection::system());
    UserAccount(std::string objectPath, base::dbus::Connection* bus);
    ~UserAccount();

    UserAccount(const UserAccount&) = delete;
    UserAccount& operator=(const UserAccount&) = delete;

    const std::string& objectPath() const { return m_path; }

    int64_t userId() const;
    int64_t groupId() const;
    AccountType accountType() const;
    void setAccountType(AccountType type);
    bool isLocked() const;
    void setLocked(bool locked);
    bool automaticLogin() const;
    void setAutomaticLogin(bool enabled);
    int64_t loginFrequency() const;
    int64_t loginTime() const;
    PasswordMode passwordMode() const;
    void setPasswordMode(PasswordMode mode);
    std::string passwordHint() const;
    bool isLocalAccount() const;
    bool isSystemAccount() const;
    std::string userName() const;
    void setUserName(const std::string& name);
    std::string realName() const;
    void setRealName(const std::string& name);
    // The real name, or the user name when there is none.
    std::string displayName() const;
    std::string homeDirectory() const;
    void setHomeDirectory(const std::string& path);
    std::string shell() const;
    void setShell(const std::string& path);
    // Empty when the file is gone.
    std::string iconFileName() const;
    void setIconFileName(const std::string& path);
    std::string email() const;
    void setEmail(const std::string& address);
    std::string language() const;
    void setLanguage(const std::string& locale);
    std::string location() const;
    void setLocation(const std::string& place);
    std::string xsession() const;
    void setXSession(const std::string& session);

    // Takes the password already hashed, as crypt(3) gives it.
    void setPassword(const std::string& hashed, const std::string& hint = {});
    void setPasswordHint(const std::string& hint);

    // After any value changed.
    base::Signal<> changed;

private:
    void refresh();
    void applyValues(base::dbus::VariantMap values);
    // Calls a setter of the service; the value shows until the reply.
    void update(const char* property, base::dbus::Variant value, const char* method);
    void call(const char* property, base::dbus::Message message);
    template<typename T>
    T value(const char* property, const T& fallback) const;

    base::dbus::Connection* m_bus;
    std::string m_path;
    base::dbus::Match m_changedMatch;
    base::dbus::PendingCall m_refresh;
    uint64_t m_refreshSerial = 0;
    std::map<uint64_t, base::dbus::PendingCall> m_calls;
    uint64_t m_nextCall = 0;
    base::dbus::VariantMap m_values;
    // Values set here that the service has not answered for yet, and how
    // many calls for each are still in flight.
    base::dbus::VariantMap m_pendingValues;
    std::map<std::string, int> m_pendingCalls;
};

} // namespace Lingmo::Accounts
