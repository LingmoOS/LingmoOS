// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/session/libseat_session.h"


#include <sys/epoll.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <utility>
#include <vector>

// libseat.h ships without extern "C" guards.
extern "C" {
#include <libseat.h>
}

#include "base/event_loop.h"
#include "core/logging.h"
#include "core/main_loop.h"

namespace Lingmo::Backend {

namespace {

void logHandler(libseat_log_level level, const char* format, va_list args)
{
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    if (level == LIBSEAT_LOG_LEVEL_ERROR)
        LM_WARNING(lingmoSession).printf("libseat: %s", buffer);
    else
        LM_DEBUG(lingmoSession).printf("libseat: %s", buffer);
}

// The signals main.cpp blocks for its signalfd, pending before it reads them.
bool stopPending()
{
    sigset_t pending;
    if (sigpending(&pending) != 0)
        return false;
    return sigismember(&pending, SIGTERM) == 1 || sigismember(&pending, SIGINT) == 1
            || sigismember(&pending, SIGHUP) == 1;
}

} // namespace

LibseatSession::~LibseatSession()
{
    if (m_watch)
        core::mainLoop()->unwatch(m_watch);
    // Owners close their devices first today; whatever is left goes with the seat.
    std::vector<int> ids;
    for (const auto &[id, fd] : m_deviceFds)
        ids.push_back(id);
    for (const int id : ids)
        closeDevice(id);
    if (m_seat)
        libseat_close_seat(m_seat);
}

std::unique_ptr<LibseatSession> LibseatSession::create()
{
    libseat_set_log_handler(logHandler);
    libseat_set_log_level(LIBSEAT_LOG_LEVEL_INFO);

    std::unique_ptr<LibseatSession> session(new LibseatSession());

    // libseat keeps this pointer and calls through it for the seat's lifetime,
    // so it cannot live on this stack frame.
    static const libseat_seat_listener listener = {
        .enable_seat = &LibseatSession::onEnableSeat,
        .disable_seat = &LibseatSession::onDisableSeat,
    };

    session->m_seat = libseat_open_seat(&listener, session.get());
    if (!session->m_seat) {
        LM_CRITICAL(lingmoSession) << "Failed to open a seat; is seatd running or is "
                                 "this a logind session?";
        return nullptr;
    }

    // The seat is unusable until the first enable event, which never comes to a
    // session nobody activates; a stop must still end the wait.
    while (!session->m_active) {
        if (stopPending()) {
            LM_INFO(lingmoSession) << "Stopped while waiting for the seat";
            return nullptr;
        }
        if (libseat_dispatch(session->m_seat, 100) < 0) {
            LM_CRITICAL(lingmoSession) << "Failed to dispatch libseat while waiting for the seat";
            return nullptr;
        }
    }

    const char* name = libseat_seat_name(session->m_seat);
    session->m_seatName = name ? name : "";

    const int fd = libseat_get_fd(session->m_seat);
    if (fd < 0) {
        LM_CRITICAL(lingmoSession) << "libseat provided no event fd";
        return nullptr;
    }

    session->m_watch = core::mainLoop()->watch(fd, EPOLLIN,
                                               [s = session.get()](uint32_t) { s->dispatch(); });

    LM_INFO(lingmoSession) << "Seat:" << session->m_seatName;
    return session;
}

void LibseatSession::dispatch()
{
    if (libseat_dispatch(m_seat, 0) >= 0 || errno == EINTR || errno == EAGAIN)
        return;
    // The seat connection is gone and its fd stays readable: watching it
    // further would only spin. Open devices keep working; switching does not.
    LM_CRITICAL(lingmoSession) << "Lost the seat connection:" << strerror(errno);
    core::mainLoop()->unwatch(std::exchange(m_watch, 0));
}

int LibseatSession::openDevice(const std::string& path, int* deviceId)
{
    int fd = -1;
    const int id = libseat_open_device(m_seat, path.c_str(), &fd);
    if (id < 0) {
        LM_WARNING(lingmoSession) << "Failed to open device" << path << "through the seat";
        return -1;
    }
    m_deviceFds.insert_or_assign(id, fd);
    if (deviceId)
        *deviceId = id;
    return fd;
}

void LibseatSession::closeDevice(int deviceId)
{
    if (deviceId < 0)
        return;
    if (libseat_close_device(m_seat, deviceId) < 0)
        LM_WARNING(lingmoSession) << "Failed to close device" << deviceId;
    // Only afterwards: with logind the id is this descriptor, which
    // libseat_close_device still stats to release the device.
    const auto entry = m_deviceFds.find(deviceId);
    if (entry != m_deviceFds.end()) {
        ::close(entry->second);
        m_deviceFds.erase(entry);
    }
}

void LibseatSession::onEnableSeat(libseat*, void* userData)
{
    auto* self = static_cast<LibseatSession*>(userData);
    if (self->m_active)
        return;
    self->m_active = true;
    LM_INFO(lingmoSession) << "Seat enabled";
    self->activeChanged(true);
}

void LibseatSession::onDisableSeat(libseat* seat, void* userData)
{
    auto* self = static_cast<LibseatSession*>(userData);
    if (self->m_active) {
        self->m_active = false;
        LM_INFO(lingmoSession) << "Seat disabled";
        self->activeChanged(false);
    }
    // Only acknowledge once listeners have stopped using their devices.
    libseat_disable_seat(seat);
}

} // namespace Lingmo::Backend
