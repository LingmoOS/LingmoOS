// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/screen_saver_service.h"
#include "core/main_loop.h"
#include "session/session_controller.h"
#include "base/logging.h"
#include <algorithm>

namespace Lingmo::Session {

namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

const std::string serviceName = "org.freedesktop.ScreenSaver";
const std::string interfaceName = "org.freedesktop.ScreenSaver";
const char *const objectPaths[] = {"/ScreenSaver", "/org/freedesktop/ScreenSaver"};
const std::string failedError = "org.freedesktop.DBus.Error.Failed";
} // namespace

ScreenSaverService::ScreenSaverService(SessionController *session, base::dbus::Connection *bus)
    : m_session(session)
    , m_bus(bus ? bus : base::dbus::Connection::session(core::mainLoop()))
    , m_active(session->isBlocked())
{
    m_session->addObserver(this);
}

ScreenSaverService::~ScreenSaverService()
{
    m_session->removeObserver(this);
    for (auto &request : m_pending)
        request.replyError(failedError, "Compositor is shutting down");
    if (m_registered)
        m_bus->releaseName(serviceName);
}

bool ScreenSaverService::registerService()
{
    if (m_registered)
        return true;
    if (!m_bus) {
        LM_WARNING(logs()) << "Cannot export ScreenSaver: no session bus";
        return false;
    }
    std::vector<std::unique_ptr<base::dbus::Object>> objects;
    for (const char *path : objectPaths) {
        auto object = std::make_unique<base::dbus::Object>(m_bus, path, interfaceName);
        object->addMethod("Lock", "", "", [this](base::dbus::MethodCall &call) { lock(&call); });
        object->addMethod("GetActive", [this] { return GetActive(); });
        object->addMethod("Inhibit", "ss", "u",
                          [this](base::dbus::MethodCall &call) { inhibit(call); },
                          base::dbus::ArgumentNames{{"application_name", "reason_for_inhibit"},
                                                    {"result"}});
        object->addMethod("UnInhibit", "u", "",
                          [this](base::dbus::MethodCall &call) { uninhibit(call); },
                          base::dbus::ArgumentNames{{"cookie"}, {}});
        object->addSignal("ActiveChanged", "b", {"active"});
        // Only the paths exported here go with a failure: another object may
        // hold this one.
        if (!object->publish()) {
            LM_WARNING(logs()) << "Cannot export ScreenSaver at" << path;
            return false;
        }
        objects.push_back(std::move(object));
    }
    if (!m_bus->requestName(serviceName)) {
        LM_WARNING(logs()) << "Cannot own ScreenSaver service";
        return false;
    }
    m_objects = std::move(objects);
    m_registered = true;
    return true;
}

void ScreenSaverService::setInhibition(std::function<void()> acquire,
                                       std::function<void()> release)
{
    m_acquire = std::move(acquire);
    m_release = std::move(release);
}

void ScreenSaverService::Lock()
{
    lock(nullptr);
}

void ScreenSaverService::lock(base::dbus::MethodCall *call)
{
    if (m_session->state() == SessionState::Locked) {
        if (call)
            call->reply();
        return;
    }
    if (m_session->state() != SessionState::Locking && !m_session->requestLock()) {
        if (call)
            call->replyError(failedError, "Session is not available for locking");
        return;
    }
    // Callers may suspend or power off only after every output is securely covered.
    if (call)
        m_pending.push_back(*call);
}

bool ScreenSaverService::GetActive() const
{
    return m_session->isBlocked();
}

void ScreenSaverService::inhibit(base::dbus::MethodCall &call)
{
    if (!m_acquire || !m_release) {
        call.replyError("org.freedesktop.DBus.Error.NotSupported",
                        "Idle inhibition is not available in this session");
        return;
    }
    const std::string owner = call.message().sender();
    do {
        ++m_nextCookie;
    } while (!m_nextCookie || m_cookies.contains(m_nextCookie));
    m_cookies.emplace(m_nextCookie, owner);
    m_acquire();
    // The owner is read once the watch is in place, so a caller that left
    // before it was is released all the same.
    if (!m_owners.contains(owner)) {
        m_owners.emplace(owner, m_bus->watchName(owner, [this, owner](const std::string &now) {
            if (now.empty())
                releaseOwner(owner);
        }));
    }
    call.reply(m_nextCookie);
}

void ScreenSaverService::uninhibit(base::dbus::MethodCall &call)
{
    uint32_t cookie = 0;
    call.read(cookie);
    call.reply();
    const auto it = m_cookies.find(cookie);
    if (it == m_cookies.end() || it->second != call.message().sender())
        return;
    const std::string owner = it->second;
    m_cookies.erase(it);
    if (std::none_of(m_cookies.begin(), m_cookies.end(),
                     [&owner](const auto &entry) { return entry.second == owner; }))
        m_owners.erase(owner);
    if (m_release)
        m_release();
}

void ScreenSaverService::releaseOwner(const std::string &owner)
{
    for (auto it = m_cookies.begin(); it != m_cookies.end();) {
        if (it->second == owner) {
            it = m_cookies.erase(it);
            if (m_release)
                m_release();
        } else {
            ++it;
        }
    }
    // Last: the watch calling this goes with it.
    m_owners.erase(owner);
}

void ScreenSaverService::onSessionStateChanged(SessionState state)
{
    const bool active = isSessionBlocked(state);
    if (active != m_active) {
        m_active = active;
        activeChanged(active);
        for (const auto &object : m_objects)
            object->emitSignal("ActiveChanged", active);
    }
    if (state == SessionState::Locked) {
        auto pending = std::exchange(m_pending, {});
        for (auto &request : pending)
            request.reply();
    }
}
} // namespace Lingmo::Session
