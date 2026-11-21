// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/sleep_lock.h"
#include "core/main_loop.h"
#include "session/session_controller.h"
#include "base/logging.h"

namespace Lingmo::Session {

namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

const char *const service = "org.freedesktop.login1";
const char *const path = "/org/freedesktop/login1";
const char *const manager = "org.freedesktop.login1.Manager";
// logind resolves it to the caller's own session.
const char *const sessionPath = "/org/freedesktop/login1/session/auto";
const char *const sessionInterface = "org.freedesktop.login1.Session";
} // namespace

SleepLock::SleepLock(SessionController *session, base::dbus::Connection *bus)
    : m_session(session)
    , m_bus(bus ? bus : base::dbus::Connection::system(core::mainLoop()))
    , m_manager(m_bus, service, path, manager)
    , m_logindSession(m_bus, service, sessionPath, sessionInterface)
    , m_relock(core::mainLoop())
{
    m_relock.setSingleShot(true);
    m_relock.onTimeout = [this] {
        if (m_sleeping && !m_session->isBlocked())
            m_session->requestLock();
    };
    m_session->addObserver(this);
}

SleepLock::~SleepLock()
{
    m_session->removeObserver(this);
}

bool SleepLock::start()
{
    if (m_started)
        return true;
    if (!m_bus) {
        LM_WARNING(logs()) << "Cannot follow logind's PrepareForSleep: no system bus";
        return false;
    }
    m_prepareForSleep =
            m_manager.onSignal("PrepareForSleep", [this](const base::dbus::Message &message) {
                bool sleeping = false;
                if (message.read(sleeping))
                    prepareForSleep(sleeping);
            });
    if (m_prepareForSleep.isNull()) {
        LM_WARNING(logs()) << "Cannot follow logind's PrepareForSleep";
        return false;
    }
    m_started = true;
    inhibit();
    updateLockedHint(m_session->state());
    return true;
}

void SleepLock::inhibit()
{
    if (!m_inhibiting.isNull() || m_inhibitor.isValid())
        return;
    m_inhibiting = m_manager.callAsync(
            "Inhibit",
            [this](const base::dbus::Reply &reply) {
                base::dbus::PendingCall done = std::move(m_inhibiting);
                if (reply.isError()) {
                    LM_WARNING(logs()) << "Cannot delay sleep to lock the screen:"
                               << reply.error().message;
                    return;
                }
                // Too late for this sleep; the next wake asks again.
                if (!m_sleeping)
                    m_inhibitor = reply.value<base::dbus::UnixFd>().value_or(base::dbus::UnixFd());
            },
            std::string("sleep"), std::string("Lingmo"),
            std::string("Lock the screen before sleeping"), std::string("delay"));
}

void SleepLock::prepareForSleep(bool sleeping)
{
    m_sleeping = sleeping;
    if (!sleeping) {
        inhibit();
        return;
    }
    // An unlock from the login screen finishes first; onSessionStateChanged
    // locks after it.
    m_session->requestLock();
    releaseWhenLocked();
}

void SleepLock::onSessionStateChanged(SessionState state)
{
    updateLockedHint(state);
    if (!m_sleeping)
        return;
    if (state == SessionState::Active) {
        // Not from inside the controller's own state change.
        m_relock.start();
        return;
    }
    releaseWhenLocked();
}

void SleepLock::releaseWhenLocked()
{
    const SessionState state = m_session->state();
    if (state == SessionState::Locked || state == SessionState::Login)
        m_inhibitor = base::dbus::UnixFd();
}

void SleepLock::updateLockedHint(SessionState state)
{
    if (!m_started)
        return;
    // The hint turns on once the lock covers every output and off only when
    // the desktop is back.
    bool locked = m_lockedHint;
    if (state == SessionState::Locked)
        locked = true;
    else if (state == SessionState::Active)
        locked = false;
    if (locked == m_lockedHint)
        return;
    m_lockedHint = locked;
    if (!m_logindSession.send("SetLockedHint", locked))
        LM_WARNING(logs()) << "Cannot tell logind that the session is"
                           << (locked ? "locked" : "unlocked");
}
} // namespace Lingmo::Session
