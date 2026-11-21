// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "polkitagentlistener.h"
#include "authsession.h"
#include "dialog.h"

#include <pwd.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/locale.h"
#include "base/logging.h"
#include "base/strings.h"

using namespace Lingmo::Base;
using namespace Lingmo::Base::strings;

static const char login1Service[] = "org.freedesktop.login1";
static const char polkitService[] = "org.freedesktop.PolicyKit1";
static const char cancelledError[] = "org.freedesktop.PolicyKit1.Error.Cancelled";
static const char failedError[] = "org.freedesktop.PolicyKit1.Error.Failed";

static const int maxTries = 3;

static const LogCategory &log()
{
    static const LogCategory category("lingmo.polkit");
    return category;
}

static std::string loginSessionId(dbus::Connection *bus)
{
    const dbus::Proxy manager(bus, login1Service, "/org/freedesktop/login1", "org.freedesktop.login1.Manager");
    const dbus::Reply session = manager.call("GetSessionByPID", uint32_t(::getpid()));
    const auto path = session.value<dbus::ObjectPath>();
    if (!path) {
        LM_WARNING(log()) << "Cannot find the login session:" << session.error().message;
        return {};
    }

    const dbus::Proxy properties(bus, login1Service, path->path, "org.freedesktop.login1.Session");
    return properties.property<std::string>("Id").value_or(std::string());
}

static std::string userName(uint32_t uid)
{
    const struct passwd *entry = getpwuid(uid);

    return entry ? std::string(entry->pw_name) : std::string();
}

// The dialog's own placeholder stands in for the plain password prompt.
static std::string promptLabel(const std::string &prompt)
{
    std::string label(trimmed(prompt));

    for (;;) {
        if (endsWith(label, ":"))
            label.pop_back();
        else if (endsWith(label, "\xEF\xBC\x9A"))
            label.resize(label.size() - 3);
        else
            break;
    }

    label = std::string(trimmed(label));

    if (equalsIgnoreAsciiCase(label, "Password"))
        return {};

    return label;
}

PolKitAgentListener::PolKitAgentListener(dbus::Connection *bus)
    : m_bus(bus)
{
}

PolKitAgentListener::~PolKitAgentListener()
{
    releaseSession();
    releaseDialog();
}

bool PolKitAgentListener::registerAgent(const std::string &objectPath)
{
    if (!m_bus)
        return false;

    m_object = std::make_unique<dbus::Object>(m_bus, objectPath, "org.freedesktop.PolicyKit1.AuthenticationAgent");
    m_object->addMethod("BeginAuthentication", "sssa{ss}sa(sa{sv})", "",
                        [this](dbus::MethodCall &call) {
                            beginAuthentication(call);
                        },
                        {{"actionId", "message", "iconName", "details", "cookie", "identities"}, {}});
    m_object->addMethod("CancelAuthentication", [this](const std::string &cookie) {
        cancelAuthentication(cookie);
    }, {{"cookie"}, {}});
    if (!m_object->publish()) {
        LM_WARNING(log()) << "Cannot export the authentication agent on the system bus";
        return false;
    }

    m_objectPath = objectPath;

    // polkitd forgets its agents when it restarts, e.g. on an upgrade.
    dbus::MatchRule rule;
    rule.sender = "org.freedesktop.DBus";
    rule.path = "/org/freedesktop/DBus";
    rule.interface = "org.freedesktop.DBus";
    rule.member = "NameOwnerChanged";
    rule.arg0 = polkitService;
    m_ownerChanged = m_bus->subscribe(rule, [this](const dbus::Message &message) {
        std::string name;
        std::string oldOwner;
        std::string newOwner;
        if (message.read(name, oldOwner, newOwner))
            onPolkitOwnerChanged(oldOwner, newOwner);
    });

    return sendRegistration();
}

bool PolKitAgentListener::sendRegistration()
{
    const std::string sessionId = loginSessionId(m_bus);

    if (sessionId.empty())
        return false;

    const dbus::Proxy authority(m_bus, polkitService, "/org/freedesktop/PolicyKit1/Authority",
                                "org.freedesktop.PolicyKit1.Authority");
    const std::tuple<std::string, dbus::VariantMap> subject {"unix-session",
                                                              {{"session-id", dbus::Variant(sessionId)}}};
    const dbus::Reply result = authority.call("RegisterAuthenticationAgent", subject,
                                              Locale::system().name() + ".UTF-8", m_objectPath);

    if (result.isError()) {
        LM_WARNING(log()) << "Cannot register the authentication agent:" << result.error().message;
        return false;
    }

    return true;
}

void PolKitAgentListener::onPolkitOwnerChanged(const std::string &oldOwner, const std::string &newOwner)
{
    // The requests belong to the polkitd that is gone; their cookies mean
    // nothing to the next one.
    if (!oldOwner.empty())
        dropRequests();

    if (!newOwner.empty())
        sendRegistration();
}

void PolKitAgentListener::dropRequests()
{
    releaseSession();
    releaseDialog();

    m_hasResponse = false;
    m_response.clear();
    m_requestPending = false;

    while (!m_queue.empty()) {
        Request request = std::move(m_queue.front());
        m_queue.pop_front();
        reply(request, Withdrawn);
    }
}

