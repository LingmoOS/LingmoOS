// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace Lingmo::ui {

class Window;

// Something that runs on its window's frames, such as an animation or a
// scroll coast. While ticking it is called once per rendered frame and keeps
// frames coming.
class FrameClient
{
public:
    enum class Interruption {
        // A pointer button or key went down in the window.
        Input,
        // Fingers came to rest on the touchpad.
        FingersDown,
        // A touchpad gesture began somewhere in the window.
        ScrollBegan,
        // The window lost keyboard focus.
        Deactivated,
    };

    explicit FrameClient(Window* window);
    virtual ~FrameClient();

    FrameClient(const FrameClient&) = delete;
    FrameClient& operator=(const FrameClient&) = delete;

    Window* window() const { return m_window; }
    bool isTicking() const { return m_ticking; }

protected:
    void startTicking();
    void stopTicking();

    // The frame's time, CLOCK_MONOTONIC: a frame clock stepping by whole
    // refreshes rather than when rendering happened to start.
    virtual void tick(uint64_t frameTimeNs) = 0;
    virtual void interrupt(Interruption) {}

private:
    friend class Window;

    Window* m_window;
    bool m_ticking = false;
};

} // namespace Lingmo::ui
