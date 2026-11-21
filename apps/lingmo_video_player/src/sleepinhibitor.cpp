// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sleepinhibitor.h"

#include "base/event_loop.h"
#include "base/i18n.h"

using namespace Lingmo;
using base::dbus::Connection;
using base::dbus::Proxy;
using base::dbus::Reply;
using base::dbus::UnixFd;
using i18n::tr;

static const char* const kScreenSaverService = "org.freedesktop.ScreenSaver";
static const char* const kScreenSaverPath = "/ScreenSaver";
static const char* const kLogin1Service = "org.freedesktop.login1";
static const char* const kLogin1Path = "/org/freedesktop/login1";
static const char* const kLogin1Manager = "org.freedesktop.login1.Manager";

static uint64_t s_nextCall = 1;

// Calls still in flight when their inhibitor went, kept so a late cookie is
// handed back. Never freed: a reply can arrive while the process exits.
static std::map<uint64_t, base::dbus::PendingCall>& orphanedCalls()
{
    static auto* calls = new std::map<uint64_t, base::dbus::PendingCall>();
    return *calls;
}

// Not from inside the call's own callback.
static void forgetOrphan(uint64_t call)
{
    if (base::EventLoop* loop = base::EventLoop::current())
        loop->post([call] { orphanedCalls().erase(call); });
}

SleepInhibitor::SleepInhibitor(Connection* session, Connection* system)
    : m_session(session ? session : Connection::session())
    , m_system(system ? system : Connection::system())
{
}

SleepInhibitor::~SleepInhibitor()
{
    for (auto& [call, pending] : m_calls)
        orphanedCalls().emplace(call, std::move(pending));
    m_calls.clear();
    release();
}

void SleepInhibitor::setMode(Mode mode)
{
    if (m_mode == mode)
        return;
    const bool wasHeld = isHeld();
    release();
    m_mode = mode;
    const uint64_t generation = ++m_generation;
    modeChanged.emit();
    if (wasHeld)
        heldChanged.emit();
    acquire(mode, generation);
}

void SleepInhibitor::retranslate()
{
    if (m_mode == Mode::Off)
        return;
    const bool wasHeld = isHeld();
    release();
    const uint64_t generation = ++m_generation;
    if (wasHeld)
        heldChanged.emit();
    acquire(m_mode, generation);
}

void SleepInhibitor::acquire(Mode mode, uint64_t generation)
{
    const uint64_t call = s_nextCall++;
    auto weak = m_weakFactory.getWeakPtr();
    if (mode == Mode::Screen) {
        const Proxy screenSaver(m_session, kScreenSaverService, kScreenSaverPath, kScreenSaverService);
        m_calls[call] = screenSaver.callAsync(
                "Inhibit",
                [weak, generation, call, session = m_session](const Reply& reply) {
                    const auto cookie = reply.value<uint32_t>();
                    if (!weak) {
                        forgetOrphan(call);
                        if (cookie)
                            Proxy(session, kScreenSaverService, kScreenSaverPath, kScreenSaverService)
                                    .send("UnInhibit", *cookie);
                        return;
                    }
                    weak->finish(call);
                    if (!cookie)
                        return;
                    if (generation == weak->m_generation) {
                        weak->m_cookie = *cookie;
                        weak->heldChanged.emit();
                    } else {
                        Proxy(weak->m_session, kScreenSaverService, kScreenSaverPath, kScreenSaverService)
                                .send("UnInhibit", *cookie);
                    }
                },
                "lingmo-video-player", tr("Playing a video"));
    } else if (mode == Mode::Suspend) {
        const Proxy login1(m_system, kLogin1Service, kLogin1Path, kLogin1Manager);
        m_calls[call] = login1.callAsync(
                "Inhibit",
                [weak, generation, call](const Reply& reply) {
                    // An outdated lock is closed as the reply goes.
                    if (!weak) {
                        forgetOrphan(call);
                        return;
                    }
                    weak->finish(call);
                    const auto lock = reply.value<UnixFd>();
                    if (lock && lock->isValid() && generation == weak->m_generation) {
                        weak->m_suspendLock = *lock;
                        weak->heldChanged.emit();
                    }
                },
                "sleep", tr("Video Player"), tr("Playing audio"), "block");
    }
}

void SleepInhibitor::finish(uint64_t call)
{
    // Not from inside the call's own callback.
    if (base::EventLoop* loop = base::EventLoop::current()) {
        loop->post([weak = m_weakFactory.getWeakPtr(), call] {
            if (weak)
                weak->m_calls.erase(call);
        });
    }
}

void SleepInhibitor::release()
{
    if (m_cookie) {
        Proxy(m_session, kScreenSaverService, kScreenSaverPath, kScreenSaverService).send("UnInhibit", m_cookie);
        m_cookie = 0;
    }
    m_suspendLock = UnixFd();
}
