#include "permissionmanager.h"
#include "permissionprompt.h"

#include "base/event_loop.h"

using namespace Lingmo::Base;

PermissionManager::PermissionManager(dbus::Connection *session)
    : m_session(session)
{
}

PermissionManager::~PermissionManager()
{
    // Each prompt's handler takes its entry out of the map as it goes.
    auto prompts = std::move(m_prompts);
    m_prompts.clear();
}

bool PermissionManager::publish()
{
    if (!m_session)
        return false;
    m_object = std::make_unique<dbus::Object>(m_session, "/org/lingmo/Services/Permissions",
                                              "org.lingmo.Services.Permissions");
    m_object->addMethod("Prompt", "ssssa{sv}", "ua{sv}",
                        [this](dbus::MethodCall &call) {
                            prompt(call);
                        },
                        {{"token", "kind", "appId", "parentWindow", "options"}, {"response", "results"}});
    m_object->addMethod("Dismiss", "s", "",
                        [this](dbus::MethodCall &call) {
                            dismiss(call);
                        },
                        {{"token"}, {}});
    return m_object->publish();
}

PermissionPrompt *PermissionManager::prompt(const std::string &token) const
{
    const auto it = m_prompts.find(token);
    return it == m_prompts.end() ? nullptr : it->second.get();
}

void PermissionManager::prompt(dbus::MethodCall &call)
{
    std::string token;
    std::string kind;
    std::string appId;
    std::string parentWindow;
    dbus::VariantMap options;
    if (!call.read(token, kind, appId, parentWindow, options)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }
    if (token.empty() || m_prompts.contains(token)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Prompt token is empty or in use");
        return;
    }
    if (!PermissionPrompt::supports(kind)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unknown permission kind");
        return;
    }
    auto prompt = std::make_unique<PermissionPrompt>(kind, appId, parentWindow, options, call);
    PermissionPrompt *shown = prompt.get();
    prompt->onFinished = [this, token] {
        remove(token);
    };
    // A caller that goes away takes its question with it.
    prompt->watchCaller(m_session);
    m_prompts.emplace(token, std::move(prompt));
    shown->show();
}

void PermissionManager::dismiss(dbus::MethodCall &call)
{
    std::string token;
    if (!call.read(token)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }
    PermissionPrompt *prompt = this->prompt(token);
    if (!prompt || prompt->caller() != call.message().sender()) {
        call.replyError("org.freedesktop.DBus.Error.AccessDenied", "No such prompt for this caller");
        return;
    }
    call.reply();
    prompt->dismiss();
}

void PermissionManager::remove(const std::string &token)
{
    const auto it = m_prompts.find(token);
    if (it == m_prompts.end())
        return;
    // Goes once the event that finished it is over.
    std::shared_ptr<PermissionPrompt> finished(std::move(it->second));
    m_prompts.erase(it);
    EventLoop::current()->post([finished] {});
}
