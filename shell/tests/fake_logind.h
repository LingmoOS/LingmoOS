// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_FAKE_LOGIND_H
#define LINGMO_TESTS_FAKE_LOGIND_H

#include <fcntl.h>
#include <memory>
#include <poll.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "base/dbus.h"
#include "core/main_loop.h"

namespace Lingmo::Test {

// logind's manager on a connection of its own, as much of it as the session
// uses: Inhibit hands out the write end of a pipe, and the read end tells
// whether anyone still holds it. The caller's session records its LockedHint.
class FakeLogind
{
public:
    FakeLogind()
        : m_bus(base::dbus::Connection::open(base::dbus::Connection::Bus::Session, core::mainLoop()))
    {
        if (!m_bus)
            return;
        m_object = std::make_unique<base::dbus::Object>(m_bus.get(), "/org/freedesktop/login1",
                                                        "org.freedesktop.login1.Manager");
        m_object->addMethod("Inhibit", [this](const std::string &what, const std::string &who,
                                              const std::string &why, const std::string &mode) {
            lastInhibit = {what, who, why, mode};
            int ends[2];
            if (pipe2(ends, O_CLOEXEC) != 0)
                return base::dbus::UnixFd();
            readEnds.push_back(ends[0]);
            return base::dbus::UnixFd::adopt(ends[1]);
        });
        m_object->addSignal("PrepareForSleep", "b");
        m_session = std::make_unique<base::dbus::Object>(m_bus.get(),
                                                         "/org/freedesktop/login1/session/auto",
                                                         "org.freedesktop.login1.Session");
        m_session->addMethod("SetLockedHint",
                             [this](bool locked) { lockedHints.push_back(locked); });
        m_valid = m_object->publish() && m_session->publish()
                && m_bus->requestName("org.freedesktop.login1");
    }

    ~FakeLogind()
    {
        if (m_bus)
            m_bus->releaseName("org.freedesktop.login1");
        for (int fd : readEnds)
            close(fd);
    }

    bool isValid() const { return m_valid; }

    // Whether the descriptor handed out last is still open anywhere.
    bool held() const
    {
        if (readEnds.empty())
            return false;
        pollfd descriptor {readEnds.back(), POLLIN, 0};
        return poll(&descriptor, 1, 0) == 0;
    }

    bool prepareForSleep(bool sleeping) { return m_object->emitSignal("PrepareForSleep", sleeping); }

    std::vector<std::string> lastInhibit;
    std::vector<int> readEnds;
    std::vector<bool> lockedHints;

private:
    std::unique_ptr<base::dbus::Connection> m_bus;
    std::unique_ptr<base::dbus::Object> m_object;
    std::unique_ptr<base::dbus::Object> m_session;
    bool m_valid = false;
};

} // namespace Lingmo::Test

#endif // LINGMO_TESTS_FAKE_LOGIND_H
