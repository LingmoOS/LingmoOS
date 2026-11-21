// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_SESSION_LIBSEAT_SESSION_H_
#define LINGMO_BACKEND_SESSION_LIBSEAT_SESSION_H_


#include <memory>
#include <string>
#include <unordered_map>

#include "backend/session/session.h"

struct libseat;

namespace Lingmo::Backend {

// Session backed by libseat (seatd or logind).
class LibseatSession : public Session
{
public:
    ~LibseatSession() override;

    // Opens the seat and pumps libseat until the initial enable event arrives.
    // Returns nullptr if no seat could be acquired.
    static std::unique_ptr<LibseatSession> create();

    std::string seatName() const override { return m_seatName; }
    bool isActive() const override { return m_active; }

    int openDevice(const std::string& path, int* deviceId) override;
    void closeDevice(int deviceId) override;

private:
    LibseatSession() = default;

    void dispatch();

    static void onEnableSeat(libseat* seat, void* userData);
    static void onDisableSeat(libseat* seat, void* userData);

    libseat* m_seat = nullptr;
    int m_watch = 0;
    std::string m_seatName;
    bool m_active = false;
    // Device id -> the descriptor libseat handed out, which libseat never closes.
    std::unordered_map<int, int> m_deviceFds;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_SESSION_LIBSEAT_SESSION_H_
