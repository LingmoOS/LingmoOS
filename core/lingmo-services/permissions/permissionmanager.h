#pragma once

#include <map>
#include <memory>
#include <string>

#include "base/dbus.h"

class PermissionPrompt;

// org.lingmo.Services.Permissions: shows permission prompts on behalf of
// portals and services, which only speak D-Bus and never draw. One prompt per
// token; the answer is the delayed reply to Prompt.
class PermissionManager
{
public:
    explicit PermissionManager(Lingmo::Base::DBus::Connection *session);
    ~PermissionManager();

    PermissionManager(const PermissionManager &) = delete;
    PermissionManager &operator=(const PermissionManager &) = delete;

    // /org/lingmo/Services/Permissions.
    bool publish();

    // The prompt a token opened, for tests; null when none is open.
    PermissionPrompt *prompt(const std::string &token) const;

private:
    // Replies (response, results): 0 allowed, 1 denied or dismissed, 2 failed.
    // results carries whatever the kind's content chose.
    void prompt(Lingmo::Base::DBus::MethodCall &call);
    void dismiss(Lingmo::Base::DBus::MethodCall &call);
    void remove(const std::string &token);

    Lingmo::Base::DBus::Connection *m_session;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_object;
    std::map<std::string, std::unique_ptr<PermissionPrompt>> m_prompts;
};
