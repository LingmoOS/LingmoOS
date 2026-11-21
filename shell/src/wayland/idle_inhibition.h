// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>

#include "base/signal.h"
#include "base/weak_ptr.h"

namespace Lingmo::Input {
class InputManager;
}

namespace Lingmo::Wayland {

// Shared inhibition count. Wayland inhibitors and the org.freedesktop.ScreenSaver
// service (session::ScreenSaverService) acquire it; idle notifications read it.
class IdleInhibition
{
public:
    IdleInhibition() = default;
    IdleInhibition(const IdleInhibition &) = delete;
    IdleInhibition &operator=(const IdleInhibition &) = delete;

    bool inhibited() const { return m_count != 0 && !m_ignored; }

    void acquire();
    void release();

    // Nothing a client shows can be seen on the lock screen, so its inhibitors
    // wait until the session is unblocked.
    void setIgnored(bool ignored);
    input::InputManager *input = nullptr;

    base::WeakPtr<IdleInhibition> weakPtr() { return m_weakFactory.getWeakPtr(); }

    base::Signal<> inhibitedChanged;
    base::Signal<> sceneChanged;

private:
    size_t m_count = 0;
    bool m_ignored = false;
    base::WeakPtrFactory<IdleInhibition> m_weakFactory {this};
};
} // namespace Lingmo::Wayland
