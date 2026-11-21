// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_SESSION_SESSION_H_
#define LINGMO_BACKEND_SESSION_SESSION_H_

#include <string>

#include "base/signal.h"

namespace Lingmo::Backend {

// Owns privileged device access. Devices are opened through the session rather
// than directly, so the compositor never needs to run as root and the seat
// manager can revoke access across VT switches.
class Session
{
public:
    Session() = default;
    virtual ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    virtual std::string seatName() const = 0;

    // False while another session owns the seat. No DRM ioctl that requires
    // master may be issued while inactive.
    virtual bool isActive() const = 0;

    // Returns a file descriptor the session owns, or -1. `deviceId` receives the
    // handle closeDevice() takes; closing the device closes the descriptor too.
    virtual int openDevice(const std::string& path, int* deviceId) = 0;
    virtual void closeDevice(int deviceId) = 0;

    // Whether the seat is now active; emitted before it is actually released,
    // so listeners can stop touching devices first.
    base::Signal<bool> activeChanged;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_SESSION_SESSION_H_