void PolKitAgentListener::beginAuthentication(dbus::MethodCall &call)
{
    std::string actionId;
    std::string message;
    std::string iconName;
    std::map<std::string, std::string> details;
    std::string cookie;
    std::vector<Identity> identities;
    if (!call.read(actionId, message, iconName, details, cookie, identities)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    // polkitd lists administrators in no useful order; prefer the one at the
    // keyboard.
    const uint32_t sessionUid = getuid();
    std::string user;

    for (const auto &[kind, identityDetails] : identities) {
        if (kind != "unix-user")
            continue;

        const auto uidValue = identityDetails.find("uid");
        const uint32_t uid = uidValue == identityDetails.end() ? 0 : uidValue->second.value<uint32_t>();

        if (user.empty() || uid == sessionUid)
            user = userName(uid);

        if (uid == sessionUid)
            break;
    }

    if (user.empty()) {
        call.replyError(failedError, "No user to authenticate as");
        return;
    }

    m_queue.push_back({message, cookie, user, call});

    startNext();
}

void PolKitAgentListener::cancelAuthentication(const std::string &cookie)
{
    for (size_t i = 0; i < m_queue.size(); ++i) {
        if (m_queue[i].cookie != cookie)
            continue;

        if (i == 0 && m_dialog) {
            finishCurrent(Withdrawn);
        } else {
            Request request = std::move(m_queue[i]);
            m_queue.erase(m_queue.begin() + long(i));
            reply(request, Withdrawn);
        }

        return;
    }
}

void PolKitAgentListener::startNext()
{
    if (m_dialog || m_queue.empty())
        return;

    const Request &request = m_queue.front();

    m_dialog = std::make_unique<Dialog>(request.message, request.user);

    m_dialog->onAccepted = [this] {
        onDialogAccepted();
    };
    m_dialog->onCancel = [this] {
        finishCurrent(Dismissed);
    };

    m_dialog->show();

    m_numTries = 0;

    startSession();
}

void PolKitAgentListener::startSession()
{
    const Request &request = m_queue.front();

    m_hasResponse = false;
    m_response.clear();
    m_requestPending = false;

    m_session = std::make_unique<AuthSession>();

    m_session->onRequest = [this](const std::string &prompt, bool echo) {
        onRequest(prompt, echo);
    };
    m_session->onNotice = [this](const std::string &text, bool error) {
        onNotice(text, error);
    };
    m_session->onCompleted = [this](bool gainedAuthorization) {
        onCompleted(gainedAuthorization);
    };

    m_session->start(request.user, request.cookie);
}

void PolKitAgentListener::releaseSession()
{
    if (!m_session)
        return;

    // It may be what is calling; it goes on the next turn.
    std::shared_ptr<AuthSession> session(std::move(m_session));
    session->onRequest = nullptr;
    session->onNotice = nullptr;
    session->onCompleted = nullptr;
    EventLoop::current()->post([session] {});
}

void PolKitAgentListener::releaseDialog()
{
    if (!m_dialog)
        return;

    // A button of it may be what is calling; it goes on the next turn.
    std::shared_ptr<Dialog> dialog(std::move(m_dialog));
    dialog->onAccepted = nullptr;
    dialog->onCancel = nullptr;
    dialog->hide();
    EventLoop::current()->post([dialog] {});
}

void PolKitAgentListener::finishCurrent(Outcome outcome)
{
    releaseSession();
    releaseDialog();

    m_hasResponse = false;
    m_response.clear();
    m_requestPending = false;

    Request request = std::move(m_queue.front());
    m_queue.pop_front();
    reply(request, outcome);

    startNext();
}

void PolKitAgentListener::reply(Request &request, Outcome outcome)
{
    switch (outcome) {
    case Completed:
        request.call.reply();
        break;
    case Dismissed:
        request.call.replyError(cancelledError, "The authentication dialog was dismissed");
        break;
    case Withdrawn:
        request.call.replyError(cancelledError, "The request was cancelled");
        break;
    }
}

void PolKitAgentListener::onRequest(const std::string &prompt, bool echo)
{
    if (m_hasResponse) {
        m_session->respond(m_response);
        m_hasResponse = false;
        m_response.clear();
        return;
    }

    m_requestPending = true;
    m_dialog->setRequest(promptLabel(prompt), echo);
}

void PolKitAgentListener::onNotice(const std::string &text, bool error)
{
    m_dialog->setNotice(text, error);
}

void PolKitAgentListener::onCompleted(bool gainedAuthorization)
{
    releaseSession();

    if (gainedAuthorization) {
        finishCurrent(Completed);
        return;
    }

    if (++m_numTries < maxTries) {
        m_dialog->authenticationFailure();
        startSession();
        return;
    }

    finishCurrent(Completed);
}

void PolKitAgentListener::onDialogAccepted()
{
    const std::string response = m_dialog->password();

    m_dialog->setNotice({}, false);

    if (m_requestPending && m_session) {
        m_requestPending = false;
        m_session->respond(response);
        return;
    }

    m_response = response;
    m_hasResponse = true;
}
