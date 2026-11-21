// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/power_button.h"
#include "core/main_loop.h"
#include "session/session_controller.h"
#include "base/logging.h"
#include <linux/input-event-codes.h>

namespace Lingmo::Session {

namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

const char *const service = "org.freedesktop.login1";
const char *const path = "/org/freedesktop/login1";
const char *const manager = "org.freedesktop.login1.Manager";

// Ash's PowerButtonController delays: the press that woke the system, and taps
// faster than they can be acted on.
constexpr std::chrono::milliseconds kIgnoreAfterResume(2000);
constexpr std::chrono::milliseconds kIgnoreRepeatedRelease(500);

bool within(const std::optional<std::chrono::steady_clock::time_point> &since,
            std::chrono::milliseconds duration)
{
    return since && std::chrono::steady_clock::now() - *since <= duration;
}
} // namespace

PowerButton::PowerButton(SessionController *session, input::EventSink *next,
                         base::dbus::Connection *bus)
    : m_session(session)
    , m_next(next)
    , m_bus(bus ? bus : base::dbus::Connection::system(core::mainLoop()))
    , m_manager(m_bus, service, path, manager)
{
}

PowerButton::~PowerButton() = default;

bool PowerButton::start()
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
    return true;
}

void PowerButton::inhibit()
{
    m_inhibiting = m_manager.callAsync(
            "Inhibit",
            [this](const base::dbus::Reply &reply) {
                if (reply.isError()) {
                    LM_WARNING(logs()) << "Cannot take the power button over from logind:"
                               << reply.error().message;
                    return;
                }
                m_inhibitor = reply.value<base::dbus::UnixFd>().value_or(base::dbus::UnixFd());
            },
            std::string("handle-power-key"), std::string("Lingmo"),
            std::string("The power button locks the screen"), std::string("block"));
}

void PowerButton::prepareForSleep(bool sleeping)
{
    if (!sleeping)
        m_resumed = std::chrono::steady_clock::now();
}

bool PowerButton::onKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    if (evdevCode != KEY_POWER || !m_inhibitor.isValid())
        return m_next && m_next->onKey(timestamp, evdevCode, pressed);

    if (pressed) {
        m_ignoreRelease = within(m_resumed, kIgnoreAfterResume);
        return true;
    }

    const bool repeated = within(m_released, kIgnoreRepeatedRelease);
    m_released = std::chrono::steady_clock::now();
    if (m_ignoreRelease || repeated)
        return true;

    m_session->requestLock();
    return true;
}

bool PowerButton::onSecureKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    return m_next && m_next->onSecureKey(timestamp, evdevCode, pressed);
}

bool PowerButton::onPointerMotion(input::Timestamp timestamp, const base::PointF &location)
{
    return m_next && m_next->onPointerMotion(timestamp, location);
}

bool PowerButton::onPointerButton(input::Timestamp timestamp, const base::PointF &location,
                                  uint32_t evdevButton, bool pressed)
{
    return m_next && m_next->onPointerButton(timestamp, location, evdevButton, pressed);
}

bool PowerButton::onPointerAxis(input::Timestamp timestamp, const base::PointF &location,
                                const base::PointF &offset, bool discrete)
{
    return m_next && m_next->onPointerAxis(timestamp, location, offset, discrete);
}

bool PowerButton::onPointerAxisStop(input::Timestamp timestamp, const base::PointF &location)
{
    return m_next && m_next->onPointerAxisStop(timestamp, location);
}

bool PowerButton::onTouchDown(input::Timestamp timestamp, int32_t id,
                              const base::PointF &location)
{
    return m_next && m_next->onTouchDown(timestamp, id, location);
}

bool PowerButton::onTouchMotion(input::Timestamp timestamp, int32_t id,
                                const base::PointF &location)
{
    return m_next && m_next->onTouchMotion(timestamp, id, location);
}

bool PowerButton::onTouchUp(input::Timestamp timestamp, int32_t id)
{
    return m_next && m_next->onTouchUp(timestamp, id);
}

void PowerButton::onTouchCancel(input::Timestamp timestamp)
{
    if (m_next)
        m_next->onTouchCancel(timestamp);
}
} // namespace Lingmo::Session
