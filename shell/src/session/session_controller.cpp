// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/session_controller.h"

#include <algorithm>

#include "core/logging.h"
#include "display/display_types.h"
#include "session/secure_container.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Session {

const char *toString(SessionState state)
{
    switch (state) {
    case SessionState::Active:
        return "Active";
    case SessionState::Login:
        return "Login";
    case SessionState::Locking:
        return "Locking";
    case SessionState::Locked:
        return "Locked";
    case SessionState::Unlocking:
        return "Unlocking";
    }
    return "Unknown";
}

SessionController::SessionController(SecureContainer *secure, Delegate *delegate,
                                     SessionState initial)
    : m_secure(secure)
    , m_delegate(delegate)
    , m_state(initial)
{
    if (m_secure) {
        m_secure->setBlocked(isSessionBlocked(initial));
        m_secure->setCovered(isSessionBlocked(initial), false);
    }
    if (isBlocked() && m_delegate)
        m_delegate->onSessionBlocked();
}

SessionController::~SessionController() = default;

void SessionController::setState(SessionState state)
{
    if (m_state == state)
        return;
    const bool wasBlocked = isBlocked();
    m_state = state;
    m_presentedOutputs.clear();
    const bool blocked = isBlocked();

    if (m_secure) {
        m_secure->setBlocked(blocked);
        // Locked and Active wait for the transition's last frame to be presented.
        if (state == SessionState::Locking || state == SessionState::Unlocking)
            m_secure->setCovered(state == SessionState::Locking, true);
        else if (state == SessionState::Login)
            m_secure->setCovered(true, false);
    }
    if (blocked != wasBlocked && m_delegate) {
        if (blocked)
            m_delegate->onSessionBlocked();
        else
            m_delegate->onSessionUnblocked();
    }
    LM_INFO(lingmoAurora) << "Session state" << toString(state);

    const auto observers = m_observers;
    for (SessionObserver *observer : observers)
        observer->onSessionStateChanged(state);
}

bool SessionController::requestLock()
{
    // A lock asked for while an unlock is under way cancels it: the lock
    // screen comes back at once and has to be authenticated again.
    if (m_state == SessionState::Unlocking && m_unlockingFrom == SessionState::Locked) {
        setState(SessionState::Locking);
        if (m_secure)
            m_secure->setCovered(true, false);
        return true;
    }
    if (isBlocked())
        return false;
    setState(SessionState::Locking);
    return true;
}

void SessionController::setDisplayConfiguration(const display::AppliedConfiguration &configuration)
{
    std::vector<SecureOutput> outputs;
    for (const auto &logical : configuration.logicalDisplays) {
        // Mirrored destinations share bounds but each needs its own presentation acknowledgement.
        for (const auto &destination : logical.destinations)
            outputs.push_back({destination.value(), logical.bounds, logical.scale.value(),
                               logical.primary});
    }
    setOutputs(outputs);
}

void SessionController::setOutputs(const std::vector<SecureOutput> &outputs)
{
    if (m_secure && m_secure->outputs() != outputs) {
        for (const auto &old : m_secure->outputs()) {
            if (std::find(outputs.begin(), outputs.end(), old) == outputs.end())
                m_presentedOutputs.erase(old.name);
        }
        m_secure->setOutputs(outputs);
        const auto observers = m_observers;
        for (SessionObserver *observer : observers)
            observer->onSessionOutputsChanged();
    }
    finishTransition();
}

bool SessionController::everyOutputPresented() const
{
    if (!m_secure)
        return true;
    // No outputs is a layout being rebuilt, not a lock on screen.
    if (m_secure->outputs().empty())
        return false;
    for (const SecureOutput &output : m_secure->outputs()) {
        if (!m_presentedOutputs.contains(output.name))
            return false;
    }
    return true;
}

void SessionController::finishTransition()
{
    if (!everyOutputPresented())
        return;
    if (m_state == SessionState::Locking)
        setState(SessionState::Locked);
    else if (m_state == SessionState::Unlocking)
        setState(SessionState::Active);
}

void SessionController::notifyOutputPresented(std::string_view outputName)
{
    if (m_state != SessionState::Locking && m_state != SessionState::Unlocking)
        return;
    m_presentedOutputs.emplace(outputName);
    finishTransition();
}

void SessionController::notifyFramePresented()
{
    if (m_state != SessionState::Locking && m_state != SessionState::Unlocking)
        return;
    if (m_secure) {
        for (const SecureOutput &output : m_secure->outputs())
            m_presentedOutputs.insert(output.name);
    }
    finishTransition();
}

bool SessionController::requestUnlock()
{
    if (m_state != SessionState::Locked && m_state != SessionState::Login)
        return false;
    m_unlockingFrom = m_state;
    setState(SessionState::Unlocking);
    return true;
}

void SessionController::addObserver(SessionObserver *observer)
{
    if (observer
        && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end()) {
        m_observers.push_back(observer);
        observer->onSessionStateChanged(m_state);
    }
}

void SessionController::removeObserver(SessionObserver *observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer),
                      m_observers.end());
}

scene::Node *SessionController::inputRoot() const
{
    return nullptr;
}

bool SessionController::allowsFocus(const compositor::Surface *) const
{
    return !isBlocked();
}

bool SessionController::allowsActivation() const
{
    return !isBlocked();
}

} // namespace Lingmo::Session
