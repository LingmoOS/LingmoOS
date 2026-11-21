// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <list>
#include <utility>

#include "base/dbus.h"

namespace Lingmo::Shell {

// Asynchronous D-Bus calls of an object, kept until their replies land and
// dropped with it, so no reply ever reaches an object that is gone.
class PendingCalls
{
public:
    // `start` sends the call with a reply handler that calls `finished` last.
    // A call that cannot be sent answers inside start(); that works too.
    using Start = std::function<base::dbus::PendingCall(std::function<void()> finished)>;

    void track(const Start &start)
    {
        const auto call = m_calls.emplace(m_calls.end());
        base::dbus::PendingCall pending = start([this, call] {
            if (call->kept)
                m_calls.erase(call);
            else
                call->finished = true;
        });
        if (call->finished) {
            m_calls.erase(call);
            return;
        }
        call->pending = std::move(pending);
        call->kept = true;
    }

private:
    struct Call {
        base::dbus::PendingCall pending;
        bool kept = false;
        bool finished = false;
    };

    std::list<Call> m_calls;
};

} // namespace Lingmo::Shell

